# Generic Linked List Library in C

This is a comprehensive, generic linked list library written in C. It is designed to be type-agnostic by using void pointers for data storage, with a **simplified API for field setting** that eliminates the need to specify struct types repeatedly.

## Key Features
להשלים!

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

For this library to work, you need to define **helper functions** in your own code that the list will use to manage your objects. These functions are essential for the library to understand how to work with your specific data type.

1. <a id="print-function"></a>`void PrintFunction(void* data)` - This function assigns a custom printing function to the list. You don't need to print a newline at the end of your print function, becase the library automatically do this for you.

2. <a id="free-function"></a>`void FreeFunction(void* data)` – This function frees all dynamically allocated memory **inside** your struct, such as strings or arrays allocated with malloc. (The library itself manages the memory of the struct itself).

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

// 2. Function to free the dynamically allocated name within a Person struct
void free_person(void* data) {

    // Cast the void pointers back to Person pointers
    Person* p = (Person*)data;

    free(p->name); // Free the allocated string

    // The struct itself is managed by the list, so we don't free 'p'
}
```

> [!NOTE]
> If your struct does **not** contain any dynamically allocated fields (such as pointers to memory allocated with `malloc`), you do **not** need to implement the last function at all.

<br></br>

## 1. Create List

### `create_list`

`LinkedList* create_list(size_t element_size);`

This function allocates memory for a new, empty `LinkedList` structure and initializes it.

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
> This function sets up dummy head and tail nodes, to simplify the logic for all other list operations by ensuring that every "real" node is always between two other nodes.This function only creates the list. It is currently "empty" (except for the dummy nodes, of course). Later, we will learn how to [add elements to it](#3-insertion-in-linked-list).

<br></br>

## 2. List Configuration

### `set_print_function` and `set_free_function`
Before using the list, you need to configure it with the [helper functions](#setup-for-examples), by calling the following functions:

```c
set_print_function(person_list, print_person);
set_free_function(person_list, free_person);
```

### `set_list_struct_name`
After setting the print and free functions, you need to set the struct name using `set_list_struct_name()`. This allows you to use the `set_field_value()` and `set_field_ptr()` functions without needing to specify the struct name each time.

```c
set_list_struct_name(person_list, "Person");
```

### `set_max_size`
Additionally, you can set a maximum size limit for your list using `set_max_size()`. This function is particularly useful when implementing caches or buffers that shouldn't grow indefinitely. You can specify the maximum number of elements allowed (or `UNLIMITED` for no limit), and choose the behavior when the list reaches capacity - either reject new insertions or automatically delete the oldest elements to make room.

for example, you can set max 100 elements, auto-delete old when full:

```c
set_max_size(person_list, 100, DELETE_OLD_WHEN_FULL);
```

Or no size limit (this is also the default behavior):

```c
set_max_size(person_list, UNLIMITED, REJECT_NEW_WHEN_FULL);
```

> [!NOTE]
> When using `UNLIMITED`, the behavior parameter is ignored since there's no capacity limit to reach.

<br></br>

## 3. Insertion in Linked List

You can insert structs into the list in two ways:

- **Pass the struct itself by value** – You create your struct as a regular stack variable (not with `malloc`). When you insert it, the library automatically copies its contents into its own internal storage on the heap. You don't need to allocate or free memory for the struct itself, just build it on the stack and pass it to the insertion function.

- **Allocate memory for the struct and pass a pointer** – you allocate your struct on the heap (using `malloc` or similar), and then pass its pointer to the insertion function. The library will store the pointer itself and use it as the address of the element in the list. In this approach, you are responsible for allocating the memory for your struct, and the library will manage freeing it when you delete elements or destroy the list.

> [!NOTE]
> Even if you want the library to handle dynamic allocation of the struct itself (by passing it by value), you are still responsible for allocating any fields inside your struct that require dynamic memory (such as a `char* name` field). Passing by value does **not** exempt you from handling internal allocations.

> [!NOTE]
> Regardless of whether you insert your struct by value or by pointer, the library will always take care of freeing all memory associated with each element when it is deleted or when the list is destroyed. This includes both the memory allocated for the struct itself, and any dynamically allocated fields inside your struct (such as strings or arrays), which are freed using your provided [second helper functions](#free-function).

Both approaches support the same insertion positions:

| Insertion Type   | Value-Based Function                     | Pointer-Based Function                |
|------------------|------------------------------------------|--------------------------------------|
| **Head**         | `insert_head_value(list, value)`         | `insert_head_ptr(list, ptr)`          |
| **Tail**         | `insert_tail_value(list, value)`         | `insert_tail_ptr(list, ptr)`          |
| **Index**        | `insert_index_value(list, index, value)` | `insert_index_ptr(list, index, ptr)`  |

For the next examples, we will assume you have the following helper function to create `Person` structs. Pay attention that the function returns the struct by value.

```c
Person create_person(int id, const char* name, int age) {

    Person p;

    p.id = id;
    p.age = age;
    p.name = malloc(strlen(name) + 1);
    if (p.name) strcpy(p.name, name);

    return p;
}
```

### `insert_head_value`

`ListResult insert_head_value(LinkedList* list, void* value);`

Adds a new element to the beginning of the list by value.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `value`: The struct value to be inserted (not a pointer).

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person alice = create_person(1012, "Alice Johnson", 16);
insert_head_value(people_list, alice);
```

