# Generic Linked List Library in C

This is a comprehensive, generic linked list library written in C. It is designed to be type-agnostic by using void pointers for data storage.

## Setup for Examples

To demonstrate the library's functionality, most examples will use a Person struct. This setup will be assumed for all subsequent examples.

```c
// The structure we will store in the list
typedef struct {
    int id;
    char* name;
    int age;
} Person;
```

For this library to work, you need to define helper functions that the list will use to manage Person objects. These functions are essential for the library to understand how to work with your specific data type.### Essential Functions (Required)

`void PrintFunction(void* data)` - This function assigns a custom printing function to the list. The `list_print` and `list_print_advanced` functions will use this function pointer to display each element in a user-defined format. Without this, the list doesn't know how to interpret and print your data structure. You must cast the void pointer back to your data type within the function.

`int CompareFunction(const void* data1, const void* data2)` - This comparison function is required for any function that needs to know the relative order of elements, such as sorting, finding an element by value, or removing specific items. The function should return a negative, zero, or positive value, following the convention of `strcmp`. Both parameters need to be cast to your data type within the function.

`void FreeFunction(void* data)` - This custom memory deallocation function is critical for preventing memory leaks when your data structure contains dynamically allocated memory (e.g., a pointer to a string). When an element is deleted, the list will call this function to free that internal memory before freeing the element itself. Cast the void pointer to your data type and free any internal allocations.

In our implementation, these functions will look like this:

```c
// 1. Function to print a Person's details
void print_person(void* data) {
   
    if (!data) return;

    // Cast the void pointer back to Person pointer
    Person* p = (Person*)data;

    printf("Id: %d, Name: %s, Age: %d\n", p->id, p->name, p->age);
}

// 2. Function to compare two Persons by age
int compare_person_age(const void* data1, const void* data2) {
   
    if (!data1 || !data2) return 0;

    // Cast the void pointers back to Person pointers
    Person* p1 = (Person*)data1;
    Person* p2 = (Person*)data2;

    return p1->age - p2->age;
}

// 3. Function to free the dynamically allocated name within a Person struct
void free_person(void* data) {

    if (!data) return;

    // Cast the void pointers back to Person pointers
    Person* p = (Person*)data;

    free(p->name); // Free the allocated string
    // The struct itself is managed by the list, so we don't free 'p'
}
```

### Optional Functions

`void CopyFunction(void* dest, const void* src)` - This function performs a "deep copy" of elements. When you insert data, the list makes a copy. By default, this is a `memcpy` (a shallow copy). If your struct contains pointers, a shallow copy would mean both the original and the list's copy point to the same memory, which is dangerous. A deep copy function allocates new memory for these pointers. לרוב זה מיותר, כי אם נבצע שינוי במקור אנחנו נרצה שזה ישתנה. אבל אם לא, אז תוכלו להגדיר את זה. זה לא חובה והספריה תעבוד גם בלי זה.

### Implementation Example

```c
// 4. Function to perform a deep copy of a Person struct
void copy_person(void* dest, const void* src) {
    if (!dest || !src) return;
    Person* dest_p = (Person*)dest;
    const Person* src_p = (const Person*)src;

    // Allocate new memory for the name and copy it
    dest_p->name = malloc(strlen(src_p->name) + 1);
    if (dest_p->name) {
        strcpy(dest_p->name, src_p->name);
    }
    dest_p->age = src_p->age;
}
```

If you want to filter or transform the elements in the list, you can define additional function pointers:

`bool FilterFunction(const void* data)`

`void MapFunction(void* dest, const void* src)`

נניח בדוגמה שלנו
נניח קוד שלוקח את כל האנשים שהם יותר מגיל 18
נניח מעלה את הגיל של כולם בעוד שנה

```c
לכתוב קוד

// 5.
נניח קוד שלוקח את כל האנשים שהם יותר מגיל 18

// 6.
נניח מעלה את הגיל של כולם בעוד שנה
```

## 1. Create List

### `list_create`

This is the starting point for using the library. It allocates memory for a new, empty `LinkedList` structure and initializes it.

**Receives:**

- `element_size`: The size in bytes of the data type you plan to store. Use the `sizeof` operator for this (e.g., `sizeof(Person)`).

**Returns:**

- A pointer to the newly created `LinkedList` on success.
- `NULL` if memory allocation fails.

