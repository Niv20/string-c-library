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
static ListResult delete_node_core(LinkedList*, Node*);          // Core deletion helper
static void copy_list_configuration(LinkedList*, const LinkedList*); // Helper to copy function pointers

// Forward declarations for functions used in handle_size_limit
ListResult delete_head(LinkedList* list);

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              0. Error Handling                ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Converts a ListResult error code to a human-readable string.
 * @param result The error code to convert.
 * @return A string describing the error.
 */
const char* error_string(ListResult result) {
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
        default: return "Unknown error";
    }
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃                1. Create List                 ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Creates and initializes a new linked list. The data is pointer to structures.
 * @param element_size The size of each element in bytes.
 * @return A pointer to the newly created LinkedList, or NULL on failure.
 */
LinkedList* create_list(size_t element_size) {

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
    list->max_size = UNLIMITED;                     // Unlimited by default
    list->allow_overwrite = REJECT_NEW_WHEN_FULL;   // Reject new when full by default
    
    // Initialize struct name
    list->struct_name = NULL;
    
    list->print_node_function = NULL;      
    // Removed compare_node_function field usage.
    list->free_node_function = NULL;       
    list->copy_node_function = NULL;       

    return list;
}

/**
 * @brief Sets the struct name for the list.
 * @param list The list to configure.
 * @param struct_name The name of the struct type.
 */
void set_list_struct_name(LinkedList* list, const char* struct_name) {
    if (!list) return;
    
    // Free old struct name if exists
    if (list->struct_name) {
        free(list->struct_name);
        list->struct_name = NULL;
    }
    
    // Set new struct name
    if (struct_name) {
        list->struct_name = malloc(strlen(struct_name) + 1);
        if (list->struct_name) {
            strcpy(list->struct_name, struct_name);
        }
    }
}
    // EOF




/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃            2. List Configuration              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Sets the print function for the list.
 * @param list The list to configure.
 * @param print_fn Function pointer for printing elements.
 */
void set_print_function(LinkedList* list, PrintFunction print_fn) {
    if (list)
        list->print_node_function = print_fn;
}

/**
 * @brief Sets the compare function for the list.
 * @param list The list to configure.
 * @param compare_fn Function pointer for comparing elements.
 */
// set_compare_function removed – comparator now passed per-call.

/**
 * @brief Sets the free function for the list.
 * @param list The list to configure.
 * @param free_fn Function pointer for freeing complex elements.
 */
void set_free_function(LinkedList* list, FreeFunction free_fn) {
    if (list)
        list->free_node_function = free_fn;
}

/**
 * @brief Sets the copy function for deep copying complex data types.
 * @param list The list to configure.
 * @param copy_fn Function pointer for deep copying complex elements.
 */
void set_copy_function(LinkedList* list, CopyFunction copy_fn) {
    if (list)
        list->copy_node_function = copy_fn;
}

/**
 * @brief Sets the maximum size of the list and overflow behavior.
 * @param list The list to configure.
 * @param max_size Maximum number of elements (UNLIMITED = no limit).
 */
ListResult set_max_size(LinkedList* list, size_t max_size, OverflowBehavior behavior) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;

    // Update the list's configuration first
    list->max_size = max_size;
    list->allow_overwrite = behavior;

    // Check if we need to remove excess elements due to new size limit
    return handle_size_limit(list);
}

// INTERNAL HELPER FUNCTION for handling size limits
static ListResult handle_size_limit(LinkedList* list) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    
    // If unlimited size or current length is within limits - no action needed
    if (list->max_size == UNLIMITED || list->length < list->max_size) {
        return LIST_SUCCESS;
    }

    // If we reach this point, the list is full, but overwrite is not allowed
    if (list->allow_overwrite == REJECT_NEW_WHEN_FULL) {
        return LIST_ERROR_LIST_FULL;
    }

    // Handle any case where we need to remove elements to stay within limit
    while (list->length >= list->max_size) {
        // Remove oldest element (from head) to make room for FIFO behavior
        ListResult result = delete_head(list);
        if (result != LIST_SUCCESS) {
            return result;
        }
    }
    
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃         3. Insertion in Linked List           ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL HELPER FUNCTION for creating a new node (unified for both modes)
static Node* create_node_generic(LinkedList* list, void* data, ListMemoryMode mode) {
    
    // Create a new node
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return NULL;

    if (mode == LIST_MODE_VALUE) {
        // Value mode: Allocate memory and copy data
        new_node->data = malloc(list->element_size);
        if (!new_node->data) {
            free(new_node);
            return NULL;
        }
        
        // Copy the data
        memcpy(new_node->data, data, list->element_size);

    } else if (mode == LIST_MODE_POINTER) {
        // Pointer mode: store pointer directly (ownership of the pointed block transfers to list)
        new_node->data = data;
    }
    
    // Initialize pointers & mode
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->mode = mode;
    
    return new_node;
}

// INTERNAL CORE HELPER FUNCTION for insertion logic (unified for both modes)
static ListResult insert_node_core_generic(LinkedList* list, void* data, ListMemoryMode mode, Node** out_new_node) {
    
    // Input validation
    if (!list || !data) return LIST_ERROR_NULL_POINTER;

    // Create new node using generic function
    Node* new_node = create_node_generic(list, data, mode);
    if (!new_node) return LIST_ERROR_MEMORY_ALLOC;
    
    *out_new_node = new_node;
    return LIST_SUCCESS;
}

