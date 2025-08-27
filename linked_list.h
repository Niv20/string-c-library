/**
 * @file linked_list.h
 * @author Niv Libovitch
 * @date 25 Aug 2025
 * @brief A generic doubly linked list library in C.
 *
 * This header file defines the public interface for a generic doubly linked list.
 * It is designed to be type-agnostic by using void pointers and manages memory
 * internally by copying data provided by the user. For complex data types that
 * contain pointers, custom copy and free functions should be provided.
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

#define UNLIMITED 100

typedef enum { FALSE, TRUE } bool;

// Enum to specify whether to find the minimum or maximum element.
typedef enum {
    FIND_MIN,
    FIND_MAX
} ExtremeFindType;


// Constants for advanced delete operations
#define DELETE_ALL_OCCURRENCES -1
#define SEARCH_FROM_HEAD 0
#define SEARCH_FROM_TAIL 1


// --- Error Codes ---

/**
 * @brief Enumeration of possible list operation results.
 */
typedef enum {
    LIST_SUCCESS = 0,           /**< Operation completed successfully */
    LIST_ERROR_NULL_POINTER,    /**< NULL pointer provided */
    LIST_ERROR_MEMORY_ALLOC,    /**< Memory allocation failed */
    LIST_ERROR_INDEX_OUT_OF_BOUNDS, /**< Index is out of bounds */
    LIST_ERROR_ELEMENT_NOT_FOUND,   /**< Element not found in list */
    LIST_ERROR_LIST_FULL,       /**< List has reached maximum capacity */
    LIST_ERROR_OVERWRITE_DISABLED,  /**< Overwrite is disabled and list is full */
    LIST_ERROR_INVALID_OPERATION,   /**< Invalid operation for current state */
    LIST_ERROR_NO_COMPARE_FUNCTION,  /**< Compare function required but not provided */
    LIST_ERROR_NO_PRINT_FUNCTION,    /**< Print function required but not provided */
    LIST_ERROR_NO_FREE_FUNCTION,     /**< Free function required but not provided */
    LIST_ERROR_NO_COPY_FUNCTION      /**< Copy function required but not provided */
} ListResult;

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
    size_t max_size;           /**< Maximum number of elements (0 = unlimited). */
    bool allow_overwrite;      /**< Whether to overwrite oldest elements when full. */

    // --- User-provided helper functions ---
    PrintFunction print_node_function;   /**< Function to print an element. */
    CompareFunction compare_node_function; /**< Function to compare two elements. */
    FreeFunction free_node_function;     /**< Function to free a complex element. */
    CopyFunction copy_node_function;     /**< Function to deep copy a complex element. */
} LinkedList;


// --- Error Handling ---
const char* list_error_string(ListResult result);

// --- Lifecycle Functions ---
LinkedList* list_create(size_t element_size);

// --- Setters for LinkedList fields ---
void list_set_print_function(LinkedList* list, PrintFunction print_fn);
void list_set_compare_function(LinkedList* list, CompareFunction compare_fn);
void list_set_free_function(LinkedList* list, FreeFunction free_fn);
void list_set_copy_function(LinkedList* list, CopyFunction copy_fn);

// --- Size and Overwrite Management ---
ListResult list_set_max_size(LinkedList* list, size_t max_size, bool allow_overwrite);

// --- Insertion Functions ---
ListResult list_insert_at_head(LinkedList* list, void* data);
ListResult list_insert_at_tail(LinkedList* list, void* data);
ListResult list_insert_at_index(LinkedList* list, size_t index, void* data);

// --- Deletion Functions ---
ListResult list_delete_from_head(LinkedList* list);
ListResult list_delete_from_tail(LinkedList* list);
ListResult list_delete_at_index(LinkedList* list, size_t index);
ListResult list_remove_advanced(LinkedList* list, void* data, int count, int direction);
ListResult list_clear(LinkedList* list);
void list_destroy(LinkedList* list);

// --- Utility Functions ---
size_t list_get_length(const LinkedList* list);
bool list_is_empty(const LinkedList* list);
ListResult list_print(const LinkedList* list);
ListResult list_print_advanced(const LinkedList* list, bool show_index, const char* separator);

// --- Search and Access Functions ---
void* list_get(const LinkedList* list, size_t index);
ListResult list_set(LinkedList* list, size_t index, void* data);
int list_index(const LinkedList* list, void* data);
int list_index_advanced(const LinkedList* list, void* data, int direction);
size_t list_count_occurrences(const LinkedList* list, void* data);

// --- Sorting and Manipulation Functions ---
ListResult list_sort(LinkedList* list, bool reverse);

// --- List Operations Functions ---
LinkedList* list_copy(const LinkedList* list);
ListResult list_extend(LinkedList* list, const LinkedList* other);
LinkedList* list_concat(const LinkedList* list1, const LinkedList* list2);
LinkedList* list_slice(const LinkedList* list, size_t start, size_t end);
ListResult list_rotate(LinkedList* list, int positions);
ListResult list_reverse(LinkedList* list);
LinkedList* list_filter(const LinkedList* list, FilterFunction filter_fn);

// --- Transformation Functions ---
LinkedList* list_map(const LinkedList* list, MapFunction map_fn, size_t new_element_size);

// --- Mathematical Functions ---
void* list_min(const LinkedList* list);
void* list_max(const LinkedList* list);
LinkedList* list_unique(const LinkedList* list);
LinkedList* list_intersection(const LinkedList* list1, const LinkedList* list2);
LinkedList* list_union(const LinkedList* list1, const LinkedList* list2);

// --- Array to List Conversion Functions ---
ListResult array_to_list(LinkedList* list, const void* arr, size_t n);
void* list_to_array(const LinkedList* list, size_t* out_size);

// --- I/O and Format Functions ---
char* list_to_string(const LinkedList* list, const char* separator);
ListResult list_save_to_file(const LinkedList* list, const char* filename);
LinkedList* list_load_from_file(const char* filename, size_t element_size,
                                PrintFunction print_fn, CompareFunction compare_fn,
                                FreeFunction free_fn, CopyFunction copy_fn);

#endif // LINKED_LIST_H