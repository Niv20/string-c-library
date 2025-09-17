# Generic Linked List Library in C

This is a comprehensive, generic linked list library written in C. It is designed to be type-agnostic by using void pointers for data storage.

## Setup for Examples

To demonstrate the library's functionality, we will use a Person struct. This setup will be assumed for all subsequent examples.

```c
// The structure we will store in the list
typedef struct {
    int id;
    char* name;
    int age;
} Person;
```

For this library to work, you need to define 3 **helper functions** in your own code, that the list will use to manage your objects. These functions are essential for the library to understand how to work with your specific data type.

1. `void PrintFunction(void* data)` - This function assigns a custom printing function to the list.Without this, the list doesn't know how to interpret and print your data structure.

2. `int CompareFunction(const void* data1, const void* data2)` – This function is used to compare two elements in your data structure. It return a negative value if the first element (`data1`) is less than the second (`data2`), zero if they are equal, and a positive value if the first is greater than the second.

3. `void FreeFunction(void* data)` – This function frees all dynamically allocated memory **inside** your struct, such as strings or arrays allocated with malloc (The library itself frees the memory of the struct itself).

> [!NOTE]
> The parameters for these functions are of type `void*`, so you need to cast the pointer back to your data type within the function.

Continuing with our example, these functions would look like this:

```c
// 1. Function to print a Person's details
void print_person(void* data) {

    // Cast the void pointer back to Person pointer
    Person* p = (Person*)data;

    printf("Id: %d, Name: %s, Age: %d\n", p->id, p->name, p->age);
}

// 2. Function to compare two Persons by age
int compare_person_age(const void* data1, const void* data2) {

    // Cast the void pointers back to Person pointers
    Person* p1 = (Person*)data1;
    Person* p2 = (Person*)data2;

    return p1->age - p2->age;
}

// 3. Function to free the dynamically allocated name within a Person struct
void free_person(void* data) {

    // Cast the void pointers back to Person pointers
    Person* p = (Person*)data;

    free(p->name); // Free the allocated string

    // The struct itself is managed by the list, so we don't free 'p'
}
```

> [!NOTE]
> If your struct does **not** contain any dynamically allocated fields (such as pointers to memory allocated with `malloc`), you do **not** need to implement the last function at all.

## 1. Create List

### `create_list`

This is the starting point for using the library. It allocates memory for a new, empty `LinkedList` structure and initializes it.

**Receives:**

- `element_size`: The size in bytes of the data type you plan to store. Use the `sizeof` operator for this (e.g., `sizeof(Person)`).

**Returns:**

- A pointer to the newly created `LinkedList` on success.
- `NULL` if memory allocation fails.

**Example:**

```c
// Create a list to store Person structs
LinkedList* person_list = create_list(sizeof(Person));

// Check if the list was created successfully
if (!person_list) {
    printf("Failed to create list.\n");
    return 1;
}

printf("Successfully created a list for Person objects.\n");
```

> [!NOTE] 
> This function sets up dummy head and tail nodes, to simplify the logic for all other list operations by ensuring that every "real" node is always between two other nodes.

