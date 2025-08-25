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

#include <stdbool.h>
#include <stddef.h>

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
    LIST_ERROR_NO_COMPARE_FUNCTION  /**< Compare function required but not provided */
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
 * @brief A function pointer type for aggregation operations.
 * @param accumulator A void pointer to the accumulator value.
 * @param current A const void pointer to the current element.
 */
typedef void (*AggregateFunction)(void* accumulator, const void* current);

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


// --- Setters for LinkedList fields ---
void list_set_element_size(LinkedList* list, size_t element_size);
void list_set_print_function(LinkedList* list, PrintFunction print_fn);
void list_set_compare_function(LinkedList* list, CompareFunction compare_fn);
void list_set_free_function(LinkedList* list, FreeFunction free_fn);
void list_set_copy_function(LinkedList* list, CopyFunction copy_fn);
void list_set_storage_mode(LinkedList* list, bool stores_pointers, bool owns_data);

// --- Size and Overwrite Management ---
ListResult list_set_max_size(LinkedList* list, size_t max_size, bool allow_overwrite);
size_t list_get_max_size(const LinkedList* list);
bool list_allows_overwrite(const LinkedList* list);

// --- Error Handling ---
const char* list_error_string(ListResult result);

// --- Lifecycle Functions ---
LinkedList* list_create(size_t element_size);
LinkedList* list_create_value_based(size_t element_size, PrintFunction print_fn,
                                   CompareFunction compare_fn, FreeFunction free_fn, CopyFunction copy_fn);
void list_destroy(LinkedList* list);

// --- Insertion Functions ---
ListResult list_insert_at_head(LinkedList* list, void* data);
ListResult list_insert_at_tail(LinkedList* list, void* data);
ListResult list_append(LinkedList* list, void* data);
ListResult list_insert_at_index(LinkedList* list, size_t index, void* data);

// --- Deletion Functions ---
ListResult list_delete_from_head(LinkedList* list);
ListResult list_delete_from_tail(LinkedList* list);
ListResult list_pop(LinkedList* list, int index, void* out_data);
ListResult list_remove(LinkedList* list, void* data);
void list_clear(LinkedList* list);

// --- Utility Functions ---
size_t list_get_length(const LinkedList* list);
bool list_is_empty(const LinkedList* list);
void list_print(const LinkedList* list);
void list_print_reverse(const LinkedList* list);

// Iterator functions for doubly-linked traversal
Node* list_begin(const LinkedList* list);
Node* list_rbegin(const LinkedList* list);
Node* list_end(const LinkedList* list);
Node* list_rend(const LinkedList* list);

// --- Search and Access Functions ---
ListResult list_get(const LinkedList* list, size_t index, void* out_data);
ListResult list_set(LinkedList* list, size_t index, void* data);
int list_index(const LinkedList* list, void* data);
size_t list_count(const LinkedList* list, void* data);

// --- Sorting and Manipulation Functions ---
void list_reverse(LinkedList* list);
void list_sort(LinkedList* list, bool reverse);

// --- List Operations Functions ---
ListResult list_extend(LinkedList* list, const LinkedList* other);
LinkedList* list_copy(const LinkedList* list);

// --- Aggregation Functions ---
ListResult list_min(const LinkedList* list, void* out_min);
ListResult list_max(const LinkedList* list, void* out_max);
ListResult list_sum(const LinkedList* list, void* out_sum);
LinkedList* list_filter(const LinkedList* list, FilterFunction filter_fn);

// --- Transformation Functions ---
LinkedList* list_map(const LinkedList* list, MapFunction map_fn, size_t new_element_size);
LinkedList* list_slice(const LinkedList* list, size_t start, size_t end);
LinkedList* list_concat(const LinkedList* list1, const LinkedList* list2);

// --- I/O and Format Functions ---
char* list_to_string(const LinkedList* list, const char* separator);
ListResult list_save_to_file(const LinkedList* list, const char* filename);
LinkedList* list_load_from_file(const char* filename, size_t element_size,
                                PrintFunction print_fn, CompareFunction compare_fn,
                                FreeFunction free_fn, CopyFunction copy_fn);

// --- Mathematical Functions ---
LinkedList* list_unique(const LinkedList* list);
LinkedList* list_intersection(const LinkedList* list1, const LinkedList* list2);
LinkedList* list_union(const LinkedList* list1, const LinkedList* list2);
ListResult list_rotate(LinkedList* list, int positions);

// --- Array to List Conversion Functions ---
ListResult array_to_list(LinkedList* list, const void* arr, size_t n);
void* list_to_array(const LinkedList* list, size_t* out_size);

// --- Helper Functions for Basic Data Types ---

// Integer helpers
void print_int(void* data);
int compare_int(const void* a, const void* b);
LinkedList* list_create_int(void);

// Double helpers  
void print_double(void* data);
int compare_double(const void* a, const void* b);
LinkedList* list_create_double(void);

// Character helpers
void print_char(void* data);
int compare_char(const void* a, const void* b);
LinkedList* list_create_char(void);

// String helpers (char*)
LinkedList* list_create_string(void);

// --- Standalone Node-based list functions ---
int recursive_length(Node *node);
void replace_matches(Node *node, int find_value, int replace_value);
void delete_all_matches(Node **head, int value_to_delete);
bool efficient_delete_match(Node **head, int value);
bool insert_after(Node *node, int data);
Node* add_lists(Node *list1, Node *list2);
Node *merge_sorted_lists(Node *list1, Node *list2);

#endif // LINKED_LIST_H