**Example:**

```c
// Create a list to store Person structs
LinkedList* person_list = list_create(sizeof(Person));

// Check if the list was created successfully
if (!person_list) { // person_list == NULL
    printf("Failed to create list.\n");
    return 1;
}

printf("Successfully created a list for Person objects.\n");
```

> [!NOTE] > _For developers:_ This function sets up dummy head and tail nodes, to simplify the logic for all other list operations by ensuring that every "real" node is always between two other nodes.

> [!IMPORTANT]
> This function only creates the list. It is currently "empty" (except for the dummy nodes, of course). Later, we will learn how to add elements to it.

---

## 2. List Configuration

Before using the list, you need to configure it with the appropriate function pointers. The first three functions are essential for most operations, while the others are optional but provide additional functionality.

```c
// Essential configuration (required for most operations)
list_set_print_function(person_list, print_person);
list_set_compare_function(person_list, compare_person_age);
list_set_free_function(person_list, free_person);

// Optional configuration
list_set_copy_function(person_list, copy_person);
```

Additionally, you can set a maximum size limit for your list using `list_set_max_size()`. This function is particularly useful when implementing caches or buffers that shouldn't grow indefinitely. You can specify the maximum number of elements allowed (or `UNLIMITED` for no limit), and choose the behavior when the list reaches capacity - either reject new insertions or automatically delete the oldest elements to make room.

for example, you can set max 100 elements, auto-delete old when full:

```c
list_set_max_size(person_list, 100, DELETE_OLD_WHEN_FULL);
```

Or no size limit (the last parameter is ignored when using UNLIMITED):

```c
list_set_max_size(person_list, UNLIMITED, REJECT_NEW_WHEN_FULL);  // behavior ignored
```

> [!IMPORTANT]
> When using `UNLIMITED`, the behavior parameter is ignored since there's no capacity limit to reach.

---

## 3. Insertion in Linked List

### `list_insert_at_head`

This function adds a new element to the very beginning of the list. The new element becomes the item at index 0. This is a very fast O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to be inserted. The library makes its own copy of this data.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_head(list, &n1); // List: [10]
list_insert_at_head(list, &n2); // List: [20, 10]
list_destroy(list);
```

### `list_insert_at_tail`

This function adds a new element to the very end of the list. This is a common operation for building a list in order. It is also a very fast O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to be inserted. The library makes its own copy.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list, &n1); // List: [10]
list_insert_at_tail(list, &n2); // List: [10, 20]
list_destroy(list);
```

### `list_insert_at_index`

This function inserts an element at a specific zero-based index. All elements from that index onward are shifted one position to the right. The operation is optimized to traverse from the head or tail, whichever is closer to the target index.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The zero-based index where the element should be inserted.
- `data`: A pointer to the data to be inserted.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20, n3 = 30;
list_insert_at_tail(list, &n1); // List: [10]
list_insert_at_tail(list, &n2); // List: [10, 20]
list_insert_at_index(list, 1, &n3); // List: [10, 30, 20]
list_destroy(list);
```

---

## 4. Deletion Functions

### `list_delete_from_head`

This function removes the first element (at index 0) from the list. This is an O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list, &n1);
list_insert_at_tail(list, &n2); // List: [10, 20]
list_delete_from_head(list);   // List: [20]
list_destroy(list);
```

### `list_delete_from_tail`

This function removes the last element from the list. This is an O(1) operation.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list, &n1);
list_insert_at_tail(list, &n2); // List: [10, 20]
list_delete_from_tail(list);   // List: [10]
list_destroy(list);
```

### `list_delete_at_index`

This function removes an element at a specific zero-based index. Subsequent elements are shifted to the left.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to delete.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20, n3 = 30;
list_insert_at_tail(list, &n1);
list_insert_at_tail(list, &n2);
list_insert_at_tail(list, &n3); // List: [10, 20, 30]
list_delete_at_index(list, 1); // List: [10, 30]
list_destroy(list);
```

### `list_remove_advanced`

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