### `insert_head_ptr`

`ListResult insert_head_ptr(LinkedList* list, void* data_ptr);`

Adds a new element to the beginning of the list by pointer.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `ptr`: A pointer to a struct that was allocated on the heap.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person* diana = (Person*)malloc(sizeof(Person));
*diana = create_person(1017, "Diana Prince", 13);
insert_head_ptr(people_list, diana);
```

### `insert_tail_value`

`ListResult insert_tail_value(LinkedList* list, void* value);`

Adds a new element to the end of the list by value.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `value`: The struct value to be inserted.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person bob = create_person(1010, "Bob Smith", 35);
insert_tail_value(people_list, bob);
```


### `insert_tail_ptr`

`ListResult insert_tail_ptr(LinkedList* list, void* data_ptr);`

Adds a new element to the end of the list by pointer.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `ptr`: A pointer to a struct that was allocated on the heap.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person* emily = (Person*)malloc(sizeof(Person));
*emily = create_person(1042, "Emily Davis", 22);
insert_tail_ptr(people_list, emily);
```

### `insert_index_value`

`ListResult insert_index_value(LinkedList* list, size_t index, void* value);`

Inserts a new element at a specific zero-based index by value.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index to insert at.
- `value`: The struct value to be inserted.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person charlie = create_person(1098, "Charlie Brown", 11);
insert_index_value(people_list, 1, charlie); 
```

### `insert_index_ptr`

`ListResult insert_index_ptr(LinkedList* list, size_t index, void* data_ptr);`

This function inserts an element at a specific zero-based index. All elements from that index onward are shifted one position to the right. The operation is optimized to traverse from the head or tail, whichever is closer to the target index.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The zero-based index where the element should be inserted.
- `ptr`: A pointer to a struct that was allocated on the heap.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person* frank = (Person*)malloc(sizeof(Person));
*frank = create_person(1033, "Frank Wilson", 31);
insert_index_ptr(people_list, 3, frank);
```

<br></br>

## 4. Deletion Functions

### `delete_head`

`ListResult delete_head(LinkedList* list);`

This function removes the first element (at index 0) from the list.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
delete_head(people_list);
```

### `delete_tail`

`ListResult delete_tail(LinkedList* list);`

This function removes the last element from the list.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or an error code if the list is empty.

**Example:**

```c
delete_tail(people_list);
```

### `delete_index`

`ListResult delete_index(LinkedList* list, size_t index);`