This function only creates the list. It is currently "empty" (except for the dummy nodes, of course). Later, we will learn how to [add elements to it](#3-insertion-in-linked-list).

---

## 2. List Configuration

Before using the list, you need to configure it with the appropriate [helper functions](#setup-for-examples).

```c
// Essential configuration
set_print_function(person_list, print_person);
set_compare_function(person_list, compare_person_age);
set_free_function(person_list, free_person);
```

Additionally, you can set a maximum size limit for your list using `set_max_size()`. This function is particularly useful when implementing caches or buffers that shouldn't grow indefinitely. You can specify the maximum number of elements allowed (or `UNLIMITED` for no limit), and choose the behavior when the list reaches capacity - either reject new insertions or automatically delete the oldest elements to make room.

for example, you can set max 100 elements, auto-delete old when full:

```c
set_max_size(person_list, 100, DELETE_OLD_WHEN_FULL);
```

Or no size limit:

```c
set_max_size(person_list, UNLIMITED, REJECT_NEW_WHEN_FULL);
```

> [!IMPORTANT]
> When using `UNLIMITED`, the behavior parameter is ignored since there's no capacity limit to reach.

---

## 3. Insertion in Linked List

The library provides two ways to insert elements: the traditional pointer-based functions and convenient value-based macros.

### 🆕 Convenient Value-Based Insertion (NEW!)

For a more intuitive API, you can use the new convenience macros that allow you to pass values directly without using the `&` operator:

```c
// Traditional way (still works)
Person alice = create_person(1001, "Alice Johnson", 28);
insert_tail_ptr(people_list, &alice);  // Notice the & operator

// NEW convenient way - pass values directly!
Person bob = create_person(1002, "Bob Smith", 35);
insert_tail_val(people_list, bob);  // No & needed!

// Works with any type
int number = 42;
insert_head_val(numbers_list, number);

// Even with literals
insert_tail_val(numbers_list, 100);
insert_at_val(numbers_list, 1, 75);
```

**Available convenience macros:**
- `insert_head_val(list, value)`
- `insert_tail_val(list, value)`
- `insert_at_val(list, index, value)`

> [!NOTE]
> These macros work with GCC and Clang compilers. For other compilers, they fall back to the traditional pointer-based approach.

### Traditional Pointer-Based Insertion

### `insert_head_ptr`

This function adds a new element to the beginning of the list.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to be inserted.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_head_ptr(list, &n1); // List: [10]
insert_head_ptr(list, &n2); // List: [20, 10]
destroy(list);
```

### `insert_tail_ptr`

This function adds a new element to the very end of the list.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to be inserted. The library makes its own copy.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list, &n1); // List: [10]
insert_tail_ptr(list, &n2); // List: [10, 20]
destroy(list);
```

### `insert_at_ptr`

This function inserts an element at a specific zero-based index. All elements from that index onward are shifted one position to the right. The operation is optimized to traverse from the head or tail, whichever is closer to the target index.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The zero-based index where the element should be inserted.
- `data`: A pointer to the data to be inserted.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20, n3 = 30;
insert_tail_ptr(list, &n1); // List: [10]
insert_tail_ptr(list, &n2); // List: [10, 20]
insert_at_ptr(list, 1, &n3); // List: [10, 30, 20]
destroy(list);
```

---

## 4. Deletion Functions

### `delete_head`

This function removes the first element (at index 0) from the list. This is an O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list, &n1);
insert_tail_ptr(list, &n2); // List: [10, 20]
delete_head(list);   // List: [20]
destroy(list);
```

### `delete_tail`

This function removes the last element from the list. This is an O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list, &n1);
insert_tail_ptr(list, &n2); // List: [10, 20]
delete_tail(list);   // List: [10]
destroy(list);
```

### `delete_at`

This function removes an element at a specific zero-based index. Subsequent elements are shifted to the left.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to delete.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20, n3 = 30;
insert_tail_ptr(list, &n1);
insert_tail_ptr(list, &n2);
insert_tail_ptr(list, &n3); // List: [10, 20, 30]
delete_at(list, 1); // List: [10, 30]
destroy(list);
```

### `remove_advanced`

This is a powerful function that finds and removes elements based on their value. It requires a compare function to be set. You can control how many matching elements to remove and from which direction to start the search.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to find and remove.
- `count`: The number of occurrences to remove. Use `DELETE_ALL_OCCURRENCES` to remove all matches.
- `direction`: `SEARCH_FROM_HEAD` or `SEARCH_FROM_TAIL`.

**Returns:**

- `LIST_SUCCESS` if at least one element was removed, otherwise `LIST_ERROR_ELEMENT_NOT_FOUND`.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list = create_list(sizeof(int));
set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 10};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 10, 30, 10]
int target = 10;
// Remove all occurrences of 10
remove_advanced(list, &target, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
// List is now: [20, 30]
destroy(list);
```

### `clear`

This function efficiently removes all elements from the list, resetting its length to zero. It properly frees the memory for each element using the configured free function.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list, &n1);
insert_tail_ptr(list, &n2); // List: [10, 20]
clear(list); // List: []
printf("Is list empty? %s\n", is_empty(list) ? "Yes" : "No"); // Output: Yes
destroy(list);
```

### `destroy`

This function is the final cleanup step. It completely deallocates all memory used by the list, including clearing all elements (using `clear`), freeing the dummy nodes, and finally freeing the `LinkedList` structure itself. After calling this, the list pointer is no longer valid.

**Receives:**

- `list`: A pointer to the `LinkedList` to be destroyed.

**Returns:**

- Nothing.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n = 10;
insert_tail_ptr(list, &n);
// ... use the list ...
destroy(list); // All memory is freed.
// list = NULL; // Good practice to nullify the pointer after destroying.
```

---

## 5. Utility Functions

### `get_length`

A straightforward function that returns the number of elements currently stored in the list. This is an O(1) operation as the length is tracked internally.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- The number of elements as a `size_t`.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list, &n1);
insert_tail_ptr(list, &n2);
printf("List length: %zu\n", get_length(list)); // Output: 2
destroy(list);
```

### `is_empty`

A simple check to see if the list contains any elements. It's slightly more expressive than checking `get_length(list) == 0`.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `TRUE` if the list has zero elements, `FALSE` otherwise.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
printf("Is list empty? %s\n", is_empty(list) ? "Yes" : "No"); // Output: Yes
int n = 10;
insert_tail_ptr(list, &n);
printf("Is list empty? %s\n", is_empty(list) ? "Yes" : "No"); // Output: No
destroy(list);
```

### `print`

This function iterates through the list and prints all elements to the console. It relies on a print function being set via `set_print_function`. It provides a default format with indices.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or `LIST_ERROR_NO_PRINT_FUNCTION` if no print function is set.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
set_print_function(list, print_person);
Person alice = {.name = strdup("Alice"), .age = 30};
insert_tail_ptr(list, &alice);
// Output:
// List len: 1
//   [0]: Name: Alice, Age: 30
print(list);
free(alice.name);
destroy(list);
```

### `print_advanced`

This function provides more control over the output format than `print`. You can choose whether to display indices and specify a custom separator string to be printed between elements.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `show_index`: If `TRUE`, prints the `[index]:` prefix for each element.
- `separator`: The string to print between elements (e.g., `", "`, `" -> "`).

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
set_print_function(list, print_person);
Person alice = {.name = strdup("Alice"), .age = 30};
Person bob = {.name = strdup("Bob"), .age = 25};
insert_tail_ptr(list, &alice);
insert_tail_ptr(list, &bob);
// Output: Name: Alice, Age: 30 ---> Name: Bob, Age: 25
print_advanced(list, FALSE, " ---> ");
free(alice.name);
free(bob.name);
destroy(list);
```

---

## 6. Search and Access Functions

### `get`

This function retrieves a direct pointer to the data stored at a specific index. This is for read-only access and does not make a copy. It's very fast if you just need to inspect an element's value.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to retrieve.

**Returns:**

- A `void*` pointer to the data at the specified index.
- `NULL` on failure (e.g., index out of bounds).

**Warning:** The returned pointer is only valid as long as the list is not modified. Deleting elements could invalidate it.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
Person alice = {.name = strdup("Alice"), .age = 30};
insert_tail_ptr(list, &alice);
Person* p = (Person*)get(list, 0);
if (p) {
    printf("Person at index 0 is %s.\n", p->name); // Output: Alice
}
free(alice.name);
destroy(list);
```

### `set`

This function updates the element at a specific index with new data. It overwrites the existing data at that position. It properly frees the old data using the configured free function before copying the new data.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to update.
- `data`: A pointer to the new data.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
// Setup copy and free functions for Person
set_copy_function(list, copy_person);
set_free_function(list, free_person);
Person alice = {.name = strdup("Alice"), .age = 30};
insert_tail_ptr(list, &alice); // List: [Alice (30)]
Person alice_new = {.name = strdup("Alice"), .age = 31};
set(list, 0, &alice_new); // List: [Alice (31)]
free(alice.name);
free(alice_new.name);
destroy(list);
```

### `index`

This function searches the list from head to tail and returns the index of the first element that matches the provided data. It requires a compare function to be set.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to search for.

**Returns:**

- The non-negative index of the element if found.
- A negative error code if not found or an error occurs.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
set_compare_function(list, compare_person_age);
Person alice = {.name = strdup("Alice"), .age = 30};
Person bob = {.name = strdup("Bob"), .age = 25};
insert_tail_ptr(list, &bob);
insert_tail_ptr(list, &alice); // List: [Bob (25), Alice (30)]
Person target = {.age = 30}; // We only need age for the comparison
int index = index(list, &target);
if (index >= 0) {
    printf("A person aged 30 is at index: %d\n", index); // Output: 1
}
free(alice.name);
free(bob.name);
destroy(list);
```

### `index_advanced`

Similar to `index`, but allows you to specify the search direction. You can search from the head (for the first match) or from the tail (for the last match).

**Receives:**

- `list`: The list to search in.
- `data`: The data to find.
- `direction`: `SEARCH_FROM_HEAD` (for the first occurrence) or `SEARCH_FROM_TAIL` (for the last occurrence).

**Returns:**

- The index if found, or a negative error code.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list = create_list(sizeof(int));
set_compare_function(list, compare_int);
int nums[] = {10, 20, 30, 20, 40};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 30, 20, 40]
int target = 20;
int first_idx = index_advanced(list, &target, SEARCH_FROM_HEAD); // Result: 1
int last_idx = index_advanced(list, &target, SEARCH_FROM_TAIL);  // Result: 3
printf("First 20 is at index %d, last 20 is at index %d\n", first_idx, last_idx);
destroy(list);
```

### `count_occurrences`

This function iterates through the entire list and counts how many times a specific element appears. It requires a compare function to be set.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to count.

**Returns:**

- The number of occurrences as a `size_t`.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list = create_list(sizeof(int));
set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 10};
// array_to_list(list, nums, 5);
int target = 10;
size_t count = count_occurrences(list, &target);
printf("The number 10 appears %zu times.\n", count); // Output: 3
destroy(list);
```

