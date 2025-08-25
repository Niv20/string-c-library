# Generic Doubly Linked List Library in C

A comprehensive, type-agnostic doubly linked list implementation in C with Python-style operations, size limits, advanced error handling, and helper functions for common data types.

## Features

### 🐍 Python-Style Operations

- `list_append()` - Add element to end
- `list_insert_at_index()` - Insert at specific position
- `list_pop()` - Remove and return element
- `list_remove()` - Remove first occurrence of value
- `list_index()` - Find index of element
- `list_count()` - Count occurrences
- `list_reverse()` - Reverse in place
- `list_sort()` - Sort ascending/descending
- `list_clear()` - Remove all elements
- `list_get()` / `list_set()` - Access by index
- `list_extend()` - Append another list
- `list_copy()` - Create deep copy

### 🛠 Built-in Data Type Support

Ready-to-use functions for common types:

- **Integers**: `list_create_int()`, `list_create_int_with_limits()`
- **Doubles**: `list_create_double()`, `list_create_double_with_limits()`
- **Characters**: `list_create_char()`, `list_create_char_with_limits()`
- **Strings**: `list_create_string()`, `list_create_string_with_limits()`

### 🎯 Size Limits & FIFO Behavior

- **Maximum size control**: Set limits with `list_set_max_size()`
- **FIFO overwrite mode**: Automatically remove oldest elements when full
- **Overflow protection**: Prevent insertion when full (if overwrite disabled)
- **Dynamic size adjustment**: Change limits at runtime

### 🔧 Advanced Error Handling

- **Detailed error codes**: `ListResult` enum with specific error types
- **Human-readable errors**: `list_error_string()` for debugging
- **Comprehensive validation**: All functions return meaningful error codes
- **Safe operations**: NULL pointer and bounds checking

### ⚡ Advanced Features

- **Dummy nodes** for simplified operations (always enabled)
- **Deep copy support** for complex data structures
- **Custom memory management** with user-defined free/copy functions
- **Array conversion** utilities
- **Generic void pointer** design for any data type

## Quick Start

### Basic Usage

```c
#include "linked_list.h"

// Create a list of integers
LinkedList* my_list = list_create_int();

// Add some elements (Python-style!)
int values[] = {10, 20, 30};
for (int i = 0; i < 3; i++) {
    list_append(my_list, &values[i]);
}

// Print the list
list_print(my_list);  // Output: [0]: 10, [1]: 20, [2]: 30

// Python-style operations
int val = 15;
list_insert_at_index(my_list, 1, &val);  // Insert 15 at index 1

int popped;
list_pop(my_list, -1, &popped);  // Pop last element

int index = list_index(my_list, &val);  // Find index of 15

// Sort and reverse
list_sort(my_list, false);  // Sort ascending
list_reverse(my_list);      // Reverse in place

// Clean up
list_destroy(my_list);
```

### Working with Strings

```c
// Create a string list (handles deep copying automatically)
LinkedList* str_list = list_create_string();

char* words[] = {"hello", "world", "python", "style"};
for (int i = 0; i < 4; i++) {
    list_append(str_list, &words[i]);
}

// Remove a word
char* target = "python";
list_remove(str_list, &target);

list_destroy(str_list);
```

### Array Conversions

```c
// Array to List
int arr[] = {1, 2, 3, 4, 5};
LinkedList* list = list_create_int();
list_from_array(list, arr, 5);

// List to Array
size_t size;
int* result = (int*)list_to_array(list, &size);
// Use result...
free(result);
```

### Size-Limited Lists (Message Queue Example)

```c
// Create a message list that keeps only the last 5 messages
LinkedList* messages = list_create_string_with_limits(5, true);

// Add messages - older ones are automatically removed
char* msg1 = "Hello";
char* msg2 = "How are you?";
char* msg3 = "I'm fine";
char* msg4 = "Great!";
char* msg5 = "See you";
char* msg6 = "Bye!";  // This will remove "Hello"

ListResult result = list_append(messages, &msg1);
if (result != LIST_SUCCESS) {
    printf("Error: %s\n", list_error_string(result));
}

// Check current settings
printf("Max size: %zu\n", list_get_max_size(messages));
printf("Allows overwrite: %s\n", list_allows_overwrite(messages) ? "yes" : "no");

list_destroy(messages);
```

### Error Handling

```c
LinkedList* list = list_create_int_with_limits(2, false);  // Max 2, no overwrite

int values[] = {1, 2, 3};
for (int i = 0; i < 3; i++) {
    ListResult result = list_append(list, &values[i]);
    if (result != LIST_SUCCESS) {
        printf("Failed to add %d: %s\n", values[i], list_error_string(result));
        // Output: "Failed to add 3: List has reached maximum capacity"
    }
}

// Change size limit at runtime
result = list_set_max_size(list, 5, true);  // Increase limit, enable overwrite
if (result == LIST_SUCCESS) {
    printf("Size limit updated successfully\n");
}

list_destroy(list);
```

## API Reference

### List Creation