This function removes an element at a specific zero-based index. Subsequent elements are shifted to the left.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to delete.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
delete_index(people_list, 1); // Deletes the *second* element
```

### `remove_advanced`

`ListResult remove_advanced(LinkedList* list, int count, Direction direction, FilterFunction predicate);`

This is a powerful function that finds and removes elements based on their value. It requires a compare function to be set. You can control how many matching elements to remove and from which direction to start the search.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `count`: The number of occurrences to remove. Use `DELETE_ALL_OCCURRENCES` to remove all matches.
- `order`: `START_FROM_HEAD` or `START_FROM_TAIL` (direction to begin searching).
- `predicate`: A function that takes a pointer to an element and returns `TRUE` if it should be removed, `FALSE` otherwise.

**Returns:**

- `LIST_SUCCESS` if at least one element was removed, otherwise `LIST_ERROR_ELEMENT_NOT_FOUND`.

**Example:**

The following example removes all people whose name contains the letter 'E' or 'e'.

First, let's write the helper function that checks if a Person's name contains the letter 'E' or 'e':

```c
bool is_person_name_contains_e(const void* struct) {
    const Person* p = (const Person*)struct;

    for (const char* c = p->name; *c; ++c)
        if (*c == 'E' || *c == 'e') return true;
    
    return false;
}
```

Now, you can use this function with `remove_advanced` to remove all matching elements from the list:

```c
remove_advanced(people_list, DELETE_ALL_OCCURRENCES, START_FROM_HEAD, is_person_name_contains_e);
```

**Example:**

This next example removes up to 3 people whose ID is divisible by 2, starting from the tail of the list:

```c
bool is_person_id_divisible_by_2(const void* element) {
    const Person* p = (const Person*)element;
    return (p->id % 2) == 0;
}
```

```c
remove_advanced(people_list, 3, START_FROM_TAIL, is_person_id_divisible_by_2);
```

These examples show how you can use your own filter functions to control which elements are removed from the list.

### `clear`

`ListResult clear(LinkedList* list);`

This function removes all elements from the list, resetting its length to zero. It properly frees the memory for each element using the configured free function.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

```c
clear(people_list);
```

### `destroy`

`void destroy(LinkedList* list);`

This function is the final cleanup step. It completely deallocates all memory used by the list, including clearing all elements (using `clear`), freeing the dummy nodes, and finally freeing the `LinkedList` structure itself.

**Receives:**

- `list`: A pointer to the `LinkedList` to be destroyed.

**Returns:**

- Nothing.

**Example:**

```c
destroy(people_list);
```

> [!WARNING]
> After calling `destroy`, the list pointer becomes invalid and should not be used again. You should set it to `NULL` to avoid dangling pointers.

<br></br>

## 5. Utility Functions

### `get_length`

`size_t get_length(const LinkedList* list);`

Function that returns the number of elements currently stored in the list.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- The number of elements as a `size_t`.

**Example:**

```c
printf("List length: %zu\n", get_length(people_list));
```

### `is_empty`

`bool is_empty(const LinkedList* list);`

A simple check to see if the list contains any elements.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `TRUE` if the list has zero elements, `FALSE` otherwise.

**Example:**

```c
printf("Is list empty? %s\n", is_empty(people_list) ? "Yes" : "No");
```

### `print_list`

`ListResult print_list(const LinkedList* list);`

This function iterates through the list and prints all elements to the console. It relies on a [print function](#print-function) being set via [`set_print_function`](#set-print-function). It provides a default format with indices.

**Receives:**

- `list`: A pointer to the `LinkedList`.

**Returns:**

- `LIST_SUCCESS` on success, or `LIST_ERROR_NO_PRINT_FUNCTION` if no print function is set.

**Example:**

```c
print_list(people_list);
// Output:
// [0]: {ID:1017, Name:"Diana Prince", Age:13}
// [1]: {ID:1098, Name:"Charlie Brown", Age:11}
// ... and so on for each element
```

> [!NOTE]
> As I wrote earlier, you do not need to manually print a newline after each element in your print function. By default `print_list` function automatically prints each structure on a separate line. If you want to customize the output format (for example, print elements without newlines or with a different separator), use the advanced printing function described below.

### `print_list_advanced`

`ListResult print_list_advanced(const LinkedList* list, bool show_size, bool show_index, const char* separator);`

This function provides more control over the output format than `print`. You can choose whether to display indices and specify a custom separator string to be printed between elements.

**Receives:**
- `list`: A pointer to the `LinkedList`.
- `show_size`: If `TRUE`, prints the total number of elements before the list.
- `show_index`: If `TRUE`, prints the `[index]:` prefix for each element.
- `separator`: The string to print between elements (e.g., `", "`, `" -> "`).

**Returns:**

- `LIST_SUCCESS` on success.

**Example:**

let's say you want to print the list length, omit indices, and separate elements with commas:

```c
print_list_advanced(people_list, true, false, ", ");
// Output:
// List len: 6
// {ID:1017, Name:"Diana Prince", Age:13}, {ID:1098, Name:"Charlie Brown", Age:11}, ...
```

<br></br>

## 6. Search and Access Functions

### `get`

`void* get(const LinkedList* list, size_t index);`

This function retrieves a direct pointer to the data stored at a specific index. This is for read-only access and does not make a copy.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `index`: The index of the element to retrieve.

**Returns:**

- A `void*` pointer to the data at the specified index.
- `NULL` on failure (e.g., index out of bounds).

**Warning:** The returned pointer is only valid as long as the list is not modified. Deleting elements could invalidate it.

**Example:**

```c
Person* first_person = (Person*)get(people_list, 0);
if (first_person) {
    printf("The ID of the first person is: %d\n", first_person->id);
}
```

### `index_of`

`int index_of(const LinkedList* list, PredicateFunction predicate);`

This function searches the list from head to tail and returns the index of the first element that satisfies the given `predicate`.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `predicate`: A function that takes an element and returns `true` if it's a match.

**Returns:**

- The index of the first matching element, or a negative error code if not found.

**Example:**

To find the index of the first person named "Charlie Brown", you would need a specific predicate function like this:

```c
bool is_charlie_brown(const void* element) {
    const Person* p = (const Person*)element;
    return strcmp(p->name, "Charlie Brown") == 0;
}
```

And then you can use it with `index_of`:

```c
int index = index_of(people_list, is_charlie_brown);