---

## 7. Sorting Functions

### `sort`

This function sorts the list in-place using the configured compare function. It is implemented by converting the list to an array, running the standard library's highly efficient `qsort` (O(n log n)), and then rebuilding the list from the sorted array.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `reverse_order`: If `TRUE`, sorts in descending order; otherwise, sorts in ascending order.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
set_print_function(list, print_person);
set_compare_function(list, compare_person_age);
// ... add Alice (30), Bob (25), Charlie (35) ...
sort(list, FALSE); // Sort ascending by age
printf("Sorted list (ascending):\n");
// print(list); // Output: Bob (25), Alice (30), Charlie (35)

sort(list, TRUE); // Sort descending by age
printf("\nSorted list (descending):\n");
// print(list); // Output: Charlie (35), Alice (30), Bob (25)
destroy(list);
```

---

## 8. Structural Transformations

### `list_copy`

This function creates a new, independent copy of an entire list. If a `copy_fn` is configured, it will be used to deep copy each element, ensuring the new list is completely separate from the original.

**Receives:**

- `list`: The list to copy.

**Returns:**

- A pointer to the new `LinkedList`, or `NULL` on failure.

**Example:**

```c
LinkedList* original = create_list(sizeof(int));
// ... populate original list ...
LinkedList* copy = list_copy(original);
// 'copy' is now a separate list with the same elements.
// Modifying 'copy' will not affect 'original'.
delete_head(copy);
printf("Original length: %zu, Copy length: %zu\n", get_length(original), get_length(copy));
destroy(original);
destroy(copy);
```

### `list_extend`

This function appends all elements from a second list (`other`) to the end of the first list (`list`). The other list remains unchanged.

**Receives:**

- `list`: The list to be extended.
- `other`: The list whose elements will be appended.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list1 = create_list(sizeof(int));
LinkedList* list2 = create_list(sizeof(int));
int n1 = 10, n2 = 20;
insert_tail_ptr(list1, &n1); // list1: [10]
insert_tail_ptr(list2, &n2); // list2: [20]
list_extend(list1, list2); // list1 is now [10, 20]
destroy(list1);
destroy(list2);
```

