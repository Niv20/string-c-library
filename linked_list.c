/**
 * @file linked_list.c
 * @author Niv Libovitch
 * @date 25 Aug 2025
 * @brief Implementation of the generic doubly linked list library.
 *
 * This file contains the function definitions for the linked list operations
 * declared in linked_list.h. It handles memory management and list manipulation.
 */

#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Function only for internal use
static ListResult handle_size_limit(LinkedList*);
static Node* create_node_with_data(LinkedList*, void*);
static ListResult insert_node_core(LinkedList*, void*, Node**);  // Core insertion helper
static ListResult delete_node_core(LinkedList*, Node*);          // Core deletion helper

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃                Error Handling                 ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Converts a ListResult error code to a human-readable string.
 * @param result The error code to convert.
 * @return A string describing the error.
 */
const char* list_error_string(ListResult result) {
    switch (result) {
        case LIST_SUCCESS: return "Success";
        case LIST_ERROR_NULL_POINTER: return "NULL pointer provided";
        case LIST_ERROR_MEMORY_ALLOC: return "Memory allocation failed";
        case LIST_ERROR_INDEX_OUT_OF_BOUNDS: return "Index out of bounds";
        case LIST_ERROR_ELEMENT_NOT_FOUND: return "Element not found";
        case LIST_ERROR_LIST_FULL: return "List has reached maximum capacity";
        case LIST_ERROR_OVERWRITE_DISABLED: return "Overwrite is disabled and list is full";
        case LIST_ERROR_INVALID_OPERATION: return "Invalid operation for current state";
        case LIST_ERROR_NO_COMPARE_FUNCTION: return "Compare function required but not provided";
        case LIST_ERROR_NO_PRINT_FUNCTION: return "Print function required but not provided";
        case LIST_ERROR_NO_FREE_FUNCTION: return "Free function required but not provided";
        case LIST_ERROR_NO_COPY_FUNCTION: return "Copy function required but not provided";
        default: return "Unknown error";
    }
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃                  Create List                  ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Creates and initializes a new linked list. The data is pointer to structures.
 * @param element_size The size of each element in bytes.
 * @return A pointer to the newly created LinkedList, or NULL on failure.
 */
LinkedList* list_create(size_t element_size) {

    // Allocate memory for the list structure
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (!list) return NULL;

    // Create dummy nodes
    list->head = (Node*)calloc(1, sizeof(Node));
    if (!list->head) {
        free(list);
        return NULL;
    }
    list->tail = (Node*)calloc(1, sizeof(Node));
    if (!list->tail) {
        free(list->head);
        free(list);
        return NULL;
    }

    // Link dummy nodes
    list->head->next = list->tail;
    list->tail->prev = list->head;

    // Initialize all fields in the list with default values
    list->length = 0;
    list->element_size = element_size;
    list->max_size = UNLIMITED;             // Unlimited by default
    list->allow_overwrite = FALSE;          // No overwrite by default
    list->print_node_function = NULL;      
    list->compare_node_function = NULL;    
    list->free_node_function = NULL;       
    list->copy_node_function = NULL;       

    return list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃             List Configuration                ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Sets the print function for the list.
 * @param list The list to configure.
 * @param print_fn Function pointer for printing elements.
 */
void list_set_print_function(LinkedList* list, PrintFunction print_fn) {
    if (list)
        list->print_node_function = print_fn;
}

/**
 * @brief Sets the compare function for the list.
 * @param list The list to configure.
 * @param compare_fn Function pointer for comparing elements.
 */
void list_set_compare_function(LinkedList* list, CompareFunction compare_fn) {
    if (list)
        list->compare_node_function = compare_fn;
}

/**
 * @brief Sets the free function for the list.
 * @param list The list to configure.
 * @param free_fn Function pointer for freeing complex elements.
 */
void list_set_free_function(LinkedList* list, FreeFunction free_fn) {
    if (list)
        list->free_node_function = free_fn;
}

/**
 * @brief Sets the copy function for the list.
 * @param list The list to configure.
 * @param copy_fn Function pointer for deep copying complex elements.
 */
void list_set_copy_function(LinkedList* list, CopyFunction copy_fn) {
    if (list)
        list->copy_node_function = copy_fn;
}

/**
 * @brief Sets the maximum size of the list and overwrite behavior.
 * @param list The list to configure.
 * @param max_size Maximum number of elements (0 = unlimited).
 * @param allow_overwrite Whether to overwrite oldest elements when full.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_set_max_size(LinkedList* list, size_t max_size, bool allow_overwrite) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;

    if (max_size <= 0) return LIST_ERROR_INVALID_OPERATION;

    // Update the list's configuration first
    list->max_size = max_size;
    list->allow_overwrite = allow_overwrite;

    // Check if we need to remove excess elements due to new size limit
    return handle_size_limit(list);
}

// INTERNAL HELPER FUNCTION for handling size limits
static ListResult handle_size_limit(LinkedList* list) {

    // If current length is within limits - no action needed
    if (list->length < list->max_size) return LIST_SUCCESS;

    // If we reach this point, the list is full, but overwrite is not allowed
    if (!list->allow_overwrite) return LIST_ERROR_LIST_FULL;

    // Handle any case where we need to remove elements to stay within limit
    while (list->length >= list->max_size) {

        // Remove oldest element (from head) to make room for FIFO behavior
        ListResult result = list_delete_from_head(list);
        if (result != LIST_SUCCESS) return result;
    }
    
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃           Insertion in Linked List            ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL HELPER FUNCTION for creating a new node with data
static Node* create_node_with_data(LinkedList* list, void* data) {
    
    // Create a new node
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return NULL;

    // Allocate memory for the data
    new_node->data = malloc(list->element_size);
    if (!new_node->data) {
        free(new_node);
        return NULL;
    }
    
    // Copy the data using appropriate method
    if (list->copy_node_function) {
        list->copy_node_function(new_node->data, data);
    } else {
        memcpy(new_node->data, data, list->element_size);
    }
    
    // Initialize pointers
    new_node->next = NULL;
    new_node->prev = NULL;
    
    return new_node;
}

// INTERNAL CORE HELPER FUNCTION for insertion logic
static ListResult insert_node_core(LinkedList* list, void* data, Node** out_new_node) {
    
    // Input validation
    if (!list || !data) return LIST_ERROR_NULL_POINTER;

    // Check size limits before insertion
    ListResult size_check = handle_size_limit(list);
    if (size_check != LIST_SUCCESS) return size_check;

    // Create new node with data
    Node* new_node = create_node_with_data(list, data);
    if (!new_node) return LIST_ERROR_MEMORY_ALLOC;
    
    *out_new_node = new_node;
    return LIST_SUCCESS;
}

/**
 * @brief Inserts a new element at the head of the list.
 * @param list The list to insert into.
 * @param data A pointer to the data to be inserted. The data is copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_at_head(LinkedList* list, void* data) {
    
    Node* new_node;
    ListResult result = insert_node_core(list, data, &new_node);
    if (result != LIST_SUCCESS) return result;

    // Link the new node at the head
    Node* old_first = list->head->next;
    list->head->next = new_node;
    new_node->prev = list->head;
    new_node->next = old_first;
    old_first->prev = new_node;
    
    list->length++;
    return LIST_SUCCESS;
}

/**
 * @brief Inserts a new element at the tail of the list.
 * @param list The list to insert into.
 * @param data A pointer to the data to be inserted. The data is copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_at_tail(LinkedList* list, void* data) {
    
    Node* new_node;
    ListResult result = insert_node_core(list, data, &new_node);
    if (result != LIST_SUCCESS) return result;

    // Link the new node at the tail
    Node* old_last = list->tail->prev;
    list->tail->prev = new_node;
    new_node->next = list->tail;
    new_node->prev = old_last;
    old_last->next = new_node;
    
    list->length++;
    return LIST_SUCCESS;
}

/**
 * @brief Inserts an element at a specific index (like Python's insert).
 * @param list The list to insert into.
 * @param index The index to insert at (0-based).
 * @param data The data to insert.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_at_index(LinkedList* list, size_t index, void* data) {

    Node* new_node;
    ListResult result = insert_node_core(list, data, &new_node);
    if (result != LIST_SUCCESS) return result;
    
    Node* current;

    // Choose direction based on which half the index is in
    if (index <= list->length / 2) {
        // Index is in first half - traverse forward from head
        current = list->head->next;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        // Index is in second half - traverse backward from tail
        current = list->tail->prev;
        for (size_t i = list->length - 1; i > index; i--) {
            current = current->prev;
        }
    }
    
    // Insert before current
    new_node->next = current;
    new_node->prev = current->prev;
    current->prev->next = new_node;
    current->prev = new_node;
    
    list->length++;
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              Deletion Functions               ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL CORE HELPER FUNCTION for deletion logic
static ListResult delete_node_core(LinkedList* list, Node* node_to_delete) {
    
    if (!list || !node_to_delete) return LIST_ERROR_NULL_POINTER;
    if (node_to_delete == list->head || node_to_delete == list->tail) {
        return LIST_ERROR_INVALID_OPERATION; // Cannot delete dummy nodes
    }

    // Re-link the list around the node
    Node* prev_node = node_to_delete->prev;
    Node* next_node = node_to_delete->next;
    prev_node->next = next_node;
    next_node->prev = prev_node;
    
    // Free the data using free function if provided
    if (list->free_node_function) {
        list->free_node_function(node_to_delete->data);
    } else {
        free(node_to_delete->data);
    }

    // Free the node itself
    free(node_to_delete);
    
    list->length--;
    return LIST_SUCCESS;
}

/**
 * @brief Deletes the element at the head of the list.
 * @param list The list to delete from.
 * @return LIST_SUCCESS on success, error code if the list is empty.
 */
ListResult list_delete_from_head(LinkedList* list) {

    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Use core deletion logic
    return delete_node_core(list, list->head->next);
}


/**
 * @brief Deletes the element at the tail of the list.
 * @param list The list to delete from.
 * @return LIST_SUCCESS on success, error code if the list is empty.
 */
ListResult list_delete_from_tail(LinkedList* list) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Use core deletion logic
    return delete_node_core(list, list->tail->prev);
}

/**
 * @brief Deletes an element at a specific index.
 * @param list The list to delete from.
 * @param index The index to delete at (0-based).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_delete_at_index(LinkedList* list, size_t index) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    Node* current;
    
    // Choose direction based on which half the index is in
    if (index <= list->length / 2) {
        // Index is in first half - traverse forward from head
        current = list->head->next;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        // Index is in second half - traverse backward from tail
        current = list->tail->prev;
        for (size_t i = list->length - 1; i > index; i--) {
            current = current->prev;
        }
    }
    
    // Use core deletion logic
    return delete_node_core(list, current);
}


/**
 * @brief Advanced removal function with multiple options.
 * @param list The list to remove from.
 * @param data The data to find and remove.
 * @param count Number of occurrences to remove (DELETE_ALL_OCCURRENCES for all).
 * @param direction SEARCH_FROM_HEAD or SEARCH_FROM_TAIL.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult list_remove_advanced(LinkedList* list, void* data, int count, int direction) {
    
    if (!list || !data) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_ELEMENT_NOT_FOUND;
    if (!list->compare_node_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    
    int removed_count = 0;
    Node* current;
    Node* end_sentinel;
    
    // Set up traversal direction
    if (direction == SEARCH_FROM_TAIL) {
        current = list->tail->prev;
        end_sentinel = list->head;
    } else {
        current = list->head->next;
        end_sentinel = list->tail;
    }
    
    // Traverse and remove matching elements
    while (current != end_sentinel && (count == DELETE_ALL_OCCURRENCES || removed_count < count)) {
        
        Node* next_node = (direction == SEARCH_FROM_TAIL) ? current->prev : current->next;
        
        if (list->compare_node_function(current->data, data) == 0) {
            // Found a match - remove it using core deletion logic
            ListResult result = delete_node_core(list, current);
            if (result == LIST_SUCCESS) {
                removed_count++;
            }
        }
        
        current = next_node;
    }
    
    return (removed_count > 0) ? LIST_SUCCESS : LIST_ERROR_ELEMENT_NOT_FOUND;
}


/**
 * @brief Clears all elements from the list (like Python's clear).
 * @param list The list to clear.
 */
void list_clear(LinkedList* list) {
    
    if (!list) return;
    
    while (!list_is_empty(list))
        list_delete_from_head(list);
}

/**
 * @brief Frees all memory associated with the list.
 * This includes all nodes and the data within them (using the free_function if provided).
 * @param list A pointer to the LinkedList to be destroyed.
 */
void list_destroy(LinkedList* list) {
    
    if (!list) return;

    // Clear all real nodes
    list_clear(list);

    // Free the dummy nodes
    free(list->head);
    free(list->tail);
    
    // Finally, free the list manager itself
    free(list);
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃               Utility Functions               ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Returns the number of elements in the list.
 * @param list The list to query.
 * @return The number of elements.
 */
size_t list_get_length(const LinkedList* list) {
    return list ? list->length : 0;
}

/**
 * @brief Checks if the list is empty.
 * @param list The list to check.
 * @return TRUE if the list is empty, FALSE otherwise.
 */
bool list_is_empty(const LinkedList* list) {
    return !list || list->length == 0;
}

/**
 * @brief Prints all elements in the list using default formatting (with indices and newlines).
 * @param list The list to print.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_print(const LinkedList* list) {
    return list_print_advanced(list, TRUE, "\n");
}

/**
 * @brief Prints all elements in the list with customizable formatting.
 * @param list The list to print.
 * @param show_index Whether to show element indices.
 * @param separator String to print between elements.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_print_advanced(const LinkedList* list, bool show_index, const char* separator) {

    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_ELEMENT_NOT_FOUND;
    if (!list->print_node_function) return LIST_ERROR_NO_PRINT_FUNCTION;
    
    if (!separator) separator = "\n"; // Default separator

    // Determine starting and ending points (always dummy nodes)
    Node* current_node = list->head->next;
    Node* end_node = list->tail;

    if (show_index)
        printf("List len: %zu\n", list->length);

    size_t index = 0;
    while(current_node != end_node) {

        // Print the index, if required
        if (show_index)
            printf("  [%zu]: ", index++);

        // Print the element data itself
        list->print_node_function(current_node->data);

        // Move to the next node
        current_node = current_node->next;

        // Print the separator (if not the last element)
        if (current_node != end_node)
            printf("%s", separator);
    }

    // Edge case: if using custom separator, ensure it ends with a newline
    if (strcmp(separator, "\n") != 0)
        printf("\n");
    
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃         Search and Access Functions           ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL HELPER FUNCTION for finding node by index
static Node* find_node_by_index(const LinkedList* list, size_t index) {
    
    if (!list || index >= list->length) return NULL;
    
    Node* current;

    // Choose direction based on which half the index is in
    if (index <= list->length / 2) {
        // Index is in first half - traverse forward from head
        current = list->head->next;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        // Index is in second half - traverse backward from tail
        current = list->tail->prev;
        for (size_t i = list->length - 1; i > index; i--) {
            current = current->prev;
        }
    }
    
    return current;
}


/**
 * @brief Gets a pointer to an element at a specific index (direct access without copying).
 * @param list The list to get from.
 * @param index The index to get.
 * @return Pointer to the data at the specified index, or NULL on failure.
 * @warning The returned pointer is valid only as long as the list structure remains unchanged.
 */
void* list_get(const LinkedList* list, size_t index) {
    
    if (!list || index >= list->length) return NULL;
    
    Node* current = find_node_by_index(list, index);
    return current ? current->data : NULL;
}

/**
 * @brief Sets an element at a specific index (like Python's list[index] = value).
 * @param list The list to set in.
 * @param index The index to set.
 * @param data The data to set.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_set(LinkedList* list, size_t index, void* data) {
    
    if (!list || !data) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    if (!list->free_node_function) return LIST_ERROR_NO_FREE_FUNCTION;

    Node* current = find_node_by_index(list, index);
    if (!current) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    // Free old data
    list->free_node_function(current->data);
    
    // Copy new data
    if (list->copy_node_function) {
        list->copy_node_function(current->data, data);
    } else {
        memcpy(current->data, data, list->element_size);
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Returns the index of the first occurrence of a value (like Python's index).
 * @param list The list to search in.
 * @param data The data to find.
 * @return The index if found, negative error code if error occurred.
 */
int list_index(const LinkedList* list, void* data) {
    return list_index_advanced(list, data, SEARCH_FROM_HEAD);
}

/**
 * @brief Returns the index of the first/last occurrence of a value with direction control.
 * @param list The list to search in.
 * @param data The data to find.
 * @param direction SEARCH_FROM_HEAD (default) or SEARCH_FROM_TAIL.
 * @return The index if found, negative error code if error occurred.
 */
int list_index_advanced(const LinkedList* list, void* data, int direction) {
    if (!list) return -LIST_ERROR_NULL_POINTER;
    if (!data) return -LIST_ERROR_NULL_POINTER;
    if (!list->compare_node_function) return -LIST_ERROR_NO_COMPARE_FUNCTION;
    
    if (direction == SEARCH_FROM_TAIL) {
        // Search from tail to head
        Node* current = list->tail->prev;
        size_t index = list->length - 1;
        while (current != list->head) {
            if (list->compare_node_function(current->data, data) == 0) {
                return (int)index;
            }
            current = current->prev;
            if (index == 0) break; // Prevent underflow
            index--;
        }
    } else {
        // Search from head to tail (default)
        Node* current = list->head->next;
        size_t index = 0;
        while (current != list->tail) {
            if (list->compare_node_function(current->data, data) == 0) {
                return (int)index;
            }
            current = current->next;
            index++;
        }
    }
    return -LIST_ERROR_ELEMENT_NOT_FOUND; // Element not found
}

/**
 * @brief Counts the number of occurrences of a value (like Python's count).
 * @param list The list to count in.
 * @param data The data to count.
 * @return The number of occurrences.
 */
size_t list_count_occurrences(const LinkedList* list, void* data) {
    
    if (!list || !data || !list->compare_node_function) return 0;
    
    size_t count = 0;
    Node* current = list->head->next;
    while (current != list->tail) {
        if (list->compare_node_function(current->data, data) == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃       Sorting and Manipulation Functions      ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Reverses the list in place (like Python's reverse).
 * @param list The list to reverse.
 */
void list_reverse(LinkedList* list) {
    if (!list || list->length <= 1) return;
    
    Node* current = list->head->next;
    Node* prev_node = list->head;
    
    // Reverse all links
    while (current != list->tail) {
        Node* next_node = current->next;
        current->next = prev_node;
        current->prev = next_node;
        prev_node = current;
        current = next_node;
    }
    
    // Update dummy nodes
    list->head->next = prev_node;
    prev_node->prev = list->head;
    list->tail->prev = list->head->next;
    
    // Fix the last node
    current = list->head->next;
    while (current->next != list->head) {
        current = current->next;
    }
    current->next = list->tail;
    list->tail->prev = current;
}

// Static variables for qsort comparison context
static CompareFunction g_compare_function = NULL;
static bool g_reverse_order = FALSE;

/* 
 * Decision to use qsort with array conversion:
 * While this requires O(n) additional memory (not space-efficient), modern computers
 * don't have memory constraints, so I prefer fast O(n log n) over slow O(n²).
 * The time savings are dramatic - for 1000 elements: qsort ~10K operations vs bubble sort ~1M operations.
 * The cost of two conversions (list->array->list) is negligible compared to the runtime performance gain.
 */

// Helper function for qsort comparison
static int qsort_compare_wrapper(const void* a, const void* b) {
    if (!g_compare_function) return 0;
    int result = g_compare_function(a, b);
    return g_reverse_order ? -result : result;
}

/**
 * @brief Sorts the list in place (like Python's sort).
 * @param list The list to sort.
 * @param reverse If TRUE, sorts in descending order.
 */
void list_sort(LinkedList* list, bool reverse_order) {
    if (!list || !list->compare_node_function || list->length <= 1) return;
    
    // Convert list to array for qsort
    size_t array_size;
    void* array = list_to_array(list, &array_size);
    if (!array) return;
    
    // Set up comparison context
    g_compare_function = list->compare_node_function;
    g_reverse_order = reverse_order;
    
    // Sort the array using qsort
    qsort(array, array_size, list->element_size, qsort_compare_wrapper);
    
    // Convert array back to list
    array_to_list(list, array, array_size);
    
    // Clean up
    g_compare_function = NULL;
    g_reverse_order = FALSE;
    free(array);
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃            List Operations Functions          ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Extends the list with elements from another list (like Python's extend).
 * @param list The list to extend.
 * @param other The other list to extend with.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_extend(LinkedList* list, const LinkedList* other) {
    if (!list || !other) return LIST_ERROR_NULL_POINTER;
    
    Node* current = other->head->next;
    while (current != other->tail) {
        ListResult result = list_insert_at_tail(list, current->data);
        if (result != LIST_SUCCESS) {
            return result;
        }
        current = current->next;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Creates a copy of the list (like Python's list.copy()).
 * @param list The list to copy.
 * @return A new list that is a copy of the original, or NULL on failure.
 */
LinkedList* list_copy(const LinkedList* list) {
    
    if (!list) return NULL;
    
    LinkedList* new_list = list_create(list->element_size);
    if (!new_list) return NULL;
    
    // Configure the new list with same settings as the original
    list_set_print_function(new_list, list->print_node_function);
    list_set_compare_function(new_list, list->compare_node_function);
    list_set_free_function(new_list, list->free_node_function);
    list_set_copy_function(new_list, list->copy_node_function);
    
    // Copy all elements from original list
    ListResult extend_result = list_extend(new_list, list);
    if (extend_result != LIST_SUCCESS) {
        list_destroy(new_list);
        return NULL;
    }
    
    return new_list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃             Aggregation Functions             ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Finds the minimum element in the list.
 * @param list The list to search in.
 * @param out_min Buffer to store the minimum element.
 * @return LIST_SUCCESS if found, error code otherwise.
 */
ListResult list_min(const LinkedList* list, void* out_min) {
    if (!list || !out_min) return LIST_ERROR_NULL_POINTER;
    if (!list->compare_node_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    Node* current = list->head->next;
    
    // Initialize with first element
    if (list->copy_node_function) {
        list->copy_node_function(out_min, current->data);
    } else {
        memcpy(out_min, current->data, list->element_size);
    }
    
    current = current->next;
    
    // Compare with rest of elements
    while (current != list->tail) {
        if (list->compare_node_function(current->data, out_min) < 0) {
            if (list->copy_node_function) {
                list->copy_node_function(out_min, current->data);
            } else {
                memcpy(out_min, current->data, list->element_size);
            }
        }
        current = current->next;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Finds the maximum element in the list.
 * @param list The list to search in.
 * @param out_max Buffer to store the maximum element.
 * @return LIST_SUCCESS if found, error code otherwise.
 */
ListResult list_max(const LinkedList* list, void* out_max) {
    if (!list || !out_max) return LIST_ERROR_NULL_POINTER;
    if (!list->compare_node_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    Node* current = list->head->next;
    
    // Initialize with first element
    if (list->copy_node_function) {
        list->copy_node_function(out_max, current->data);
    } else {
        memcpy(out_max, current->data, list->element_size);
    }
    
    current = current->next;
    
    // Compare with rest of elements
    while (current != list->tail) {
        if (list->compare_node_function(current->data, out_max) > 0) {
            if (list->copy_node_function) {
                list->copy_node_function(out_max, current->data);
            } else {
                memcpy(out_max, current->data, list->element_size);
            }
        }
        current = current->next;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Calculates the sum of all elements (for numeric types).
 * @param list The list to sum.
 * @param out_sum Buffer to store the sum result.
 * @return LIST_SUCCESS on success, error code otherwise.
 */
ListResult list_sum(const LinkedList* list, void* out_sum) {
    if (!list || !out_sum) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    // Initialize sum to zero (works for most numeric types)
    memset(out_sum, 0, list->element_size);
    
    Node* current = list->head->next;
    while (current != list->tail) {
        // For common numeric types
        if (list->element_size == sizeof(int)) {
            *(int*)out_sum += *(int*)current->data;
        } else if (list->element_size == sizeof(double)) {
            *(double*)out_sum += *(double*)current->data;
        } else if (list->element_size == sizeof(float)) {
            *(float*)out_sum += *(float*)current->data;
        } else if (list->element_size == sizeof(long)) {
            *(long*)out_sum += *(long*)current->data;
        } else {
            return LIST_ERROR_INVALID_OPERATION; // Unsupported type
        }
        current = current->next;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Creates a new list with elements that pass the filter function.
 * @param list The source list.
 * @param filter_fn Function to test each element.
 * @return A new filtered list, or NULL on failure.
 */
LinkedList* list_filter(const LinkedList* list, FilterFunction filter_fn) {
    if (!list || !filter_fn) return NULL;
    
    LinkedList* filtered = list_create(list->element_size);
    if (!filtered) return NULL;
    
    // Configure the filtered list with same settings as the original
    list_set_print_function(filtered, list->print_node_function);
    list_set_compare_function(filtered, list->compare_node_function);
    list_set_free_function(filtered, list->free_node_function);
    list_set_copy_function(filtered, list->copy_node_function);
    
    Node* current = list->head->next;
    while (current != list->tail) {
        if (filter_fn(current->data)) {
            if (list_insert_at_tail(filtered, current->data) != LIST_SUCCESS) {
                list_destroy(filtered);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return filtered;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              Iteration Functions              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Applies a function to each element in the list (forward iteration).
 * @param list The list to iterate over.
 * @param action Function to call on each element.
 */
void list_for_each(const LinkedList* list, ForEachFunction action) {
    if (!list || !action) return;
    
    Node* current = list->head->next;
    size_t index = 0;
    
    while (current != list->tail) {
        action(current->data, index);
        current = current->next;
        index++;
    }
}

/**
 * @brief Applies a function to each element in the list (reverse iteration).
 * @param list The list to iterate over.
 * @param action Function to call on each element.
 */
void list_for_each_reverse(const LinkedList* list, ForEachFunction action) {
    if (!list || !action) return;
    
    Node* current = list->tail->prev;
    size_t index = list->length - 1;
    
    while (current != list->head) {
        action(current->data, index);
        current = current->prev;
        index--;
    }
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃           Transformation Functions            ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Creates a new list with transformed elements.
 * @param list The source list.
 * @param map_fn Function to transform each element.
 * @param new_element_size Size of elements in the new list.
 * @return A new transformed list, or NULL on failure.
 */
LinkedList* list_map(const LinkedList* list, MapFunction map_fn, size_t new_element_size) {
    if (!list || !map_fn) return NULL;
    
    LinkedList* mapped = list_create(new_element_size);
    if (!mapped) return NULL;
    
    // Configure the mapped list
    list_set_free_function(mapped, list->free_node_function);
    list_set_copy_function(mapped, list->copy_node_function);
    
    Node* current = list->head->next;
    while (current != list->tail) {
        void* transformed = malloc(new_element_size);
        if (!transformed) {
            list_destroy(mapped);
            return NULL;
        }
        
        map_fn(transformed, current->data);
        
        if (list_insert_at_tail(mapped, transformed) != LIST_SUCCESS) {
            free(transformed);
            list_destroy(mapped);
            return NULL;
        }
        
        free(transformed);
        current = current->next;
    }
    
    return mapped;
}

/**
 * @brief Creates a new list containing a slice of the original.
 * @param list The source list.
 * @param start Start index (inclusive).
 * @param end End index (exclusive).
 * @return A new sliced list, or NULL on failure.
 */
LinkedList* list_slice(const LinkedList* list, size_t start, size_t end) {
    if (!list || start >= end || start >= list->length) return NULL;
    
    if (end > list->length) end = list->length;
    
    LinkedList* sliced = list_create(list->element_size);
    if (!sliced) return NULL;
    
    // Configure the sliced list with same settings as the original
    list_set_print_function(sliced, list->print_node_function);
    list_set_compare_function(sliced, list->compare_node_function);
    list_set_free_function(sliced, list->free_node_function);
    list_set_copy_function(sliced, list->copy_node_function);
    
    Node* current = list->head->next;
    
    // Skip to start position
    for (size_t i = 0; i < start && current != list->tail; i++) {
        current = current->next;
    }
    
    // Copy elements from start to end
    for (size_t i = start; i < end && current != list->tail; i++) {
        if (list_insert_at_tail(sliced, current->data) != LIST_SUCCESS) {
            list_destroy(sliced);
            return NULL;
        }
        current = current->next;
    }
    
    return sliced;
}

/**
 * @brief Creates a new list by concatenating two lists.
 * @param list1 First list.
 * @param list2 Second list.
 * @return A new concatenated list, or NULL on failure.
 */
LinkedList* list_concat(const LinkedList* list1, const LinkedList* list2) {
    if (!list1 || !list2) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    LinkedList* concatenated = list_create(list1->element_size);
    if (!concatenated) return NULL;
    
    // Configure the concatenated list with settings from first list
    list_set_print_function(concatenated, list1->print_node_function);
    list_set_compare_function(concatenated, list1->compare_node_function);
    list_set_free_function(concatenated, list1->free_node_function);
    list_set_copy_function(concatenated, list1->copy_node_function);
    
    // Copy all elements from first list
    if (list_extend(concatenated, list1) != LIST_SUCCESS) {
        list_destroy(concatenated);
        return NULL;
    }
    
    // Copy all elements from second list
    if (list_extend(concatenated, list2) != LIST_SUCCESS) {
        list_destroy(concatenated);
        return NULL;
    }
    
    return concatenated;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃           I/O and Format Functions            ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Converts the list to a string representation.
 * @param list The list to convert.
 * @param separator String to separate elements.
 * @return A newly allocated string, or NULL on failure.
 * @note The caller is responsible for freeing the returned string.
 */
char* list_to_string(const LinkedList* list, const char* separator) {
    if (!list || !separator) return NULL;
    if (!list->print_node_function) return NULL;
    if (list_is_empty(list)) {
        char* empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    
    // Calculate approximate size needed
    size_t sep_len = strlen(separator);
    size_t estimated_size = list->length * 50 + (list->length - 1) * sep_len + 1;
    char* result = malloc(estimated_size);
    if (!result) return NULL;
    
    result[0] = '\0';
    bool first = TRUE;
    
    Node* current = list->head->next;
    while (current != list->tail) {
        if (!first) {
            strcat(result, separator);
        }
        
        // For basic types, we can convert directly
        if (list->element_size == sizeof(int)) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%d", *(int*)current->data);
            strcat(result, temp);
        } else if (list->element_size == sizeof(double)) {
            char temp[64];
            snprintf(temp, sizeof(temp), "%.2f", *(double*)current->data);
            strcat(result, temp);
        } else if (list->element_size == sizeof(char)) {
            char temp[4];
            snprintf(temp, sizeof(temp), "%c", *(char*)current->data);
            strcat(result, temp);
        } else {
            // For other types, show generic representation
            strcat(result, "[data]");
        }
        
        first = FALSE;
        current = current->next;
    }
    
    return result;
}

/**
 * @brief Saves the list to a file.
 * @param list The list to save.
 * @param filename Path to the output file.
 * @return LIST_SUCCESS on success, error code otherwise.
 */
ListResult list_save_to_file(const LinkedList* list, const char* filename) {
    if (!list || !filename) return LIST_ERROR_NULL_POINTER;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return LIST_ERROR_INVALID_OPERATION;
    
    // Write header information
    fwrite(&list->length, sizeof(size_t), 1, file);
    fwrite(&list->element_size, sizeof(size_t), 1, file);
    
    // Write elements
    Node* current = list->head->next;
    while (current != list->tail) {
        fwrite(current->data, list->element_size, 1, file);
        current = current->next;
    }
    
    fclose(file);
    return LIST_SUCCESS;
}

/**
 * @brief Loads a list from a file.
 * @param filename Path to the input file.
 * @param element_size Size of each element.
 * @param print_fn Print function for the elements.
 * @param compare_fn Compare function for the elements.
 * @param free_fn Free function for the elements (optional).
 * @param copy_fn Copy function for the elements (optional).
 * @return A new list loaded from file, or NULL on failure.
 */
LinkedList* list_load_from_file(const char* filename, size_t element_size,
                                PrintFunction print_fn, CompareFunction compare_fn,
                                FreeFunction free_fn, CopyFunction copy_fn) {
    if (!filename) return NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    
    size_t saved_length, saved_element_size;
    
    // Read header
    if (fread(&saved_length, sizeof(size_t), 1, file) != 1 ||
        fread(&saved_element_size, sizeof(size_t), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    // Verify element size matches
    if (saved_element_size != element_size) {
        fclose(file);
        return NULL;
    }
    
    LinkedList* list = list_create(element_size);
    if (!list) {
        fclose(file);
        return NULL;
    }
    
    // Set the function pointers if provided
    if (print_fn) list_set_print_function(list, print_fn);
    if (compare_fn) list_set_compare_function(list, compare_fn);
    if (free_fn) list_set_free_function(list, free_fn);
    if (copy_fn) list_set_copy_function(list, copy_fn);
    
    // Read elements
    for (size_t i = 0; i < saved_length; i++) {
        void* element = malloc(element_size);
        if (!element || fread(element, element_size, 1, file) != 1) {
            free(element);
            list_destroy(list);
            fclose(file);
            return NULL;
        }
        
        if (list_insert_at_tail(list, element) != LIST_SUCCESS) {
            free(element);
            list_destroy(list);
            fclose(file);
            return NULL;
        }
        
        free(element);
    }
    
    fclose(file);
    return list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃            Mathematical Functions             ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Creates a new list with unique elements (preserves order).
 * @param list The source list.
 * @return A new list with unique elements, or NULL on failure.
 */
LinkedList* list_unique(const LinkedList* list) {
    if (!list || !list->compare_node_function) return NULL;
    
    LinkedList* unique = list_create(list->element_size);
    if (!unique) return NULL;
    
    // Configure the unique list with same settings as the original
    list_set_print_function(unique, list->print_node_function);
    list_set_compare_function(unique, list->compare_node_function);
    list_set_free_function(unique, list->free_node_function);
    list_set_copy_function(unique, list->copy_node_function);
    
    Node* current = list->head->next;
    while (current != list->tail) {
        // Check if element already exists in unique list
        if (list_index(unique, current->data) == -1) {
            if (list_insert_at_tail(unique, current->data) != LIST_SUCCESS) {
                list_destroy(unique);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return unique;
}

/**
 * @brief Creates a new list containing the intersection of two lists.
 * @param list1 First list.
 * @param list2 Second list.
 * @return A new list with common elements, or NULL on failure.
 */
LinkedList* list_intersection(const LinkedList* list1, const LinkedList* list2) {
    if (!list1 || !list2 || !list1->compare_node_function) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    LinkedList* intersection = list_create(list1->element_size);
    if (!intersection) return NULL;
    
    // Configure the intersection list with settings from first list
    list_set_print_function(intersection, list1->print_node_function);
    list_set_compare_function(intersection, list1->compare_node_function);
    list_set_free_function(intersection, list1->free_node_function);
    list_set_copy_function(intersection, list1->copy_node_function);
    
    Node* current = list1->head->next;
    while (current != list1->tail) {
        // If element exists in both lists and not already in result
        if (list_index(list2, current->data) != -1 && 
            list_index(intersection, current->data) == -1) {
            if (list_insert_at_tail(intersection, current->data) != LIST_SUCCESS) {
                list_destroy(intersection);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return intersection;
}

/**
 * @brief Creates a new list containing the union of two lists.
 * @param list1 First list.
 * @param list2 Second list.
 * @return A new list with all unique elements from both lists, or NULL on failure.
 */
LinkedList* list_union(const LinkedList* list1, const LinkedList* list2) {
    if (!list1 || !list2) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    // Start with unique elements from first list
    LinkedList* union_list = list_unique(list1);
    if (!union_list) return NULL;
    
    // Add unique elements from second list
    Node* current = list2->head->next;
    while (current != list2->tail) {
        if (list_index(union_list, current->data) == -1) {
            if (list_insert_at_tail(union_list, current->data) != LIST_SUCCESS) {
                list_destroy(union_list);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return union_list;
}

/**
 * @brief Rotates the list elements by a specified number of positions.
 * @param list The list to rotate.
 * @param positions Number of positions to rotate (positive = right, negative = left).
 * @return LIST_SUCCESS on success, error code otherwise.
 */
ListResult list_rotate(LinkedList* list, int positions) {
    if (!list || list->length <= 1) return LIST_SUCCESS;
    
    // Normalize positions to be within list length
    int actual_positions = positions % (int)list->length;
    if (actual_positions < 0) {
        actual_positions += list->length;
    }
    
    if (actual_positions == 0) return LIST_SUCCESS;
    
    // Find the split point
    Node* split_point = list->head->next;
    for (int i = 0; i < actual_positions; i++) {
        split_point = split_point->next;
    }
    
    // If split point is at the end, no rotation needed
    if (split_point == list->tail) return LIST_SUCCESS;
    
    // Save the nodes we're moving
    Node* first_part_start = list->head->next;
    Node* first_part_end = split_point->prev;
    Node* second_part_start = split_point;
    Node* second_part_end = list->tail->prev;
    
    // Reconnect: dummy_head -> second_part -> first_part -> dummy_tail
    list->head->next = second_part_start;
    second_part_start->prev = list->head;
    
    second_part_end->next = first_part_start;
    first_part_start->prev = second_part_end;
    
    first_part_end->next = list->tail;
    list->tail->prev = first_part_end;
    
    return LIST_SUCCESS;
}

// Legacy Node function for adding values from two lists
Node* add_lists(Node *list1, Node *list2) {
    if (!list1 || !list2) return NULL;
    
    Node *result = NULL;
    Node *tail = NULL;
    int carry = 0;
    
    while (list1 || list2 || carry) {
        int sum = carry;
        if (list1) {
            sum += *(int*)list1->data;
            list1 = list1->next;
        }
        if (list2) {
            sum += *(int*)list2->data;
            list2 = list2->next;
        }
        
        Node *new_node = (Node*)malloc(sizeof(Node));
        if (!new_node) return NULL;
        
        new_node->data = malloc(sizeof(int));
        if (!new_node->data) {
            free(new_node);
            return NULL;
        }
        
        *(int*)new_node->data = sum % 10;
        carry = sum / 10;
        new_node->next = NULL;
        new_node->prev = tail;
        
        if (!result) {
            result = new_node;
        } else {
            tail->next = new_node;
        }
        tail = new_node;
    }
    
    return result;
}

// Legacy Node function for merging two sorted lists
Node* merge_sorted_lists(Node *list1, Node *list2) {
    if (!list1) return list2;
    if (!list2) return list1;
    
    Node *result;
    if (*(int*)list1->data <= *(int*)list2->data) {
        result = list1;
        result->next = merge_sorted_lists(list1->next, list2);
        if (result->next) result->next->prev = result;
    } else {
        result = list2;
        result->next = merge_sorted_lists(list1, list2->next);
        if (result->next) result->next->prev = result;
    }
    
    return result;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃      Array to List Conversion Functions       ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Converts an array to a linked list (using generic LinkedList structure).
 * @param list The LinkedList to populate with array data.
 * @param arr A pointer to the array data.
 * @param n The number of elements in the array.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult array_to_list(LinkedList* list, const void* arr, size_t n) {
    if (!list || !arr || n == 0) return LIST_ERROR_NULL_POINTER;
    
    // Clear existing list content if any
    while (!list_is_empty(list)) {
        ListResult result = list_delete_from_head(list);
        if (result != LIST_SUCCESS) break;
    }
    
    const char* byte_arr = (const char*)arr;
    
    // Insert each element from the array
    for (size_t i = 0; i < n; i++) {
        const void* element = byte_arr + (i * list->element_size);
        ListResult result = list_insert_at_tail(list, (void*)element);
        if (result != LIST_SUCCESS) {
            return result; // Failed to insert
        }
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Converts a linked list to an array.
 * @param list The LinkedList to convert.
 * @param out_size Pointer to store the size of the returned array.
 * @return A pointer to the newly allocated array, or NULL on failure.
 * @note The caller is responsible for freeing the returned array.
 */
void* list_to_array(const LinkedList* list, size_t* out_size) {
    if (!list || !out_size) return NULL;
    
    *out_size = list->length;
    if (list->length == 0) return NULL;
    
    // Allocate memory for the array
    void* array = malloc(list->length * list->element_size);
    if (!array) return NULL;
    
    char* byte_array = (char*)array;
    
    // Determine starting point (always dummy nodes)
    Node* current = list->head->next;
    Node* end_sentinel = list->tail;
    
    // Copy each element to the array
    size_t index = 0;
    while (current != end_sentinel && index < list->length) {
        void* dest = byte_array + (index * list->element_size);
        
        // Use custom copy function if available, otherwise memcpy
        if (list->copy_node_function) {
            list->copy_node_function(dest, current->data);
        } else {
            memcpy(dest, current->data, list->element_size);
        }
        
        current = current->next;
        index++;
    }
    
    return array;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃     Helper Functions for Basic Data Types     ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// Integer helpers
/**
 * @brief Print function for integer elements.
 * @param data Pointer to integer data to print.
 */
void print_int(void* data) {
    printf("%d", *(int*)data);
}

/**
 * @brief Compare function for integer elements.
 * @param a Pointer to first integer.
 * @param b Pointer to second integer.
 * @return Comparison result (-1, 0, 1).
 */
int compare_int(const void* a, const void* b) {
    int val_a = *(const int*)a;
    int val_b = *(const int*)b;
    return (val_a > val_b) - (val_a < val_b);
}

/**
 * @brief Creates a new integer list (value-based for basic types).
 * @return A new LinkedList configured for integers, or NULL on failure.
 */
LinkedList* list_create_int(void) {
    LinkedList* list = list_create(sizeof(int));
    if (!list) return NULL;
    
    list_set_print_function(list, print_int);
    list_set_compare_function(list, compare_int);
    
    return list;
}

// Double helpers  
/**
 * @brief Print function for double elements.
 * @param data Pointer to double data to print.
 */
void print_double(void* data) {
    printf("%.2f", *(double*)data);
}

/**
 * @brief Compare function for double elements.
 * @param a Pointer to first double.
 * @param b Pointer to second double.
 * @return Comparison result (-1, 0, 1).
 */
int compare_double(const void* a, const void* b) {
    double val_a = *(const double*)a;
    double val_b = *(const double*)b;
    return (val_a > val_b) - (val_a < val_b);
}

/**
 * @brief Creates a new double list (value-based for basic types).
 * @return A new LinkedList configured for doubles, or NULL on failure.
 */
LinkedList* list_create_double(void) {
    LinkedList* list = list_create(sizeof(double));
    if (!list) return NULL;
    
    list_set_print_function(list, print_double);
    list_set_compare_function(list, compare_double);
    
    return list;
}

// Character helpers
/**
 * @brief Print function for character elements.
 * @param data Pointer to character data to print.
 */
void print_char(void* data) {
    printf("%c", *(char*)data);
}

/**
 * @brief Compare function for character elements.
 * @param a Pointer to first character.
 * @param b Pointer to second character.
 * @return Comparison result (-1, 0, 1).
 */
int compare_char(const void* a, const void* b) {
    char val_a = *(const char*)a;
    char val_b = *(const char*)b;
    return (val_a > val_b) - (val_a < val_b);
}

/**
 * @brief Creates a new character list (value-based for basic types).
 * @return A new LinkedList configured for characters, or NULL on failure.
 */
LinkedList* list_create_char(void) {
    LinkedList* list = list_create(sizeof(char));
    if (!list) return NULL;
    
    list_set_print_function(list, print_char);
    list_set_compare_function(list, compare_char);
    
    return list;
}

// String helpers (char*)
/**
 * @brief Print function for string elements (internal).
 * @param data Pointer to string data to print.
 */
void print_string_builtin(void* data) {
    printf("%s", *(char**)data);
}

/**
 * @brief Compare function for string elements (internal).
 * @param a Pointer to first string.
 * @param b Pointer to second string.
 * @return Comparison result from strcmp.
 */
int compare_string_builtin(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

/**
 * @brief Free function for string elements (internal).
 * @param data Pointer to string data to free.
 */
void free_string_builtin(void* data) {
    free(*(char**)data);
}

/**
 * @brief Copy function for string elements (internal).
 * @param dest Destination for copied string.
 * @param src Source string to copy.
 */
void copy_string_builtin(void* dest, const void* src) {
    *(char**)dest = strdup(*(const char**)src);
}

/**
 * @brief Creates a new string list (value-based with special string handling).
 * @return A new LinkedList configured for strings, or NULL on failure.
 */
LinkedList* list_create_string(void) {
    LinkedList* list = list_create(sizeof(char*));
    if (!list) return NULL;
    
    list_set_print_function(list, print_string_builtin);
    list_set_compare_function(list, compare_string_builtin);
    list_set_free_function(list, free_string_builtin);
    list_set_copy_function(list, copy_string_builtin);
    
    return list;
}