if (index >= 0) {
    printf("Found 'Charlie Brown' at index %d.\n", index);
} else {
    printf("'Charlie Brown' not found.\n");
}
```

### `index_of_advanced`

`int index_of_advanced(const LinkedList* list, Direction direction, PredicateFunction predicate);`

Similar to `index_of`, but allows you to specify the search direction (`START_FROM_HEAD` or `START_FROM_TAIL`).

**Receives:**

- `list`: The list to search in.
- `direction`: The direction to search.
- `predicate`: A function that returns `true` for a matching element.

**Returns:**

- The index if found, or a negative error code.

**Example:**

To find the *last* person in the list who is a minor (under 18), you would define a predicate function like this:

```c
bool is_minor(const void* element) {
    const Person* p = (const Person*)element;
    return p->age < 18;
}
```

And then use it with `index_of_advanced`:

```c
int last_minor_index = index_of_advanced(people_list, START_FROM_TAIL, is_minor);

if (last_minor_index >= 0) {
    Person* minor = (Person*)get(people_list, last_minor_index);
    printf("The last minor found is '%s'.\n", minor->name);
} else {
    printf("No minors found in the list.\n");
}
```

### `count_matching`

`size_t count_matching(const LinkedList* list, PredicateFunction predicate);`

This function iterates through the entire list and counts how many elements satisfy the given `predicate`.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `predicate`: A function that takes an element and returns `true` if it satisfies the condition, and `false` otherwise.

**Returns:**

- The number of elements that satisfy the condition.

**Example:**

To count how many people have names with 10 or fewer characters, you would define a predicate function like this:

```c
bool has_short_name(const void* element) {
    const Person* p = (const Person*)element;
    return p->name && strlen(p->name) <= 10;
}
```

And then use it with `count_matching`:

```c
size_t short_names_count = count_matching(people_list, has_short_name);
printf("Number of people with short names (10 characters or less): %zu\n", short_names_count);
```

### `set_field_value`

`ListResult set_field_value(LinkedList* list, field_name, index, new_value);`

This function updates a field of a struct stored at a specific index in the list. 

> [!NOTE]
>> This function uses simple `memcpy` and does NOT free old memory or allocate new memory. For pointer fields that require memory management, use [`set_field_ptr`](#set_field_ptr).

**Receives:**

- `list`: A pointer to the `LinkedList` (must have struct name set with `set_list_struct_name`).
- `field_name`: The name of the field to update.
- `index`: The index of the element to modify.
- `new_value`: The new value to assign to the field.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
ListResult result = set_field_value(people_list, age, 1, 99);
if (result == LIST_SUCCESS) {
    printf("Age updated successfully\n");
} else {
    printf("Failed to update age: %s\n", error_string(result));
}

result = set_field_value(people_list, id, 2, 5555);
if (result != LIST_SUCCESS) {
    printf("Failed to update ID: %s\n", error_string(result));
}
```

> **Legacy Support**: The old name `set_field()` still works for backward compatibility but is deprecated.

### `set_field_ptr`

`ListResult set_field_ptr(LinkedList* list, field_name, index, new_value, should_free_old, should_alloc_new, data_size);`

This function provides full control over memory management when setting pointer fields. It uses the simplified syntax with the struct name from the list configuration.

**Receives:**

- `list`: A pointer to the `LinkedList` (must have struct name set with `set_list_struct_name`).
- `field_name`: The name of the field to update.
- `index`: The index of the element to modify.
- `new_value`: The new value or data to assign.
- `should_free_old`: `true` to free existing memory, `false` to leave it.
- `should_alloc_new`: `true` to allocate new memory and copy data, `false` to use provided pointer.
- `data_size`: Size of data to allocate (when `should_alloc_new` is `true`).

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Examples:**