### `list_concat`

Creates a brand new list that is the result of concatenating two existing lists. Both original lists remain unchanged.

**Receives:**

- `list1`: The first list.
- `list2`: The second list.

**Returns:**

- A new `LinkedList` containing elements of `list1` followed by `list2`, or `NULL` on failure.

**Example:**

```c
LinkedList* list1 = create_list(sizeof(int)); // Contains [10, 20]
LinkedList* list2 = create_list(sizeof(int)); // Contains [30, 40]
// ... populate lists ...
LinkedList* concatenated = list_concat(list1, list2);
// 'concatenated' is a new list: [10, 20, 30, 40]
destroy(list1);
destroy(list2);
destroy(concatenated);
```

### `list_slice`

Creates a new list containing a copy of a portion of the original list, from a start index (inclusive) to an end index (exclusive).

**Receives:**

- `list`: The source list.
- `start`: The starting index (inclusive).
- `end`: The ending index (exclusive).

**Returns:**

- A new `LinkedList` containing the specified slice, or `NULL` on failure.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 30, 40, 50};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 30, 40, 50]
// Get elements from index 1 up to (but not including) index 4
LinkedList* sliced = list_slice(list, 1, 4);
// 'sliced' is a new list: [20, 30, 40]
destroy(list);
destroy(sliced);
```

### `list_rotate`

Rotates the list elements in-place by a specified number of positions. A positive number rotates to the right (end to front), and a negative number rotates to the left (front to end). This is an O(n) operation but does not involve reallocating nodes.

**Receives:**

- `list`: The list to rotate.
- `positions`: Number of positions to rotate (positive for right, negative for left).

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 30, 40, 50};
// array_to_list(list, nums, 5);
// List: [10, 20, 30, 40, 50]
list_rotate(list, 2); // Rotate right by 2
// List is now: [40, 50, 10, 20, 30]
list_rotate(list, -1); // Rotate left by 1
// List is now: [50, 10, 20, 30, 40]
destroy(list);
```