```c
// Basic creation
LinkedList* list_create_int(void);           // Integer list
LinkedList* list_create_double(void);        // Double list
LinkedList* list_create_char(void);          // Character list
LinkedList* list_create_string(void);        // String list

// Creation with size limits
LinkedList* list_create_int_with_limits(size_t max_size, bool allow_overwrite);
LinkedList* list_create_double_with_limits(size_t max_size, bool allow_overwrite);
LinkedList* list_create_char_with_limits(size_t max_size, bool allow_overwrite);
LinkedList* list_create_string_with_limits(size_t max_size, bool allow_overwrite);

// Generic creation
LinkedList* list_create(size_t element_size, PrintFunction print_fn,
                       CompareFunction compare_fn, FreeFunction free_fn,
                       CopyFunction copy_fn);
LinkedList* list_create_with_limits(size_t element_size, size_t max_size, bool allow_overwrite,
                                   PrintFunction print_fn, CompareFunction compare_fn,
                                   FreeFunction free_fn, CopyFunction copy_fn);
```

### Size Management

```c
ListResult list_set_max_size(LinkedList* list, size_t max_size, bool allow_overwrite);
size_t list_get_max_size(const LinkedList* list);
bool list_allows_overwrite(const LinkedList* list);
```

### Error Handling

```c
const char* list_error_string(ListResult result);

// Error codes
typedef enum {
    LIST_SUCCESS = 0,
    LIST_ERROR_NULL_POINTER,
    LIST_ERROR_MEMORY_ALLOC,
    LIST_ERROR_INDEX_OUT_OF_BOUNDS,
    LIST_ERROR_ELEMENT_NOT_FOUND,
    LIST_ERROR_LIST_FULL,
    LIST_ERROR_OVERWRITE_DISABLED,
    LIST_ERROR_INVALID_OPERATION,
    LIST_ERROR_NO_COMPARE_FUNCTION
} ListResult;
```

### Python-Style Operations

```c
ListResult list_append(LinkedList* list, void* data);
ListResult list_insert_at_index(LinkedList* list, size_t index, void* data);
ListResult list_pop(LinkedList* list, int index, void* out_data);
ListResult list_remove(LinkedList* list, void* data);
int list_index(const LinkedList* list, void* data);
size_t list_count(const LinkedList* list, void* data);
void list_reverse(LinkedList* list);
void list_sort(LinkedList* list, bool reverse);
void list_clear(LinkedList* list);
ListResult list_get(const LinkedList* list, size_t index, void* out_data);
ListResult list_set(LinkedList* list, size_t index, void* data);
ListResult list_extend(LinkedList* list, const LinkedList* other);
LinkedList* list_copy(const LinkedList* list);
```

### Utility Functions

```c
size_t list_get_length(const LinkedList* list);
bool list_is_empty(const LinkedList* list);
void list_print(const LinkedList* list);
void list_destroy(LinkedList* list);
```

### Array Conversions

```c
ListResult list_from_array(LinkedList* list, const void* arr, size_t n);
void* list_to_array(const LinkedList* list, size_t* out_size);
```

## Examples

### Example 1: Working with Numbers

```c
LinkedList* numbers = list_create_int();

// Add numbers 1-10
for (int i = 1; i <= 10; i++) {
    list_append(numbers, &i);
}

// Remove even numbers
for (int i = 2; i <= 10; i += 2) {
    list_remove(numbers, &i);
}

list_print(numbers);  // Shows: 1, 3, 5, 7, 9
list_destroy(numbers);
```

### Example 2: List Operations

```c
LinkedList* list1 = list_create_int();
LinkedList* list2 = list_create_int();

// Fill lists
int vals1[] = {1, 2, 3};
int vals2[] = {4, 5, 6};

for (int i = 0; i < 3; i++) {
    list_append(list1, &vals1[i]);
    list_append(list2, &vals2[i]);
}

// Extend list1 with list2
list_extend(list1, list2);  // list1 now contains: 1,2,3,4,5,6

// Create a copy
LinkedList* copy = list_copy(list1);

// Sort in descending order
list_sort(copy, true);

list_destroy(list1);
list_destroy(list2);
list_destroy(copy);
```

## Building

```bash
gcc -Wall -Wextra -std=c99 -o demo demo.c linked_list.c
gcc -Wall -Wextra -std=c99 -o python_demo python_style_demo.c linked_list.c
gcc -Wall -Wextra -std=c99 -o size_demo size_limit_demo.c linked_list.c
```

## Use Cases

### 📧 Message Queue/Chat History

```c
// Keep only the last 10 messages
LinkedList* chat = list_create_string_with_limits(10, true);
// Automatically removes old messages as new ones arrive
```

### 📊 Sliding Window Data

```c
// Keep last N data points for calculations
LinkedList* readings = list_create_double_with_limits(100, true);
// Perfect for moving averages, trend analysis
```

### 🔄 LRU Cache Implementation

```c
// Combined with hash table for O(1) access
LinkedList* lru_list = list_create_int_with_limits(cache_size, true);
// Most recently used items stay, old ones are evicted
```

### 📝 Undo/Redo Buffer

```c
// Limited undo history
LinkedList* undo_stack = list_create_with_limits(sizeof(Action), 50, false);
// Prevents excessive memory usage
```

## Memory Management

The library handles memory management automatically:

- **Simple types** (int, double, char): Automatic copying with `memcpy`
- **Complex types** (strings, structs with pointers): Uses custom copy/free functions
- **Dummy nodes**: Always enabled for simplified logic
- **Deep copying**: Proper handling of nested data structures
- **Size limits**: Automatic cleanup when limits are exceeded
- **Error handling**: Comprehensive validation and meaningful error messages

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Niv Libovitch - August 25, 2025