```c
// Allocate new memory and copy string data
const char* new_name = "David Parker";
ListResult result = set_field_ptr(people_list, name, 0, new_name, true, true, strlen(new_name) + 1);
if (result != LIST_SUCCESS) {
    printf("Failed to update name: %s\n", error_string(result));
}

// Just replace pointer (for pre-allocated data)
char* pre_allocated = malloc(50);
strcpy(pre_allocated, "Pre-allocated");
result = set_field_ptr(people_list, name, 0, pre_allocated, true, false, 0);
if (result != LIST_SUCCESS) {
    printf("Failed to update with pre-allocated data: %s\n", error_string(result));
    free(pre_allocated); // Clean up on failure
}

// Free old memory and set to NULL
result = set_field_ptr(people_list, name, 0, NULL, true, false, 0);

// Simple assignment without memory management (same as set_field_value)
result = set_field_ptr(people_list, age, 0, 30, false, false, 0);
```

> **Legacy Support**: The old name `set_field_advanced()` still works for backward compatibility but is deprecated.

### `set_node_value`

`ListResult set_node_value(LinkedList* list, index, new_value);`

This function replaces an entire struct at the specified index with a new value.

> [!NOTE]
> This function uses simple `memcpy` and does NOT free old memory or allocate new memory. For replacing with heap-allocated data, use [`set_node_ptr`](#set_node_ptr).

**Receives:**

- `list`: A pointer to the `LinkedList` (must have struct name set with `set_list_struct_name`).
- `index`: The index of the element to replace.
- `new_value`: The new struct value to replace the existing one.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
Person new_person = create_person(9999, "New Person", 50);
ListResult result = set_node_value(people_list, 0, new_person);
if (result == LIST_SUCCESS) {
    printf("Node replaced successfully\n");
} else {
    printf("Failed to replace node: %s\n", error_string(result));
    // Clean up the new_person's allocated memory if operation failed
    free_person(&new_person);
}
```

> **Legacy Support**: The old name `set_node()` still works for backward compatibility but is deprecated.

### `set_node_ptr`

`ListResult set_node_ptr(LinkedList* list, index, new_value_ptr);`

This function replaces an entire struct at the specified index with data from a heap-allocated pointer.

**Receives:**

- `list`: A pointer to the `LinkedList` (must have struct name set with `set_list_struct_name`).
- `index`: The index of the element to replace.
- `new_value_ptr`: Pointer to the new struct data (will be copied and then freed).

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Example:**

```c
// Replace with heap-allocated data
Person* heap_person = malloc(sizeof(Person));
*heap_person = create_person(8888, "Heap Person", 35);
ListResult result = set_node_ptr(people_list, 0, heap_person);
if (result == LIST_SUCCESS) {
    printf("Node replaced successfully\n");
    // heap_person is automatically freed after copying
} else {
    printf("Failed to replace node: %s\n", error_string(result));
    // Clean up manually if operation failed
    free_person(heap_person);
    free(heap_person);
}
```

<br></br>

## 7. Sorting Functions

### `sort_list`

`ListResult sort_list(LinkedList* list, CompareFunction compare_fn);`

This function sorts the list in-place using the configured compare function.

**Receives:**

- `list`: A pointer to the `LinkedList`.
- `compare_fn`: A function that compares two elements and returns:
  - A positive value if the first element is greater than the second.
  - Zero if they are equal.
  - A negative value if the first element is less than the second.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Examples:**

To sort the list by age in ascending order, you would define a compare function like this:

```c
int compare_person_age(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->age - p2->age);
}
```

And then call `sort_list`:

```c
sort_list(people_list, compare_person_age);
```

To sort by ID in ascending order, you could use this compare function:

```c
int compare_person_id(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->id - p2->id);
}
```

And then call `sort_list`:

```c
sort_list(people_list, compare_person_id);
```

Similarly, to sort by name in alphabetical order, you could use this compare function:

```c
int compare_person_name(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    if (!p1->name && !p2->name) return 0;
    if (!p1->name) return -1;
    if (!p2->name) return 1;
    return strcmp(p1->name, p2->name);
}
```

And then call `sort_list`:

```c
sort_list(people_list, compare_person_name);
```

<br></br>

## 8. Structural Transformations

### `list_copy`

`LinkedList* copy(const LinkedList* list);`

This function creates a new, independent copy of an entire list. If a `copy_fn` is configured, it will be used to deep copy each element, ensuring the new list is completely separate from the original. Without a copy function, it performs a shallow copy using `memcpy`.

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

`ListResult extend(LinkedList* list, const LinkedList* other);`

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

`LinkedList* concat(const LinkedList* list1, const LinkedList* list2);`

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

`LinkedList* slice(const LinkedList* list, size_t start, size_t end);`

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

`ListResult rotate(LinkedList* list, int positions);`

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

`ListResult reverse(LinkedList* list);`

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

`LinkedList* filter(const LinkedList* list, FilterFunction filter_fn);`

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

`LinkedList* map(const LinkedList* list, MapFunction map_fn, size_t new_element_size);`

Creates a new list by applying a transformation function to every element of the original list. This is useful for projecting your data into a different form.

**Receives:**

- `list`: The source list.
- `map_fn`: A function that takes a destination pointer and a source element's data and performs the transformation.
- `new_element_size`: The `sizeof` the elements in the new, mapped list.

<br></br>

## 9. Mathematical Functions

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

<br></br>

## 9. Mathematical Functions

### `min_by` / `max_by`

`void* min_by(const LinkedList* list, int (*compare)(const void *a, const void *b));`
`void* max_by(const LinkedList* list, int (*compare)(const void *a, const void *b));`

These functions find the minimum or maximum element in the list, respectively. They require a compare function to determine the ordering.

**Receives:**

- `list`: The list to search in.
- `compare`: A function that compares two elements and returns a negative, zero, or positive value.

**Returns:**

- A direct pointer to the data of the min/max element, or `NULL` if not found or list is empty.

**Example:**

```c
// Compare function for Person age
int compare_person_age(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->age - p2->age);
}