LinkedList* list = list_create(sizeof(int));
list_set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 10};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 10, 30, 10]
int target = 10;
// Remove all occurrences of 10
list_remove_advanced(list, &target, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
// List is now: [20, 30]
list_destroy(list);
```

### `list_clear`

This function efficiently removes all elements from the list, resetting its length to zero. It properly frees the memory for each element using the configured free function.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list, &n1);
list_insert_at_tail(list, &n2); // List: [10, 20]
list_clear(list); // List: []
printf("Is list empty? %s\n", list_is_empty(list) ? "Yes" : "No"); // Output: Yes
list_destroy(list);
```

### `list_destroy`

This function is the final cleanup step. It completely deallocates all memory used by the list, including clearing all elements (using `list_clear`), freeing the dummy nodes, and finally freeing the `LinkedList` structure itself. After calling this, the list pointer is no longer valid.

**Receives:**

- `list`: A pointer to the `LinkedList` to be destroyed.

**Returns:**

- Nothing.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n = 10;
list_insert_at_tail(list, &n);
// ... use the list ...
list_destroy(list); // All memory is freed.
// list = NULL; // Good practice to nullify the pointer after destroying.
```

---

## 5. Utility Functions

### `list_get_length`

A straightforward function that returns the number of elements currently stored in the list. This is an O(1) operation as the length is tracked internally.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- The number of elements as a `size_t`.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list, &n1);
list_insert_at_tail(list, &n2);
printf("List length: %zu\n", list_get_length(list)); // Output: 2
list_destroy(list);
```

### `list_is_empty`

A simple check to see if the list contains any elements. It's slightly more expressive than checking `list_get_length(list) == 0`.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `TRUE` if the list has zero elements, `FALSE` otherwise.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
printf("Is list empty? %s\n", list_is_empty(list) ? "Yes" : "No"); // Output: Yes
int n = 10;
list_insert_at_tail(list, &n);
printf("Is list empty? %s\n", list_is_empty(list) ? "Yes" : "No"); // Output: No
list_destroy(list);
```

### `list_print`

This function iterates through the list and prints all elements to the console. It relies on a print function being set via `list_set_print_function`. It provides a default format with indices.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or `LIST_ERROR_NO_PRINT_FUNCTION` if no print function is set.

**Example:**

```c
LinkedList* list = list_create(sizeof(Person));
list_set_print_function(list, print_person);
Person alice = {.name = strdup("Alice"), .age = 30};
list_insert_at_tail(list, &alice);
// Output:
// List len: 1
//   [0]: Name: Alice, Age: 30
list_print(list);
free(alice.name);
list_destroy(list);
```

### `list_print_advanced`

This function provides more control over the output format than `list_print`. You can choose whether to display indices and specify a custom separator string to be printed between elements.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `show_index`: If `TRUE`, prints the `[index]:` prefix for each element.
- `separator`: The string to print between elements (e.g., `", "`, `" -> "`).

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = list_create(sizeof(Person));
list_set_print_function(list, print_person);
Person alice = {.name = strdup("Alice"), .age = 30};
Person bob = {.name = strdup("Bob"), .age = 25};
list_insert_at_tail(list, &alice);
list_insert_at_tail(list, &bob);
// Output: Name: Alice, Age: 30 ---> Name: Bob, Age: 25
list_print_advanced(list, FALSE, " ---> ");
free(alice.name);
free(bob.name);
list_destroy(list);
```

---

## 6. Search and Access Functions

### `list_get`

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
LinkedList* list = list_create(sizeof(Person));
Person alice = {.name = strdup("Alice"), .age = 30};
list_insert_at_tail(list, &alice);
Person* p = (Person*)list_get(list, 0);
if (p) {
    printf("Person at index 0 is %s.\n", p->name); // Output: Alice
}
free(alice.name);
list_destroy(list);
```

### `list_set`

This function updates the element at a specific index with new data. It overwrites the existing data at that position. It properly frees the old data using the configured free function before copying the new data.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to update.
- `data`: A pointer to the new data.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(Person));
// Setup copy and free functions for Person
list_set_copy_function(list, copy_person);
list_set_free_function(list, free_person);
Person alice = {.name = strdup("Alice"), .age = 30};
list_insert_at_tail(list, &alice); // List: [Alice (30)]
Person alice_new = {.name = strdup("Alice"), .age = 31};
list_set(list, 0, &alice_new); // List: [Alice (31)]
free(alice.name);
free(alice_new.name);
list_destroy(list);
```

### `list_index`

This function searches the list from head to tail and returns the index of the first element that matches the provided data. It requires a compare function to be set.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `data`: A pointer to the data to search for.

**Returns:**

- The non-negative index of the element if found.
- A negative error code if not found or an error occurs.

**Example:**

```c
LinkedList* list = list_create(sizeof(Person));
list_set_compare_function(list, compare_person_age);
Person alice = {.name = strdup("Alice"), .age = 30};
Person bob = {.name = strdup("Bob"), .age = 25};
list_insert_at_tail(list, &bob);
list_insert_at_tail(list, &alice); // List: [Bob (25), Alice (30)]
Person target = {.age = 30}; // We only need age for the comparison
int index = list_index(list, &target);
if (index >= 0) {
    printf("A person aged 30 is at index: %d\n", index); // Output: 1
}
free(alice.name);
free(bob.name);
list_destroy(list);
```

### `list_index_advanced`

Similar to `list_index`, but allows you to specify the search direction. You can search from the head (for the first match) or from the tail (for the last match).

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

LinkedList* list = list_create(sizeof(int));
list_set_compare_function(list, compare_int);
int nums[] = {10, 20, 30, 20, 40};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 30, 20, 40]
int target = 20;
int first_idx = list_index_advanced(list, &target, SEARCH_FROM_HEAD); // Result: 1
int last_idx = list_index_advanced(list, &target, SEARCH_FROM_TAIL);  // Result: 3
printf("First 20 is at index %d, last 20 is at index %d\n", first_idx, last_idx);
list_destroy(list);
```

### `list_count_occurrences`

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

LinkedList* list = list_create(sizeof(int));
list_set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 10};
// array_to_list(list, nums, 5);
int target = 10;
size_t count = list_count_occurrences(list, &target);
printf("The number 10 appears %zu times.\n", count); // Output: 3
list_destroy(list);
```

