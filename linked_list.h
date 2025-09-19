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

// Portable-ish deprecation macro (compiler hint). Not critical if unsupported.
#if defined(__GNUC__) || defined(__clang__)
#define LL_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define LL_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define LL_DEPRECATED(msg)
#endif

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
 * @brief A function pointer type for checking a condition on an element.
 * @param element A const void pointer to the element's data to test.
 * @return True if the condition is met, false otherwise.
 */
typedef bool (*PredicateFunction)(const void* element);

/**
 * @brief Internal structure representing a node in the linked list.
 * @note Users of the library should not manipulate this structure directly.
 */
typedef struct Node {
    void* data;          /**< Pointer to the data stored in the node. */
    struct Node* next;   /**< Pointer to the next node in the list. */
    struct Node* prev;   /**< Pointer to the previous node in the list. */
    ListMemoryMode mode; /**< How the data is managed: value copy (owned) or external pointer (ownership transferred). */
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
    FreeFunction free_node_function;     /**< Function to free a complex element. */
    CopyFunction copy_node_function;     /**< Function to deep copy a complex element. */
} LinkedList;


// --- Error Handling ---
const char* error_string(ListResult result);

// --- Lifecycle Functions ---
LinkedList* create_list(size_t element_size);

// --- Setters for LinkedList fields ---
void set_print_function(LinkedList* list, PrintFunction print_fn);
void set_free_function(LinkedList* list, FreeFunction free_fn);
void set_copy_function(LinkedList* list, CopyFunction copy_fn);

// --- Size and Overwrite Management ---
ListResult set_max_size(LinkedList* list, size_t max_size, OverflowBehavior behavior);

// --- Insertion Functions ---
// Internal functions (used by macros) - copy data into list-managed memory
ListResult insert_head_value_internal(LinkedList* list, void* data);
ListResult insert_tail_value_internal(LinkedList* list, void* data);
ListResult insert_index_value_internal(LinkedList* list, size_t index, void* data);

ListResult insert_head_ptr(LinkedList* list, void* data_ptr);
ListResult insert_tail_ptr(LinkedList* list, void* data_ptr);
ListResult insert_index_ptr(LinkedList* list, size_t index, void* data_ptr);

// --- Deletion Functions ---
ListResult delete_head(LinkedList* list);
ListResult delete_tail(LinkedList* list);
ListResult delete_index(LinkedList* list, size_t index);
ListResult remove_advanced(LinkedList* list, int count, Direction direction, FilterFunction predicate);
ListResult clear(LinkedList* list);
void destroy(LinkedList* list);

// --- Utility Functions ---
size_t get_length(const LinkedList* list);
bool is_empty(const LinkedList* list);
ListResult print_list(const LinkedList* list);
ListResult print_list_advanced(const LinkedList* list, bool show_size, bool show_index, const char* separator);

// --- Search and Access Functions ---
void* get(const LinkedList* list, size_t index);

// Generic field setting function - internal use by macro
ListResult set_field_generic(LinkedList* list, size_t index, size_t field_offset, size_t field_size, const void* new_value);

// Macro for setting individual fields in structs
#define set_field(list, index, struct_type, field_name, new_value) \
    do { \
        __typeof__(((struct_type*)0)->field_name) temp_value = (new_value); \
        set_field_generic(list, index, offsetof(struct_type, field_name), \
                         sizeof(((struct_type*)0)->field_name), &temp_value); \
    } while(0)

int index_of(const LinkedList* list, PredicateFunction predicate);
int index_of_advanced(const LinkedList* list, Direction direction, PredicateFunction predicate);

// --- Sorting and Manipulation Functions ---
ListResult sort_list(LinkedList* list, CompareFunction compare_fn);

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
void* min_by(const LinkedList* list, int (*compare)(const void *a, const void *b));
void* max_by(const LinkedList* list, int (*compare)(const void *a, const void *b));
size_t count_if(const LinkedList* list, PredicateFunction predicate);
LinkedList* unique(const LinkedList* list, CompareFunction compare_fn);
LinkedList* unique_advanced(const LinkedList* list, CompareFunction compare_fn, Direction order);
LinkedList* intersection(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn);
LinkedList* union_lists(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn);

// --- Array to List Conversion Functions ---
ListResult from_array(LinkedList* list, const void* arr, size_t n);
void* to_array(const LinkedList* list, size_t* out_size);

// --- I/O and Format Functions ---
// to_string: quick, human-readable join of primitive values (int/double/char) using 'separator'.
//   Not a lossless serializer: complex element sizes become the literal token "[data]".
//   Caller must free the returned char*.
char* to_string(const LinkedList* list, const char* separator);

// File formats for persistence
typedef enum {
    FILE_FORMAT_BINARY = 0,  // Existing binary format
    FILE_FORMAT_TEXT   = 1   // Human-readable text (one element per line)
} FileFormat;

// save_to_file / load_from_file (unified API):
//   format == FILE_FORMAT_BINARY -> binary layout: [size_t length][size_t element_size][raw bytes...]
//   format == FILE_FORMAT_TEXT   -> human readable tokens.
// TEXT MODE RULES:
//   * Primitive element sizes (int/double/char) -> printed plainly.
//   * Other sizes -> hex dump (space separated bytes) when using whitespace tokenization.
//   * 'separator' (default "\n") placed BETWEEN elements. If NULL/empty during load -> whitespace mode.
//   * Custom separator load currently supports only primitive types (no hex parsing there).
// Portability: binary not endian/size_t portable (demo/educational). For production add magic header + fixed widths.
ListResult save_to_file(const LinkedList* list, const char* filename, FileFormat format, const char* separator);
LinkedList* load_from_file(const char* filename, size_t element_size, FileFormat format,
                           const char* separator,
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

#ifdef __GNUC__  // GCC and Clang support __typeof__ for type inference
    #define insert_head_value(list, value) do { \
        __typeof__(value) _temp = (value); \
        insert_head_value_internal((list), &_temp); \
    } while(0)
    #define insert_tail_value(list, value) do { \
        __typeof__(value) _temp = (value); \
        insert_tail_value_internal((list), &_temp); \
    } while(0)
    #define insert_index_value(list, index, value) do { \
        __typeof__(value) _temp = (value); \
        insert_index_value_internal((list), (index), &_temp); \
    } while(0)
#else
    // Fallback for compilers that don't support __typeof__
    #define insert_head_value(list, value) do { \
        insert_head_value_internal((list), &(value)); \
    } while(0)
    #define insert_tail_value(list, value) do { \
        insert_tail_value_internal((list), &(value)); \
    } while(0)
    #define insert_index_value(list, index, value) do { \
        insert_index_value_internal((list), (index), &(value)); \
    } while(0)
#endif

#endif
// EOF