LinkedList* person_list = create_list(sizeof(Person));
// ... add Alice (30), Bob (25), Charlie (35) ...
Person* youngest = (Person*)min_by(person_list, compare_person_age);
Person* oldest = (Person*)max_by(person_list, compare_person_age);
if (youngest && oldest) {
    printf("Youngest: %s, Oldest: %s\n", youngest->name, oldest->name);
}
destroy(person_list);
```

### `unique`

`LinkedList* unique(const LinkedList* list, CompareFunction compare_fn);`

This function creates a new list containing only the unique elements from the original list. It preserves the order of the first occurrence of each element.

**Receives:**

- `list`: The source list.
- `compare_fn`: A function to compare elements for equality.

**Returns:**

- A new `LinkedList` with unique elements, or `NULL` on failure.

**Example:**

```c
// Compare function for integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 10, 30, 20, 10};
from_array(list, nums, 6);
// List: [10, 20, 10, 30, 20, 10]
LinkedList* unique_list = unique(list, compare_int);
// 'unique_list' is a new list: [10, 20, 30]
destroy(list);
destroy(unique_list);
```

### `intersection`

`LinkedList* intersection(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn);`

This function creates a new list containing only the elements that are present in both input lists.

**Receives:**

- `list1`: The first list.
- `list2`: The second list.
- `compare_fn`: A function to compare elements for equality.

**Returns:**

- A new `LinkedList` with the common elements, or `NULL` on failure.

**Example:**

```c
// Compare function for integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list1 = create_list(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = create_list(sizeof(int)); // Contains [20, 40, 30]
// ... populate lists ...
LinkedList* intersection_list = intersection(list1, list2, compare_int);
// 'intersection_list' is a new list: [20, 30]
destroy(list1);
destroy(list2);
destroy(intersection_list);
```

### `union_lists`

`LinkedList* union_lists(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn);`

This function creates a new list containing all unique elements from both input lists combined.

**Receives:**

- `list1`: The first list.
- `list2`: The second list.
- `compare_fn`: A function to compare elements for equality.

**Returns:**

- A new `LinkedList` with the union of elements, or `NULL` on failure.

**Example:**

```c
// Compare function for integers
int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

LinkedList* list1 = create_list(sizeof(int)); // Contains [10, 20, 30]
LinkedList* list2 = create_list(sizeof(int)); // Contains [20, 40, 30]
// ... populate lists ...
LinkedList* union_list = union_lists(list1, list2, compare_int);
// 'union_list' is a new list: [10, 20, 30, 40]
destroy(list1);
destroy(list2);
destroy(union_list);
```

<br></br>

## 10. List \<--\> Array

### `from_array`

`ListResult from_array(LinkedList* list, const void* arr, size_t n);`

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
from_array(num_list, numbers, 4);
// num_list now contains [10, 20, 30, 40]
destroy(num_list);
```

### `to_array`

`void* to_array(const LinkedList* list, size_t* out_size);`