---

## 7. Sorting Functions

### `list_sort`

This function sorts the list in-place using the configured compare function. It is implemented by converting the list to an array, running the standard library's highly efficient `qsort` (O(n log n)), and then rebuilding the list from the sorted array.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `reverse_order`: If `TRUE`, sorts in descending order; otherwise, sorts in ascending order.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
LinkedList* list = list_create(sizeof(Person));
list_set_print_function(list, print_person);
list_set_compare_function(list, compare_person_age);
// ... add Alice (30), Bob (25), Charlie (35) ...
list_sort(list, FALSE); // Sort ascending by age
printf("Sorted list (ascending):\n");
// list_print(list); // Output: Bob (25), Alice (30), Charlie (35)

list_sort(list, TRUE); // Sort descending by age
printf("\nSorted list (descending):\n");
// list_print(list); // Output: Charlie (35), Alice (30), Bob (25)
list_destroy(list);
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
LinkedList* original = list_create(sizeof(int));
// ... populate original list ...
LinkedList* copy = list_copy(original);
// 'copy' is now a separate list with the same elements.
// Modifying 'copy' will not affect 'original'.
list_delete_from_head(copy);
printf("Original length: %zu, Copy length: %zu\n", list_get_length(original), list_get_length(copy));
list_destroy(original);
list_destroy(copy);
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
LinkedList* list1 = list_create(sizeof(int));
LinkedList* list2 = list_create(sizeof(int));
int n1 = 10, n2 = 20;
list_insert_at_tail(list1, &n1); // list1: [10]
list_insert_at_tail(list2, &n2); // list2: [20]
list_extend(list1, list2); // list1 is now [10, 20]
list_destroy(list1);
list_destroy(list2);
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
LinkedList* list1 = list_create(sizeof(int)); // Contains [10, 20]
LinkedList* list2 = list_create(sizeof(int)); // Contains [30, 40]
// ... populate lists ...
LinkedList* concatenated = list_concat(list1, list2);
// 'concatenated' is a new list: [10, 20, 30, 40]
list_destroy(list1);
list_destroy(list2);
list_destroy(concatenated);
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
LinkedList* list = list_create(sizeof(int));
int nums[] = {10, 20, 30, 40, 50};
// array_to_list(list, nums, 5); // Assuming this function exists
// List: [10, 20, 30, 40, 50]
// Get elements from index 1 up to (but not including) index 4
LinkedList* sliced = list_slice(list, 1, 4);
// 'sliced' is a new list: [20, 30, 40]
list_destroy(list);
list_destroy(sliced);
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
LinkedList* list = list_create(sizeof(int));
int nums[] = {10, 20, 30, 40, 50};
// array_to_list(list, nums, 5);
// List: [10, 20, 30, 40, 50]
list_rotate(list, 2); // Rotate right by 2
// List is now: [40, 50, 10, 20, 30]
list_rotate(list, -1); // Rotate left by 1
// List is now: [50, 10, 20, 30, 40]
list_destroy(list);
```

### `list_reverse`

This function reverses the order of all elements in the list in-place by manipulating the next and prev pointers of each node. It's an efficient O(n) operation.

**Receives:**

- `list`: The list to reverse.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* list = list_create(sizeof(int));
int nums[] = {10, 20, 30};
// array_to_list(list, nums, 3); // Assuming this function exists
// List: [10, 20, 30]
list_reverse(list); // List is now: [30, 20, 10]
list_destroy(list);
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

LinkedList* list = list_create(sizeof(int));
int nums[] = {1, 2, 3, 4, 5};
// array_to_list(list, nums, 5);
LinkedList* evens = list_filter(list, is_even);
// 'evens' is a new list: [2, 4]
list_destroy(list);
list_destroy(evens);
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

LinkedList* person_list = list_create(sizeof(Person));
// ... populate with Alice (30), Bob (25) ...
// Create a new list containing only the ages
LinkedList* age_list = list_map(person_list, get_age, sizeof(int));
// 'age_list' is a new list of ints: [30, 25]
list_destroy(person_list);
list_destroy(age_list);
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
LinkedList* list = list_create(sizeof(Person));
list_set_compare_function(list, compare_person_age);
// ... add Alice (30), Bob (25), Charlie (35) ...
Person* youngest = (Person*)list_min(list);
Person* oldest = (Person*)list_max(list);
if (youngest && oldest) {
    printf("Youngest: %s, Oldest: %s\n", youngest->name, oldest->name);
}
list_destroy(list);
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

LinkedList* list = list_create(sizeof(int));
list_set_compare_function(list, compare_int);
int nums[] = {10, 20, 10, 30, 20, 10};
// array_to_list(list, nums, 6);
// List: [10, 20, 10, 30, 20, 10]
LinkedList* unique_list = list_unique(list);
// 'unique_list' is a new list: [10, 20, 30]
list_destroy(list);
list_destroy(unique_list);
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

LinkedList* list1 = list_create(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = list_create(sizeof(int)); // Contains [20, 40, 30]
list_set_compare_function(list1, compare_int);
// ... populate lists ...
LinkedList* intersection = list_intersection(list1, list2);
// 'intersection' is a new list: [20, 30]
list_destroy(list1);
list_destroy(list2);
list_destroy(intersection);
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

LinkedList* list1 = list_create(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = list_create(sizeof(int)); // Contains [20, 40, 30]
list_set_compare_function(list1, compare_int);
// ... populate lists ...
LinkedList* union_list = list_union(list1, list2);
// 'union_list' is a new list: [10, 20, 30, 40]
list_destroy(list1);
list_destroy(list2);
list_destroy(union_list);
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
LinkedList* num_list = list_create(sizeof(int));
// array_to_list(num_list, numbers, 4);
// num_list now contains [10, 20, 30, 40]
list_destroy(num_list);
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
LinkedList* num_list = list_create(sizeof(int));
// ... num_list contains [10, 20, 30, 40] ...
size_t array_size;
int* new_array = (int*)list_to_array(num_list, &array_size);
if (new_array) {
    printf("Array has %zu elements. First element: %d\n", array_size, new_array[0]);
    free(new_array); // Don't forget to free!
}
list_destroy(num_list);
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
LinkedList* list = list_create(sizeof(int));
int nums[] = {10, 20, 30};
// array_to_list(list, nums, 3);
char* str = list_to_string(list, ", ");
if (str) {
    printf("List as string: %s\n", str); // Output: 10, 20, 30
    free(str);
}
list_destroy(list);
```

### `list_save_to_file`

This function serializes the list's data and saves it to a binary file. This allows for persistent storage. It saves the length and element size, followed by the raw data of each element.

**Receives:**

- `list`: The list to save.
- `filename`: The path to the output file.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
LinkedList* num_list = list_create(sizeof(int));
// ... n
```