/**
 * @brief Inserts a new element at the head of the list (value mode - copies data).
 * @param list The list to insert into.
 * @param data A pointer to the data to be copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult insert_head_value_internal(LinkedList* list, void* data) {
    
    Node* new_node;
    ListResult result = insert_node_core_generic(list, data, LIST_MODE_VALUE, &new_node);
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
 * @brief Inserts a new element at the tail of the list (value mode - copies data).
 * @param list The list to insert into.
 * @param data A pointer to the data to be copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult insert_tail_value_internal(LinkedList* list, void* data) {
    
    Node* new_node;
    ListResult result = insert_node_core_generic(list, data, LIST_MODE_VALUE, &new_node);
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
 * @brief Inserts an element at a specific index (value mode - copies data).
 * @param list The list to insert into.
 * @param index The index to insert at (0-based).
 * @param data The data to copy into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult insert_index_value_internal(LinkedList* list, size_t index, void* data) {

    // Handle boundary conditions
    if (index == 0) {
        // Insert at head for index 0
        return insert_head_value_internal(list, data);
    }
    if (index >= list->length) {
        // Insert at tail for indices beyond list length
        return insert_tail_value_internal(list, data);
    }

    Node* new_node;
    ListResult result = insert_node_core_generic(list, data, LIST_MODE_VALUE, &new_node);
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

/**
 * @brief Inserts a new element at the head of the list (pointer mode - copies pointed data).
 * @param list The list to insert into.
 * @param data_ptr A pointer to data to be copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult insert_head_ptr(LinkedList* list, void* data_ptr) {
    
    Node* new_node;
    ListResult result = insert_node_core_generic(list, data_ptr, LIST_MODE_POINTER, &new_node);
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
 * @brief Inserts a new element at the tail of the list (pointer mode - stores pointer directly).
 * @param list The list to insert into.
 * @param data_ptr A pointer to existing data. The pointer itself is stored, not copied.
 * @return LIST_SUCCESS on success, error code on failure.
 * @warning The caller must ensure the pointed-to data remains valid for the list's lifetime.
 */