Converts the entire linked list into a newly allocated contiguous C array. Useful for interoperability with APIs that expect raw arrays.

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
int* new_array = (int*)to_array(num_list, &array_size);
if (new_array) {
    printf("Array has %zu elements. First element: %d\n", array_size, new_array[0]);
    free(new_array); // Don't forget to free!
}
destroy(num_list);
```

<br></br>

## 11. List \<--\> String / File

### `to_string`

`char* to_string(const LinkedList* list, const char* separator);`

Converts the list into a single heap‑allocated string with a custom separator. Intended for primitive element sizes (int / double / char). Other element sizes become the token `[data]` (not a complete reversible serialization).

**Receives:**

- `list`: The list to convert.
- `separator`: The string to place between elements.

**Returns:**

- A newly allocated string. The caller is responsible for freeing it.

**Example:**

```c
LinkedList* list = create_list(sizeof(int));
int nums[] = {10, 20, 30};
from_array(list, nums, 3);
char* str = to_string(list, ", ");
if (str) {
    printf("List as string: %s\n", str); // Output: 10, 20, 30
    free(str);
}
destroy(list);
```

### `save_to_file`

`ListResult save_to_file(const LinkedList* list, const char* filename, FileFormat format, const char* separator);`

Saves the list to a file in the specified format.

**Receives:**

- `list`: The list to save.
- `filename`: The name of the file to create.
- `format`: `FILE_FORMAT_BINARY` or `FILE_FORMAT_TEXT`.
- `separator`: For text format - string between elements (default "\n"). For binary format - ignored.

**Returns:**

- `LIST_SUCCESS` on success, or an error code on failure.

**Binary layout:**
```
[size_t length][size_t element_size][raw bytes...]
```

**Text mode rules:**
- `int` / `double` / `char` → written as readable values.
- Other sizes → Hex dump (space-separated bytes) when using whitespace tokenization.

**Example (text save):**

```c
LinkedList* numbers = create_list(sizeof(int));
for (int i = 1; i <= 5; ++i) insert_tail_value(numbers, i);
ListResult r = save_to_file(numbers, "numbers.txt", FILE_FORMAT_TEXT, "\n");
if (r == LIST_SUCCESS) {
    printf("File saved successfully\n");
}
destroy(numbers);
```

**Example (binary save):**

```c
LinkedList* numbers = create_list(sizeof(int));
for (int i = 1; i <= 5; ++i) insert_tail_value(numbers, i);
ListResult r = save_to_file(numbers, "numbers.bin", FILE_FORMAT_BINARY, NULL);
if (r == LIST_SUCCESS) {
    printf("Binary file saved successfully\n");
}
destroy(numbers);
```

### `load_from_file`

`LinkedList* load_from_file(const char* filename, size_t element_size, FileFormat format, const char* separator, PrintFunction print_fn, CompareFunction compare_fn, FreeFunction free_fn, CopyFunction copy_fn);`

Loads a new list from a file (Binary or Text).

**Receives:**

- `filename`: The name of the file to load.
- `element_size`: The size of each element to load.
- `format`: `FILE_FORMAT_BINARY` or `FILE_FORMAT_TEXT`.
- `separator`: For text format - separator string (NULL/"" means whitespace tokens).
- `print_fn`: Optional print function for the new list.
- `compare_fn`: Optional compare function for the new list.
- `free_fn`: Optional free function for the new list.
- `copy_fn`: Optional copy function for the new list.

**Returns:**

- A new `LinkedList` on success, or `NULL` on failure.

**Text parsing modes:**
1. `separator == NULL || separator[0] == '\0'` → parses by whitespace/lines. Supports hex for non-primitive types.
2. Custom `separator` (e.g., ",") → currently supports only int/double/char (no hex in this mode).

**Example (text load):**

```c
LinkedList* loaded = load_from_file("numbers.txt", sizeof(int),
                                    FILE_FORMAT_TEXT, "\n",
                                    NULL, NULL, NULL, NULL);
if (loaded) {
    print_list(loaded);
    destroy(loaded);
}
```

**Example (binary load):**

```c
LinkedList* loaded = load_from_file("numbers.bin", sizeof(int), 
                                    FILE_FORMAT_BINARY, NULL,
                                    NULL, NULL, NULL, NULL);
if (loaded) {
    print_list(loaded);
    destroy(loaded);
}
```

**Custom separator example:**

Save and load values as comma-separated in a single line:

```c
LinkedList* ids = create_list(sizeof(int));
for (int i = 100; i <= 105; ++i) insert_tail_value(ids, i);

// Write as: 100,101,102,103,104,105 (single line + '\n' at end)
save_to_file(ids, "ids.csv", FILE_FORMAT_TEXT, ",");
destroy(ids);