### `list_reverse`

This function reverses the order of all elements in the list in-place by manipulating the next and prev pointers of each node. It's an efficient O(n) operation.

**Receives:**

- `list`: The list to reverse.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 30};
// array_to_list(list, nums, 3); // Assuming this function exists
// List: [10, 20, 30]
list_reverse(list); // List is now: [30, 20, 10]
destroy(list);
```

### `list_filter`

Creates a new list containing only the elements from the original list that satisfy a certain condition, defined by a `filter_fn` function.

**Receives:**

- `list`: The source list.
- `filter_fn`: A function that takes an element's data and returns `TRUE` to keep it or `FALSE` to discard it.

**Returns:**

- A new `LinkedList` with the filtered elements, or `NULL` on failure.

**Example:**

```c
// A filter function that keeps only even numbers
bool is_even(const void* data) {
    return (*(int*)data) % 2 == 0;
}

LinkedList* list = create_list(sizeof(int));
int nums[] = {1, 2, 3, 4, 5};
// array_to_list(list, nums, 5);
LinkedList* evens = list_filter(list, is_even);
// 'evens' is a new list: [2, 4]
destroy(list);
destroy(evens);
```

### `list_map`

Creates a new list by applying a transformation function to every element of the original list. This is useful for projecting your data into a different form.

**Receives:**

- `list`: The source list.
- `map_fn`: A function that takes a destination pointer and a source element's data and performs the transformation.
- `new_element_size`: The `sizeof` the elements in the new, mapped list.

**Returns:**

- A new transformed `LinkedList`, or `NULL` on failure.

**Example:**

```c
// A map function to get the age from a Person and store it as an int
void get_age(void* dest, const void* src) {
    *(int*)dest = ((Person*)src)->age;
}

LinkedList* person_list = create_list(sizeof(Person));
// ... populate with Alice (30), Bob (25) ...
// Create a new list containing only the ages
LinkedList* age_list = list_map(person_list, get_age, sizeof(int));
// 'age_list' is a new list of ints: [30, 25]
destroy(person_list);
destroy(age_list);
```

---

## 9. Mathematical Functions

### `list_min` / `list_max`

These functions find the minimum or maximum element in the list, respectively. They require a compare function to be set to determine the ordering.

**Receives:**

- `list`: The list to search in.

**Returns:**

- A direct pointer to the data of the min/max element, or `NULL` if not found or list is empty.

**Example:**

```c
LinkedList* list = create_list(sizeof(Person));
set_compare_function(list, compare_person_age);
// ... add Alice (30), Bob (25), Charlie (35) ...
Person* youngest = (Person*)list_min(list);
Person* oldest = (Person*)list_max(list);
if (youngest && oldest) {
    printf("Youngest: %s, Oldest: %s\n", youngest->name, oldest->name);
}
destroy(list);
```

### `list_unique`

This function creates a new list containing only the unique elements from the original list. It preserves the order of the first occurrence of each element. Requires a compare function.

**Receives:**

- `list`: The source list.

**Returns:**

- A new `LinkedList` with unique elements, or `NULL` on failure.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list = create_list(sizeof(int));
set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 20, 10};
// array_to_list(list, nums, 6);
// List: [10, 20, 10, 30, 20, 10]
LinkedList* unique_list = list_unique(list);
// 'unique_list' is a new list: [10, 20, 30]
destroy(list);
destroy(unique_list);
```

