/**
 * @file linked_list.h
 * @author Niv Libovitch
 * @date 25 Aug 2025
 * @brief A generic doubly linked list library in C.
 *
 * This header file defines the public interface for a doubly linked list.
 * It is designed to be type-agnostic by using void pointers and manages memory
 * internally by copying data provided by the user. For complex data types that
 * contain pointers, custom copy and free functions should be provided.
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>
#include <stdbool.h>

#define UNLIMITED 0

typedef enum { 
    REJECT_NEW_WHEN_FULL = false, 
    DELETE_OLD_WHEN_FULL = true 
} OverflowBehavior;

// Constants for advanced delete operations
#define DELETE_ALL_OCCURRENCES -1

// --- Error Codes ---

/**
 * @brief Enumeration of possible list operation results.
 */
typedef enum {
    LIST_SUCCESS = 0,               /**< Operation completed successfully */
    LIST_ERROR_NULL_POINTER,        /**< NULL pointer provided */
    LIST_ERROR_MEMORY_ALLOC,        /**< Memory allocation failed */
    LIST_ERROR_INDEX_OUT_OF_BOUNDS, /**< Index is out of bounds */
    LIST_ERROR_ELEMENT_NOT_FOUND,   /**< Element not found in list */
    LIST_ERROR_LIST_FULL,           /**< List has reached maximum capacity */
    LIST_ERROR_OVERWRITE_DISABLED,  /**< Overwrite is disabled and list is full */
    LIST_ERROR_INVALID_OPERATION,   /**< Invalid operation for current state */
    LIST_ERROR_NO_COMPARE_FUNCTION, /**< Compare function required but not provided */
    LIST_ERROR_NO_PRINT_FUNCTION,   /**< Print function required but not provided */
    LIST_ERROR_NO_FREE_FUNCTION     /**< Free function required but not provided */
} ListResult;

typedef enum {
    START_FROM_HEAD,
    START_FROM_TAIL 
} Direction;


// --- Type Definitions ---

/**
 * @brief A function pointer type for printing an element's data.
 * @param data A void pointer to the element's data.
 */
typedef void (*PrintFunction)(void* data);

/**
 * @brief A function pointer type for comparing two elements.
 * @param data1 A const void pointer to the first element's data.
 * @param data2 A const void pointer to the second element's data.
 * @return An integer less than, equal to, or greater than zero if data1 is found,
 * respectively, to be less than, to match, or be greater than data2.
 */
typedef int (*CompareFunction)(const void* data1, const void* data2);

/**
 * @brief A function pointer type for freeing an element's data.
 * Required for complex data types that allocate memory internally (e.g., structs with pointers).
 * @param data A void pointer to the element's data to be freed.
 */
typedef void (*FreeFunction)(void* data);



/**
 * @brief A function pointer type for deep copying an element's data.
 * Required for complex data types to solve the shallow copy problem.
 * @param dest A void pointer to the destination memory block.
 * @param src A const void pointer to the source data to be copied.
 */
typedef void (*CopyFunction)(void* dest, const void* src);

/**
 * @brief A function pointer type for filtering elements.
 * @param data A const void pointer to the element's data to test.
 * @return True if the element should be included, false otherwise.
 */
typedef bool (*FilterFunction)(const void* data);

/**
 * @brief Specifies how the list manages data memory.
 * Internal use only for insertion functions.
 */
typedef enum {
    LIST_MODE_VALUE,    /**< Copy data into list-managed memory. */
    LIST_MODE_POINTER   /**< Store user-provided pointers directly. */
} ListMemoryMode;

/**
 * @brief A function pointer type for transforming elements.
 * @param dest A void pointer to the destination for the transformed data.
 * @param src A const void pointer to the source data to transform.
 */
typedef void (*MapFunction)(void* dest, const void* src);

/**
 * @brief Internal structure representing a node in the linked list.
 * @note Users of the library should not manipulate this structure directly.
 */
typedef struct Node {
    void* data;          /**< Pointer to the data stored in the node. */
    struct Node* next;   /**< Pointer to the next node in the list. */
    struct Node* prev;   /**< Pointer to the previous node in the list. */
} Node;

/**
 * @brief The main structure to manage the linked list.
 * This acts as a handle for all list operations.
 */
typedef struct LinkedList {
    Node* head;                /**< Pointer to the dummy head node. */
    Node* tail;                /**< Pointer to the dummy tail node. */
    size_t length;             /**< The number of elements in the list. */
    size_t element_size;       /**< The size in bytes of the data type stored. */
    size_t max_size;           /**< Maximum number of elements (UNLIMITED = no limit). */
    OverflowBehavior allow_overwrite; /**< Behavior when list reaches max capacity. */

    // --- User-provided helper functions ---
    PrintFunction print_node_function;   /**< Function to print an element. */
    CompareFunction compare_node_function; /**< Function to compare two elements. */
    FreeFunction free_node_function;     /**< Function to free a complex element. */
    CopyFunction copy_node_function;     /**< Function to deep copy a complex element. */
} LinkedList;


// --- Error Handling ---
const char* error_string(ListResult result);

// --- Lifecycle Functions ---
LinkedList* create_list(size_t element_size);