// Load back using the SAME separator string
LinkedList* loaded_ids = load_from_file("ids.csv", sizeof(int), FILE_FORMAT_TEXT, ",",
                                        NULL, NULL, NULL, NULL);
if (loaded_ids) {
    printf("Loaded (comma-separated):\n");
    print_list(loaded_ids);
    destroy(loaded_ids);
}
```

**Notes:**
- Binary format is not architecture-neutral (endianness / size_t).
- If you change the separator between writing and reading, you'll get incorrect parsing.
- In custom separator mode (not whitespace), there's currently no support for hex for complex types – only int/double/char.- A new `LinkedList` with the common elements, or `NULL` on failure.

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

`LinkedList* union_lists(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn);`

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

<br></br>שומר את הרשימה לקובץ בפורמט הנבחר.

Signature:
```c
ListResult save_to_file(const LinkedList* list,
                        const char* filename,
                        FileFormat format,
                        const char* separator); // TEXT only (NULL => "\n")
```

Parameters:
- `format`: `FILE_FORMAT_BINARY` או `FILE_FORMAT_TEXT`.
- `separator`: בטקסט – בין איברים (ברירת מחדל "\n"). אם לא מסתיים ב־'\n' הספרייה מוסיפה אחד בסוף הקובץ לנוחות.

Binary layout (פשוט / דמו):
```
[size_t length][size_t element_size][raw bytes...]
```
Text mode rules:
- `int` / `double` / `char` → כתיבה כערך קריא.
- גודל אחר → Hex dump (במצב טעינת רווחים). אם אתה משתמש במפצל מותאם אישית (separator מותאם) — כרגע רק טיפוסים פרימיטיביים נתמכים.

Example (text save):
```c
LinkedList* numbers = create_list(sizeof(int));
for (int i = 1; i <= 5; ++i) insert_tail_value(numbers, i);
ListResult r = save_to_file(numbers, "numbers.txt", FILE_FORMAT_TEXT, "\n");
printf("save_to_file (TEXT): %s\n", error_string(r));
```

### `load_from_file`

`LinkedList* load_from_file(const char* filename, size_t element_size, FileFormat format, const char* separator, PrintFunction print_fn, CompareFunction compare_fn, FreeFunction free_fn, CopyFunction copy_fn);`

טוען רשימה חדשה מקובץ (Binary או Text).

Signature:
```c
LinkedList* load_from_file(const char* filename,
                           size_t element_size,
                           FileFormat format,
                           const char* separator, // TEXT: NULL/"" => whitespace tokens
                           PrintFunction print_fn,
                           CompareFunction compare_fn,
                           FreeFunction free_fn,
                           CopyFunction copy_fn);
```

Text parsing modes:
1. `separator == NULL || separator[0] == '\0'` → מפרק לפי רווחים/שורות. תומך גם ב‑hex לטיפוסים לא פרימיטיביים.
2. `separator` מותאם (למשל ",") → תומך כרגע רק ב‑int/double/char (בלי hex במצב זה).

**Example (text load):**

```c
LinkedList* loaded = load_from_file("numbers.txt", sizeof(int),
                                    FILE_FORMAT_TEXT, "\n",
                                    NULL, NULL, NULL, NULL);
if (loaded) {
    print_list(loaded);
    destroy(loaded);
}
```

**Example (binary load):**

```c
LinkedList* loaded = load_from_file("numbers.bin", sizeof(int), 
                                    FILE_FORMAT_BINARY, NULL,
                                    NULL, NULL, NULL, NULL);
if (loaded) {
    print_list(loaded);
    destroy(loaded);
}
```

**Custom separator example:**

Save and load values as comma-separated in a single line:

```c
LinkedList* ids = create_list(sizeof(int));
for (int i = 100; i <= 105; ++i) insert_tail_value(ids, i);

// Write as: 100,101,102,103,104,105 (single line + '\n' at end)
save_to_file(ids, "ids.csv", FILE_FORMAT_TEXT, ",");
destroy(ids);

// Load back using the SAME separator string
LinkedList* loaded_ids = load_from_file("ids.csv", sizeof(int), FILE_FORMAT_TEXT, ",",
                                        NULL, NULL, NULL, NULL);
if (loaded_ids) {
    printf("Loaded (comma-separated):\n");
    print_list(loaded_ids);
    destroy(loaded_ids);
}
```

**Notes:**
- Binary format is not architecture-neutral (endianness / size_t).
- If you change the separator between writing and reading, you'll get incorrect parsing.
- In custom separator mode (not whitespace), there's currently no support for hex for complex types – only int/double/char.