### `list_intersection`

This function creates a new list containing only the elements that are present in both input lists. Requires a compare function.

**Receives:**

- `list1`: The first list.
- `list2`: The second list.

**Returns:**

- A new `LinkedList` with the common elements, or `NULL` on failure.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list1 = create_list(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = create_list(sizeof(int)); // Contains [20, 40, 30]
set_compare_function(list1, compare_int);
// ... populate lists ...
LinkedList* intersection = list_intersection(list1, list2);
// 'intersection' is a new list: [20, 30]
destroy(list1);
destroy(list2);
destroy(intersection);
```

### `list_union`

This function creates a new list containing all unique elements from both input lists combined. Requires a compare function.

**Receives:**

- `list1`: The first list.
- `list2`: The second list.

**Returns:**

- A new `LinkedList` with the union of elements, or `NULL` on failure.

**Example:**

```c
// Assume a compare_int function exists for comparing integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list1 = create_list(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = create_list(sizeof(int)); // Contains [20, 40, 30]
set_compare_function(list1, compare_int);
// ... populate lists ...
LinkedList* union_list = list_union(list1, list2);
// 'union_list' is a new list: [10, 20, 30, 40]
destroy(list1);
destroy(list2);
destroy(union_list);
```

---

## 10. List \<--\> Array

### `array_to_list`

This function clears a list and then populates it with all the elements from a standard C array.

**Receives:**

- `list`: The list to populate.
- `arr`: A pointer to the array data.
- `n`: The number of elements in the array.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
int numbers[] = {10, 20, 30, 40};
LinkedList* num_list = create_list(sizeof(int));
// array_to_list(num_list, numbers, 4);
// num_list now contains [10, 20, 30, 40]
destroy(num_list);
```

### `list_to_array`

This function converts the entire linked list into a new, dynamically allocated C array. This is useful when you need to interface with other libraries or functions that expect a contiguous array.

**Receives:**

- `list`: The list to convert.
- `out_size`: A pointer to a `size_t` where the number of elements in the new array will be stored.

**Returns:**

- A `void*` pointer to the newly allocated array.

**Note:** The caller is responsible for freeing this array.

**Example:**

```c
LinkedList* num_list = create_list(sizeof(int));
// ... num_list contains [10, 20, 30, 40] ...
size_t array_size;
int* new_array = (int*)list_to_array(num_list, &array_size);
if (new_array) {
    printf("Array has %zu elements. First element: %d\n", array_size, new_array[0]);
    free(new_array); // Don't forget to free!
}
destroy(num_list);
```

---

## 11. List \<--\> String / File

### `list_to_string`

Converts the list into a single string representation, with elements separated by a custom separator. This is mainly for simple data types like `int`, `char`, `double`.

**Receives:**

- `list`: The list to convert.
- `separator`: The string to place between elements.

**Returns:**

- A newly allocated string. The caller is responsible for freeing it.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 30};
// array_to_list(list, nums, 3);
char* str = list_to_string(list, ", ");
if (str) {
    printf("List as string: %s\n", str); // Output: 10, 20, 30
    free(str);
}
destroy(list);
```

### `save_to_file`

This function serializes the list's data and saves it to a binary file. This allows for persistent storage. It saves the length and element size, followed by the raw data of each element.

**Receives:**

- `list`: The list to save.
- `filename`: The path to the output file.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* num_list = create_list(sizeof(int));
// ... n
```