// --- Setters for LinkedList fields ---
void set_print_function(LinkedList* list, PrintFunction print_fn);
void set_compare_function(LinkedList* list, CompareFunction compare_fn);
void set_free_function(LinkedList* list, FreeFunction free_fn);
void set_copy_function(LinkedList* list, CopyFunction copy_fn);

// --- Size and Overwrite Management ---
ListResult set_max_size(LinkedList* list, size_t max_size, OverflowBehavior behavior);

// --- Insertion Functions ---
// Internal functions (used by macros) - copy data into list-managed memory
ListResult insert_head_value_internal(LinkedList* list, void* data);
ListResult insert_tail_value_internal(LinkedList* list, void* data);
ListResult insert_index_value_internal(LinkedList* list, size_t index, void* data);

// Pointer mode functions - also copy data into list-managed memory
ListResult insert_head_ptr(LinkedList* list, void* data_ptr);
ListResult insert_tail_ptr(LinkedList* list, void* data_ptr);
ListResult insert_index_ptr(LinkedList* list, size_t index, void* data_ptr);

// --- Deletion Functions ---
ListResult delete_head(LinkedList* list);
ListResult delete_tail(LinkedList* list);
ListResult delete_index(LinkedList* list, size_t index);
ListResult remove_advanced(LinkedList* list, void* data, int count, Direction direction);
ListResult clear(LinkedList* list);
void destroy(LinkedList* list);

// --- Utility Functions ---
size_t get_length(const LinkedList* list);
bool is_empty(const LinkedList* list);
ListResult print(const LinkedList* list);
ListResult print_advanced(const LinkedList* list, bool show_index, const char* separator);

// --- Search and Access Functions ---
void* get(const LinkedList* list, size_t index);
ListResult set(LinkedList* list, size_t index, void* data);
int index_of(const LinkedList* list, void* data);
int index_of_advanced(const LinkedList* list, void* data, Direction direction);

// --- Sorting and Manipulation Functions ---
ListResult sort(LinkedList* list, bool reverse);

// --- List Operations Functions ---
LinkedList* copy(const LinkedList* list);
ListResult extend(LinkedList* list, const LinkedList* other);
LinkedList* concat(const LinkedList* list1, const LinkedList* list2);
LinkedList* slice(const LinkedList* list, size_t start, size_t end);
ListResult rotate(LinkedList* list, int positions);
ListResult reverse(LinkedList* list);
LinkedList* filter(const LinkedList* list, FilterFunction filter_fn);

// --- Transformation Functions ---
LinkedList* map(const LinkedList* list, MapFunction map_fn, size_t new_element_size);

// --- Mathematical Functions ---
size_t count_if(const LinkedList* list, bool (*predicate)(const void *element, void *arg), void *arg);
void* min_by(const LinkedList* list, int (*compare)(const void *a, const void *b));
void* max_by(const LinkedList* list, int (*compare)(const void *a, const void *b));
LinkedList* unique(const LinkedList* list);
LinkedList* unique_advanced(const LinkedList* list, CompareFunction custom_compare, Direction order);
LinkedList* intersection(const LinkedList* list1, const LinkedList* list2);
LinkedList* union_lists(const LinkedList* list1, const LinkedList* list2);

// --- Array to List Conversion Functions ---
ListResult from_array(LinkedList* list, const void* arr, size_t n);
void* to_array(const LinkedList* list, size_t* out_size);

// --- I/O and Format Functions ---
char* to_string(const LinkedList* list, const char* separator);
ListResult save_to_file(const LinkedList* list, const char* filename);
LinkedList* load_from_file(const char* filename, size_t element_size,
                                PrintFunction print_fn, CompareFunction compare_fn,
                                FreeFunction free_fn, CopyFunction copy_fn);

// --- Convenience Macros for Passing Values Directly ---

/**
 * @brief Convenience macros that allow passing values directly instead of pointers.
 * 
 * These macros create a temporary variable to hold the value and pass its address
 * to the underlying pointer-based functions. This provides a more intuitive API
 * while maintaining the existing implementation.
 * 
 * Usage examples:
 *   Person alice = create_person(1, "Alice", 25);
 *   insert_tail_value(people_list, alice);  // Pass value directly
 *   
 *   int number = 42;
 *   insert_head_value(numbers_list, number);  // Pass value directly
 *   
 *   // Or even with literals (though be careful with scope):
 *   insert_tail_value(numbers_list, 100);
 */

#ifdef __GNUC__  // GCC and Clang support
    #define insert_head_value(list, value) \
        ({ __typeof__(value) _temp = (value); insert_head_value_internal((list), &_temp); })

    #define insert_tail_value(list, value) \
        ({ __typeof__(value) _temp = (value); insert_tail_value_internal((list), &_temp); })

    #define insert_index_value(list, index, value) \
        ({ __typeof__(value) _temp = (value); insert_index_value_internal((list), (index), &_temp); })
#else
    // Fallback for compilers that don't support __typeof__
    // These will require explicit casting or won't work with all types
    #define insert_head_value(list, value) \
        insert_head_value_internal((list), &(value))

    #define insert_tail_value(list, value) \
        insert_tail_value_internal((list), &(value))

    #define insert_index_value(list, index, value) \
        insert_index_value_internal((list), (index), &(value))
#endif

#endif