ListResult insert_tail_ptr(LinkedList* list, void* data_ptr) {
    
    Node* new_node;
    ListResult result = insert_node_core_generic(list, data_ptr, LIST_MODE_POINTER, &new_node);
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
 * @brief Inserts an element at a specific index (pointer mode - stores pointer directly).
 * @param list The list to insert into.
 * @param index The index to insert at (0-based).
 * @param data_ptr A pointer to existing data. The pointer itself is stored, not copied.
 * @return LIST_SUCCESS on success, error code on failure.
 * @warning The caller must ensure the pointed-to data remains valid for the list's lifetime.
 */
ListResult insert_index_ptr(LinkedList* list, size_t index, void* data_ptr) {
    
    // Handle boundary conditions
    if (index <= 0) {
        // Insert at head for index 0
        return insert_head_ptr(list, data_ptr);
    }
    if (index >= list->length) {
        // Insert at tail for indices beyond list length
        return insert_tail_ptr(list, data_ptr);
    }
    
    Node* new_node;
    ListResult result = insert_node_core_generic(list, data_ptr, LIST_MODE_POINTER, &new_node);
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
┃            4. Deletion Functions              ┃
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
    
    // Free the data based on ownership mode
    if (node_to_delete->mode == LIST_MODE_VALUE) {
        // We allocated the block; free inner fields via callback then free block
        if (list->free_node_function) {
            list->free_node_function(node_to_delete->data);
        }
        free(node_to_delete->data);
    } else { // LIST_MODE_POINTER
        // We only stored user pointer; we now own it so apply free function first then free struct
        if (list->free_node_function) {
            list->free_node_function(node_to_delete->data);
        }
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
ListResult delete_head(LinkedList* list) {

    if (!list) return LIST_ERROR_NULL_POINTER;
    if (is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Use core deletion logic
    return delete_node_core(list, list->head->next);
}


/**
 * @brief Deletes the element at the tail of the list.
 * @param list The list to delete from.
 * @return LIST_SUCCESS on success, error code if the list is empty.
 */
ListResult delete_tail(LinkedList* list) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Use core deletion logic
    return delete_node_core(list, list->tail->prev);
}

/**
 * @brief Deletes an element at a specific index.
 * @param list The list to delete from.
 * @param index The index to delete at (0-based).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult delete_index(LinkedList* list, size_t index) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS; // size_t cannot be negative
    
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
 * @param order START_FROM_HEAD or START_FROM_TAIL.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult remove_advanced(LinkedList* list, int count, Direction order, FilterFunction predicate) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (!predicate) return LIST_ERROR_INVALID_OPERATION; // Or define a new error if desired.
    if (is_empty(list)) return LIST_ERROR_ELEMENT_NOT_FOUND;

    int removed_count = 0;
    Node* current;
    Node* end_sentinel;

    // Set up traversal direction
    if (order == START_FROM_TAIL) {
        current = list->tail->prev;
        end_sentinel = list->head;
    } else {
        current = list->head->next;
        end_sentinel = list->tail;
    }

    // Traverse and remove matching elements
    while (current != end_sentinel && (count == DELETE_ALL_OCCURRENCES || removed_count < count)) {
        Node* next_node = (order == START_FROM_TAIL) ? current->prev : current->next;

        if (predicate(current->data)) {
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
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult clear(LinkedList* list) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    
    while (!is_empty(list)) {
        ListResult result = delete_head(list);
        if (result != LIST_SUCCESS) {
            return result;
        }
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Frees all memory associated with the list.
 * This includes all nodes and the data within them (using the free_function if provided).
 * @param list A pointer to the LinkedList to be destroyed.
 */
void destroy(LinkedList* list) {
    
    if (!list) return;

    // Clear all real nodes - ignore errors during destruction
    clear(list);

    // Free the dummy nodes
    free(list->head);
    free(list->tail);
    
    // Free struct name if allocated
    if (list->struct_name) {
        free(list->struct_name);
    }
    
    // Finally, free the list manager itself
    free(list);
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃             5. Utility Functions              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Returns the number of elements in the list.
 * @param list The list to query.
 * @return The number of elements.
 */
size_t get_length(const LinkedList* list) {
    return list ? list->length : 0;
}

/**
 * @brief Checks if the list is empty.
 * @param list The list to check.
 * @return TRUE if the list is empty, FALSE otherwise.
 */
bool is_empty(const LinkedList* list) {
    return !list || list->length == 0;
}

/**
 * @brief Prints all elements in the list using default formatting (with indices and newlines).
 * @param list The list to print.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult print_list(const LinkedList* list) {
    return print_list_advanced(list, false, true, "\n");
}


/**
 * @brief Prints all elements in the list with customizable formatting.
 * @param list The list to print.
 * @param show_size Whether to show the total size of the list.
 * @param show_index Whether to show element indices.
 * @param separator String to print between elements.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult print_list_advanced(const LinkedList* list, bool show_size, bool show_index, const char* separator) {

    if (!list) return LIST_ERROR_NULL_POINTER;
    if (is_empty(list)) return LIST_ERROR_ELEMENT_NOT_FOUND;
    if (!list->print_node_function) return LIST_ERROR_NO_PRINT_FUNCTION;
    
    // Determine starting and ending points (always dummy nodes)
    Node* current_node = list->head->next;
    Node* end_node = list->tail;

    if (show_size)
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

    // Final newline for clean output
    printf("\n");
    
    return LIST_SUCCESS;
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃        6. Search and Access Functions         ┃
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
void* get(const LinkedList* list, size_t index) {
    
    if (!list || index >= list->length) return NULL;
    
    Node* current = find_node_by_index(list, index);
    return current ? current->data : NULL;
}

/**
 * @brief Returns the index of the first element that satisfies the predicate.
 * @param list The list to search in.
 * @param predicate The function to test each element.
 * @param arg An optional argument to pass to the predicate.
 * @return The index if found, or a negative error code.
 */
int index_of(const LinkedList* list, PredicateFunction predicate) {
    return index_of_advanced(list, START_FROM_HEAD, predicate);
}

/**
 * @brief Returns the index of the first or last element that satisfies the predicate.
 * @param list The list to search in.
 * @param direction START_FROM_HEAD or START_FROM_TAIL.
 * @param predicate The function to test each element.
 * @return The index if found, or a negative error code.
 */
int index_of_advanced(const LinkedList* list, Direction order, PredicateFunction predicate) {
    if (!list) return -LIST_ERROR_NULL_POINTER;
    if (!predicate) return -LIST_ERROR_INVALID_OPERATION;

    if (order == START_FROM_TAIL) {
        // Search from tail to head
        Node* current = list->tail->prev;
        size_t index = list->length - 1;
        while (current != list->head) {
            if (predicate(current->data)) {
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
            if (predicate(current->data)) {
                return (int)index;
            }
            current = current->next;
            index++;
        }
    }
    return -LIST_ERROR_ELEMENT_NOT_FOUND; // Element not found
}

/**
 * @brief Counts how many elements in the list satisfy a given condition.
 * @param list The list to iterate over.
 * @param predicate A function pointer that returns true if an element satisfies the condition.
 * @param arg An optional argument to pass to the predicate function.
 * @return The number of elements that satisfy the condition.
 */
size_t count_matching(const LinkedList* list, PredicateFunction predicate) {
    if (!list || !predicate) return 0;
    
    size_t count = 0;
    Node* current = list->head->next;
    while (current != list->tail) {
        if (predicate(current->data)) {
            count++;
        }
        current = current->next;
    }
    return count;
}


/**
 * @brief Sets a field value by copying data directly (no memory management).
 * @param list The list containing the element.
 * @param index The index of the element to modify.
 * @param field_offset The byte offset of the field within the struct.
 * @param field_size The size of the field in bytes.
 * @param new_value Pointer to the new value to set.
 * @return LIST_SUCCESS on success, error code on failure.
 * @note Use this for primitive types and when you manage memory yourself.
 */
ListResult set_field_impl(LinkedList* list, size_t index, size_t field_offset, size_t field_size, const void* new_value) {
    return set_field_advanced_impl(list, index, field_offset, field_size, new_value, false, false, 0);
}

/**
 * @brief Sets a dynamically allocated field with proper memory management.
 * @param list The list containing the element.
 * @param index The index of the element to modify.
 * @param field_offset The byte offset of the pointer field within the struct.
 * @param data_size The size of the data to allocate and copy.
 * @param new_data Pointer to the new data to copy.
 * @return LIST_SUCCESS on success, error code on failure.
 * @note This frees old memory, allocates new memory, and copies the data.
 */
ListResult set_allocated_field_impl(LinkedList* list, size_t index, size_t field_offset, size_t data_size, const void* new_data) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    if (field_offset + sizeof(void*) > list->element_size) return LIST_ERROR_INVALID_OPERATION;

    Node* current = find_node_by_index(list, index);
    if (!current) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    // Calculate the address of the pointer field
    void** ptr_field = (void**)((char*)current->data + field_offset);
    
    // Free old memory if it exists
    if (*ptr_field) {
        free(*ptr_field);
    }
    
    // Allocate and copy new data
    if (new_data && data_size > 0) {
        *ptr_field = malloc(data_size);
        if (*ptr_field) {
            memcpy(*ptr_field, new_data, data_size);
        } else {
            return LIST_ERROR_MEMORY_ALLOC;
        }
    } else {
        *ptr_field = NULL;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Sets a field with advanced memory management control.
 * @param list The list containing the element.
 * @param index The index of the element to modify.
 * @param field_offset The byte offset of the field within the struct.
 * @param field_size The size of the field in bytes.
 * @param new_value Pointer to the new value or data.
 * @param should_free_old Whether to free existing memory (for pointer fields).
 * @param should_alloc_new Whether to allocate new memory and copy data.
 * @param data_size Size of data to allocate (when should_alloc_new is true).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult set_field_advanced_impl(LinkedList* list, size_t index, size_t field_offset, size_t field_size, 
                                   const void* new_value, bool should_free_old, bool should_alloc_new, size_t data_size) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    if (field_offset + field_size > list->element_size) return LIST_ERROR_INVALID_OPERATION;

    Node* current = find_node_by_index(list, index);
    if (!current) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    // Handle different memory management modes
    if (field_size == sizeof(void*) && (should_free_old || should_alloc_new)) {
        // This is likely a pointer field with memory management
        void** ptr_field = (void**)((char*)current->data + field_offset);
        
        // Free old memory if requested
        if (should_free_old && *ptr_field) {
            free(*ptr_field);
            *ptr_field = NULL;
        }
        
        // Set new value based on allocation mode
        if (should_alloc_new && new_value && data_size > 0) {
            // Allocate new memory and copy data
            *ptr_field = malloc(data_size);
            if (!*ptr_field) return LIST_ERROR_MEMORY_ALLOC;
            memcpy(*ptr_field, new_value, data_size);
        } else if (new_value) {
            // Just store the provided pointer (could be NULL)
            // Cast away const since we're storing it as void*
            *ptr_field = (void*)(uintptr_t)new_value;
        } else {
            // Set to NULL
            *ptr_field = NULL;
        }
    } else {
        // Simple field assignment using memcpy (equivalent to set_field_impl)
        if (new_value) {
            char* field_ptr = (char*)current->data + field_offset;
            memcpy(field_ptr, new_value, field_size);
        }
    }
    
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃             7. Sorting Functions              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Sorts the list in place (ascending order).
 * @param list The list to sort.
 * @param compare_fn Comparison function.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult sort_list(LinkedList* list, CompareFunction compare_fn) {

    if (!list) return LIST_ERROR_NULL_POINTER;
    if (!compare_fn) return LIST_ERROR_NO_COMPARE_FUNCTION;
    if (list->length <= 1) return LIST_SUCCESS;
    
    bool swapped;
    do {
        swapped = false;
        Node* current = list->head->next;
        
        while (current->next != list->tail) {
            Node* next_node = current->next;
            
            int comparison = compare_fn(current->data, next_node->data);
            if (comparison > 0) {
                // Swap the data pointers (not the nodes themselves)
                void* temp_data = current->data;
                current->data = next_node->data;
                next_node->data = temp_data;
                swapped = true;
            }
            
            current = next_node;
        }
    } while (swapped);
    
    return LIST_SUCCESS;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃         8. Structural Transformations         ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL HELPER to copy all function pointers from a source list to a destination list.
static void copy_list_configuration(LinkedList* dest, const LinkedList* src) {
    
    if (!dest || !src) return;
    
    dest->print_node_function = src->print_node_function;
    dest->free_node_function = src->free_node_function;
    dest->copy_node_function = src->copy_node_function;
    
    // Copy struct name
    if (src->struct_name) {
        dest->struct_name = malloc(strlen(src->struct_name) + 1);
        if (dest->struct_name) {
            strcpy(dest->struct_name, src->struct_name);
        }
    }
}

/**
 * @brief Creates a copy of the list.
 * @param list The list to copy.
 * @return A new list that is a copy of the original, or NULL on failure.
 */
LinkedList* copy(const LinkedList* list) {

    if (!list) return NULL;
    
    LinkedList* new_list = create_list(list->element_size);
    if (!new_list) return NULL;
    
    // Configure the new list with same settings as the original
    copy_list_configuration(new_list, list);
    
    // Copy all elements from original list
    ListResult extend_result = extend(new_list, list);
    if (extend_result != LIST_SUCCESS) {
        destroy(new_list);
        return NULL;
    }
    
    return new_list;
}

/**
 * @brief Extends the list with elements from another list.
 * @param list The list to extend.
 * @param other The other list to extend with.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult extend(LinkedList* list, const LinkedList* other) {
    
    if (!list || !other) return LIST_ERROR_NULL_POINTER;
    
    Node* current = other->head->next;
    while (current != other->tail) {
    // current->data already points to an element of size element_size,
    // so we must call the internal function directly to avoid macro creating a void* temp (size mismatch)
    ListResult result = insert_tail_value_internal(list, current->data);
        if (result != LIST_SUCCESS) {
            return result;
        }
        current = current->next;
    }
    
    return LIST_SUCCESS;
}


/**
 * @brief Creates a new list by concatenating two lists.
 * @param list1 First list.
 * @param list2 Second list.
 * @return A new concatenated list, or NULL on failure.
 */
LinkedList* concat(const LinkedList* list1, const LinkedList* list2) {
    if (!list1 || !list2) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    LinkedList* concatenated = create_list(list1->element_size);
    if (!concatenated) return NULL;
    
    // Configure the concatenated list with settings from first list
    copy_list_configuration(concatenated, list1);
    
    // Copy all elements from first list
    if (extend(concatenated, list1) != LIST_SUCCESS) {
        destroy(concatenated);
        return NULL;
    }
    
    // Copy all elements from second list
    if (extend(concatenated, list2) != LIST_SUCCESS) {
        destroy(concatenated);
        return NULL;
    }
    
    return concatenated;
}


/**
 * @brief Creates a new list containing a slice of the original.
 * @param list The source list.
 * @param start Start index (inclusive).
 * @param end End index (exclusive).
 * @return A new sliced list, or NULL on failure.
 */
LinkedList* slice(const LinkedList* list, size_t start, size_t end) {
    
    if (!list || start >= end || start >= list->length) return NULL;
    
    if (end > list->length) end = list->length;
    
    LinkedList* sliced = create_list(list->element_size);
    if (!sliced) return NULL;
    
    // Configure the sliced list with same settings as the original
    copy_list_configuration(sliced, list);
    
    Node* current = list->head->next;
    
    // Skip to start position
    for (size_t i = 0; i < start && current != list->tail; i++) {
        current = current->next;
    }
    
    // Copy elements from start to end
    for (size_t i = start; i < end && current != list->tail; i++) {
    if (insert_tail_value_internal(sliced, current->data) != LIST_SUCCESS) {
            destroy(sliced);
            return NULL;
        }
        current = current->next;
    }
    
    return sliced;
}

/**
 * @brief Rotates the list elements by a specified number of positions.
 * @param list The list to rotate.
 * @param positions Number of positions to rotate (positive = right, negative = left).
 * @return LIST_SUCCESS on success, error code otherwise.
 */
ListResult rotate(LinkedList* list, int positions) {
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

/**
 * @brief Reverses the list in place.
 * @param list The list to reverse.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult reverse(LinkedList* list) {
    
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list->length <= 1) return LIST_SUCCESS;
    
    Node* current = list->head->next;
    Node* prev_node = list->head;
    
    // Swap next and prev pointers for each node
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
    
    return LIST_SUCCESS;
}

/**
 * @brief Creates a new list with elements that pass the filter function.
 * @param list The source list.
 * @param filter_fn Function to test each element.
 * @return A new filtered list, or NULL on failure.
 */
LinkedList* filter(const LinkedList* list, FilterFunction filter_fn) {
    
    if (!list || !filter_fn) return NULL;
    
    LinkedList* filtered = create_list(list->element_size);
    if (!filtered) return NULL;
    
    // Configure the filtered list with same settings as the original
    copy_list_configuration(filtered, list);
    
    Node* current = list->head->next;
    while (current != list->tail) {
        if (filter_fn(current->data)) {
            if (insert_tail_value_internal(filtered, current->data) != LIST_SUCCESS) {
                destroy(filtered);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return filtered;
}

/**
 * @brief Creates a new list with transformed elements.
 * @param list The source list.
 * @param map_fn Function to transform each element.
 * @param new_element_size Size of elements in the new list.
 * @return A new transformed list, or NULL on failure.
 */
LinkedList* map(const LinkedList* list, MapFunction map_fn, size_t new_element_size) {
    if (!list || !map_fn) return NULL;
    
    LinkedList* mapped = create_list(new_element_size);
    if (!mapped) return NULL;
    
    // Don't copy the original list's free/copy functions since the new list 
    // may contain a different data type that requires different handling
    
    Node* current = list->head->next;
    while (current != list->tail) {
        void* transformed = malloc(new_element_size);
        if (!transformed) {
            destroy(mapped);
            return NULL;
        }
        
        map_fn(transformed, current->data);
        
    if (insert_tail_value_internal(mapped, transformed) != LIST_SUCCESS) {
            free(transformed);
            destroy(mapped);
            return NULL;
        }
        
        free(transformed);
        current = current->next;
    }
    
    return mapped;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃           9. Mathematical Functions           ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Finds the minimum element in the list based on a custom comparison function.
 * @param list The list to search in.
 * @param compare A function pointer to compare two elements. Should return < 0 if a < b, 0 if a == b, > 0 if a > b.
 * @return A direct pointer to the minimum element's data, or NULL if the list is empty or compare function is not provided.
 */
void* min_by(const LinkedList* list, int (*compare)(const void *a, const void *b)) {
    if (is_empty(list) || !compare) return NULL;

    void* min_elem = list->head->next->data;
    Node* current = list->head->next->next;
    
    while (current != list->tail) {
        if (compare(current->data, min_elem) < 0) {
            min_elem = current->data;
        }
        current = current->next;
    }
    return min_elem;
}

/**
 * @brief Finds the maximum element in the list based on a custom comparison function.
 * @param list The list to search in.
 * @param compare A function pointer to compare two elements. Should return < 0 if a < b, 0 if a == b, > 0 if a > b.
 * @return A direct pointer to the maximum element's data, or NULL if the list is empty or compare function is not provided.
 */
void* max_by(const LinkedList* list, int (*compare)(const void *a, const void *b)) {
    if (is_empty(list) || !compare) return NULL;

    void* max_elem = list->head->next->data;
    Node* current = list->head->next->next;

    while (current != list->tail) {
        if (compare(current->data, max_elem) > 0) {
            max_elem = current->data;
        }
        current = current->next;
    }
    return max_elem;
}

/**
 * @brief Creates a new list with unique elements based on the default compare function, preserving the first occurrence.
 * @param list The source list.
 * @return A new list with unique elements, or NULL on failure.
 */
LinkedList* unique(const LinkedList* list, CompareFunction compare_fn) { 
    return unique_advanced(list, compare_fn, START_FROM_HEAD); 
}

/**
 * @brief Creates a new list with unique elements based on a custom comparison function and preservation order.
 * @param list The source list.
 * @param custom_compare A function to determine uniqueness. If NULL, the list's default compare function is used.
 * @param order START_FROM_HEAD to keep the first seen unique element, START_FROM_TAIL to keep the last.
 * @return A new list with unique elements, or NULL on failure.
 */
LinkedList* unique_advanced(const LinkedList* list, CompareFunction compare_fn, Direction order) {
    if (!list) return NULL;
    if (!compare_fn) return NULL;

    LinkedList* unique_list = create_list(list->element_size);
    if (!unique_list) return NULL;

    copy_list_configuration(unique_list, list);

    if (order == START_FROM_TAIL) {
        // To preserve the last occurrence, we iterate the source list in reverse.
        Node* current = list->tail->prev;
        while (current != list->head) {
            // Check if the element is already in our unique list.
            bool found = false;
            Node* unique_current = unique_list->head->next;
            while (unique_current != unique_list->tail) {
                if (compare_fn(current->data, unique_current->data) == 0) {
                    found = true;
                    break;
                }
                unique_current = unique_current->next;
            }

            if (!found) {
                // Since we are iterating backwards, we insert at the head to maintain the original relative order.
                if (insert_head_value_internal(unique_list, current->data) != LIST_SUCCESS) {
                    destroy(unique_list);
                    return NULL;
                }
            }
            current = current->prev;
        }
    } else { // START_FROM_HEAD
        // To preserve the first occurrence, we iterate the source list forwards.
        Node* current = list->head->next;
        while (current != list->tail) {
            // Check if the element is already in our unique list.
            bool found = false;
            Node* unique_current = unique_list->head->next;
            while (unique_current != unique_list->tail) {
                if (compare_fn(current->data, unique_current->data) == 0) {
                    found = true;
                    break;
                }
                unique_current = unique_current->next;
            }

            if (!found) {
                if (insert_tail_value_internal(unique_list, current->data) != LIST_SUCCESS) {
                    destroy(unique_list);
                    return NULL;
                }
            }
            current = current->next;
        }
    }

    return unique_list;
}

/**
 * @brief Helper function to find index of element using compare function.
 * @param list The list to search in.
 * @param data The data to search for.
 * @param compare_fn Comparison function.
 * @return Index if found, -1 if not found.
 */
static int index_of_with_compare(const LinkedList* list, const void* data, CompareFunction compare_fn) {
    if (!list || !data || !compare_fn) return -1;
    
    Node* current = list->head->next;
    int index = 0;
    while (current != list->tail) {
        if (compare_fn(current->data, data) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

/**
 * @brief Creates a new list containing the intersection of two lists.
 * @param list1 First list.
 * @param list2 Second list.
 * @return A new list with common elements, or NULL on failure.
 */
LinkedList* intersection(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn) {
    if (!list1 || !list2 || !compare_fn) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    LinkedList* intersection = create_list(list1->element_size);
    if (!intersection) return NULL;
    
    // Configure the intersection list with settings from first list
    copy_list_configuration(intersection, list1);
    
    Node* current = list1->head->next;
    while (current != list1->tail) {

        // If element exists in both lists and not already in result
        if (index_of_with_compare(list2, current->data, compare_fn) != -1 && 
            index_of_with_compare(intersection, current->data, compare_fn) == -1) {
            if (insert_tail_value_internal(intersection, current->data) != LIST_SUCCESS) {
                destroy(intersection);
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
LinkedList* union_lists(const LinkedList* list1, const LinkedList* list2, CompareFunction compare_fn) {
    if (!list1 || !list2 || !compare_fn) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    // Start with unique elements from first list
    LinkedList* union_list = unique(list1, compare_fn);
    if (!union_list) return NULL;
    
    // Add unique elements from second list
    Node* current = list2->head->next;
    while (current != list2->tail) {
    if (index_of_with_compare(union_list, current->data, compare_fn) == -1) {
            if (insert_tail_value_internal(union_list, current->data) != LIST_SUCCESS) {
                destroy(union_list);
                return NULL;
            }
        }
        current = current->next;
    }
    
    return union_list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              10. List <--> Array              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Converts an array to a linked list.
 * @param list The list to populate with array elements.
 * @param arr Pointer to the array data.
 * @param n Number of elements in the array.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult from_array(LinkedList* list, const void* arr, size_t n) {
    if (!list || !arr) return LIST_ERROR_NULL_POINTER;
    
    // Clear the list first
    ListResult clear_result = clear(list);
    if (clear_result != LIST_SUCCESS) return clear_result;
    
    // Add each element from the array
    const char* byte_arr = (const char*)arr;
    for (size_t i = 0; i < n; i++) {
        const void* element = byte_arr + (i * list->element_size);
    ListResult result = insert_tail_value_internal(list, (void*)element);
        if (result != LIST_SUCCESS) return result;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Converts a linked list to an array.
 * @param list The list to convert.
 * @param out_size Pointer to store the number of elements in the array.
 * @return Pointer to the newly allocated array, or NULL on failure.
 * @note The caller is responsible for freeing the returned array.
 */
void* to_array(const LinkedList* list, size_t* out_size) {
    if (!list || !out_size) return NULL;
    
    *out_size = list->length;
    if (list->length == 0) return NULL;
    
    // Allocate memory for the array
    void* array = malloc(list->length * list->element_size);
    if (!array) return NULL;
    
    // Copy elements from list to array
    char* byte_array = (char*)array;
    Node* current = list->head->next;
    size_t index = 0;
    
    while (current != list->tail) {
        void* dest = byte_array + (index * list->element_size);
        memcpy(dest, current->data, list->element_size);
        current = current->next;
        index++;
    }
    
    return array;
}



/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃          11. List <--> String (file)          ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * Converts the list into a single C string (null-terminated).
 * Simple, human-readable representation intended for logging / quick export.
 * IMPORTANT LIMITATIONS:
 *   1. Only primitive element sizes (int / double / char) are rendered with real values.
 *      Any other element size becomes the literal token "[data]" (no round‑trip guaranteed).
 *   2. This is NOT meant for lossless serialization. Use save_to_file() with FILE_FORMAT_* for that.
 *   3. We allocate an estimated buffer (length * 50 chars per element) which is usually
 *      enough for typical numeric sizes. If you store very large textual numbers you may
 *      want to re‑implement with dynamic growth (realloc).
 * Memory ownership: caller must free the returned pointer.
 */
char* to_string(const LinkedList* list, const char* separator) {
    
    if (!list || !separator) return NULL;
    if (!list->print_node_function) return NULL;

    // Edge case: empty list
    if (is_empty(list)) {
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
    bool first = true;
    
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
        
        first = false;
        current = current->next;
    }
    
    return result;
}

// save_to_file: unified binary/text persistence.
//   format == FILE_FORMAT_BINARY -> layout: [size_t length][size_t element_size][raw bytes...]
//   format == FILE_FORMAT_TEXT   -> primitives printed plainly, others hex (whitespace mode) or skipped (custom separator mode).
//   separator: for TEXT mode only; placed between tokens (default "\n"). Trailing newline ensured if not present.
ListResult save_to_file(const LinkedList* list, const char* filename, FileFormat format, const char* separator) {
    if (!list || !filename) return LIST_ERROR_NULL_POINTER;
    if (format == FILE_FORMAT_BINARY) {
        FILE* file = fopen(filename, "wb");
        if (!file) return LIST_ERROR_INVALID_OPERATION;
        fwrite(&list->length, sizeof(size_t), 1, file);
        fwrite(&list->element_size, sizeof(size_t), 1, file);
        Node* cur = list->head->next;
        while (cur != list->tail) { fwrite(cur->data, list->element_size, 1, file); cur = cur->next; }
        fclose(file);
        return LIST_SUCCESS;
    }

    // TEXT format
    FILE* file = fopen(filename, "w");
    if (!file) return LIST_ERROR_INVALID_OPERATION;
    if (!separator) separator = "\n"; // default line-per-element

    Node* current = list->head->next;
    while (current != list->tail) {
        // Support a few primitive element sizes. For other sizes fallback to hex dump length element_size.
        if (list->element_size == sizeof(int)) {
            fprintf(file, "%d", *(int*)current->data);
        } else if (list->element_size == sizeof(double)) {
            fprintf(file, "%.*g", 15, *(double*)current->data);
        } else if (list->element_size == sizeof(char)) {
            fprintf(file, "%c", *(char*)current->data);
        } else {
            // Generic: print as bytes in hex (compact)
            unsigned char* bytes = (unsigned char*)current->data;
            for (size_t i = 0; i < list->element_size; ++i) {
                fprintf(file, "%02X", bytes[i]);
                if (i + 1 < list->element_size) fputc(' ', file);
            }
        }
        current = current->next;
        if (current != list->tail) {
            // If separator contains a newline we just print it wholly; else we add separator then maybe newline later.
            fputs(separator, file);
        }
    }
    // Ensure file ends with newline for POSIX friendliness if separator lacked one
    size_t sep_len = strlen(separator);
    if (sep_len == 0 || separator[sep_len-1] != '\n') {
        fputc('\n', file);
    }
    fclose(file);
    return LIST_SUCCESS;
}

// load_from_file: unified binary/text loader.
//   Binary: reads header then raw bytes.
//   Text: whitespace or custom separator parsing as described in header doc.
LinkedList* load_from_file(const char* filename, size_t element_size, FileFormat format,
                           const char* separator,
                           PrintFunction print_fn, CompareFunction compare_fn,
                           FreeFunction free_fn, CopyFunction copy_fn) {
    (void)compare_fn; // comparator no longer stored; kept for backward signature compatibility
    if (!filename) return NULL;
    if (format == FILE_FORMAT_BINARY) {
        FILE* bfile = fopen(filename, "rb");
        if (!bfile) return NULL;
        size_t saved_length=0, saved_element_size=0;
        if (fread(&saved_length, sizeof(size_t), 1, bfile) != 1 ||
            fread(&saved_element_size, sizeof(size_t), 1, bfile) != 1) { fclose(bfile); return NULL; }
        if (saved_element_size != element_size) { fclose(bfile); return NULL; }
        LinkedList* blist = create_list(element_size);
        if (!blist) { fclose(bfile); return NULL; }
        if (print_fn) set_print_function(blist, print_fn);
    // compare_fn provided per call now (no stored comparator)
        if (free_fn) set_free_function(blist, free_fn);
        if (copy_fn) set_copy_function(blist, copy_fn);
        for (size_t i=0;i<saved_length;i++) {
            void* elem = malloc(element_size);
            if (!elem || fread(elem, element_size, 1, bfile) != 1) { free(elem); destroy(blist); fclose(bfile); return NULL; }
            if (insert_tail_value_internal(blist, elem) != LIST_SUCCESS) { free(elem); destroy(blist); fclose(bfile); return NULL; }
            free(elem);
        }
        fclose(bfile);
        return blist;
    }

    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    LinkedList* list = create_list(element_size);
    if (!list) { fclose(file); return NULL; }
    if (print_fn) set_print_function(list, print_fn);
    // compare_fn provided per call now (no stored comparator)
    if (free_fn) set_free_function(list, free_fn);
    if (copy_fn) set_copy_function(list, copy_fn);

    // Determine actual separator behavior: if NULL fallback to whitespace tokenization (original behavior)
    if (format == FILE_FORMAT_TEXT) {
        if (!separator || separator[0] == '\0') {
            // Legacy whitespace/newline parsing.
            if (element_size == sizeof(int)) { // Parse each whitespace-delimited token as int
                int value; while (fscanf(file, "%d", &value) == 1) {
                    if (insert_tail_value_internal(list, &value) != LIST_SUCCESS) { destroy(list); fclose(file); return NULL; }
                }
            } else if (element_size == sizeof(double)) { // Parse double tokens
                double dval; while (fscanf(file, "%lf", &dval) == 1) {
                    if (insert_tail_value_internal(list, &dval) != LIST_SUCCESS) { destroy(list); fclose(file); return NULL; }
                }
            } else if (element_size == sizeof(char)) { // Read each non-newline char as element
                int c; while ((c = fgetc(file)) != EOF) {
                    if (c == '\n' || c == '\r') continue; char ch = (char)c;
                    if (insert_tail_value_internal(list, &ch) != LIST_SUCCESS) { destroy(list); fclose(file); return NULL; }
                }
            } else {
                unsigned char* buffer = (unsigned char*)malloc(element_size); // Temp buffer for hex bytes
                if (!buffer) { destroy(list); fclose(file); return NULL; }
                char line[4096];
                while (fgets(line, sizeof(line), file)) {
                    size_t count = 0; char* tok = strtok(line, " \t\r\n");
                    while (tok && count < element_size) {
                        unsigned int byteVal; if (sscanf(tok, "%02X", &byteVal) != 1) { break; }
                        buffer[count++] = (unsigned char)byteVal; tok = strtok(NULL, " \t\r\n");
                    }
                    if (count == element_size) {
                        if (insert_tail_value_internal(list, buffer) != LIST_SUCCESS) { free(buffer); destroy(list); fclose(file); return NULL; }
                    }
                }
                free(buffer);
            }
        } else {
            // Custom separator parsing: read entire file, split by exact separator string.
            // Load whole file into memory (teaching simplification; not for huge files)
            fseek(file, 0, SEEK_END); long fsize = ftell(file); if (fsize < 0) { destroy(list); fclose(file); return NULL; }
            rewind(file);
            char* content = (char*)malloc(fsize + 1);
            if (!content) { destroy(list); fclose(file); return NULL; }
            size_t read_bytes = fread(content, 1, fsize, file); content[read_bytes] = '\0';

            size_t sep_len = strlen(separator); // We'll split by this exact substring
            char* start = content; char* pos;
            while ((pos = strstr(start, separator)) != NULL) {
                *pos = '\0';
                if (*start != '\0') { // Non-empty token
                    if (element_size == sizeof(int)) {
                        int v; if (sscanf(start, "%d", &v) == 1) {
                            if (insert_tail_value_internal(list, &v) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                        }
                    } else if (element_size == sizeof(double)) {
                        double dv; if (sscanf(start, "%lf", &dv) == 1) {
                            if (insert_tail_value_internal(list, &dv) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                        }
                    } else if (element_size == sizeof(char)) {
                        if (start[0] != '\0') {
                            char ch = start[0];
                            if (insert_tail_value_internal(list, &ch) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                        }
                    } else {
                        // Unsupported complex type for custom separator parsing; user should fall back to binary/hex whitespace mode.
                    }
                }
                start = pos + sep_len;
            }
            // Last token
            if (*start != '\0') { // Final token after last separator
                if (element_size == sizeof(int)) {
                    int v; if (sscanf(start, "%d", &v) == 1) {
                        if (insert_tail_value_internal(list, &v) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                    }
                } else if (element_size == sizeof(double)) {
                    double dv; if (sscanf(start, "%lf", &dv) == 1) {
                        if (insert_tail_value_internal(list, &dv) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                    }
                } else if (element_size == sizeof(char)) {
                    char ch = start[0]; if (insert_tail_value_internal(list, &ch) != LIST_SUCCESS) { free(content); destroy(list); fclose(file); return NULL; }
                }
            }
            free(content);
        }
    }

    fclose(file);
    return list;
}
