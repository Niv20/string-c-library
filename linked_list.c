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
        default: return "Unknown error";
    }
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃             List Configuration                ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Sets the element size of the list.
 * @param list The list to configure.
 * @param element_size The size of elements in bytes.
 */
void list_set_element_size(LinkedList* list, size_t element_size) {
    if (list) {
        list->element_size = element_size;
    }
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
    
    // If reducing max_size and current length exceeds new limit
    if (max_size > 0 && list->length > max_size) {
        if (!allow_overwrite) {
            return LIST_ERROR_OVERWRITE_DISABLED;
        }
        
        // Remove excess elements from the head (oldest in FIFO)
        while (list->length > max_size) {
            ListResult result = list_delete_from_head(list);
            if (result != LIST_SUCCESS) return result;
        }
    }
    
    list->max_size = max_size;
    list->allow_overwrite = allow_overwrite;
    return LIST_SUCCESS;
}

/**
 * @brief Gets the maximum size setting of the list.
 * @param list The list to query.
 * @return The maximum size (0 = unlimited).
 */
size_t list_get_max_size(const LinkedList* list) {
    return list ? list->max_size : 0;
}

/**
 * @brief Checks if the list allows overwriting when full.
 * @param list The list to query.
 * @return True if overwrite is allowed, false otherwise.
 */
bool list_allows_overwrite(const LinkedList* list) {
    return list ? list->allow_overwrite : false;
}


/**
 * @brief Sets the print function for the list.
 * @param list The list to configure.
 * @param print_fn Function pointer for printing elements.
 */
void list_set_print_function(LinkedList* list, PrintFunction print_fn) {
    if (list)
        list->print_function = print_fn;
}

/**
 * @brief Sets the compare function for the list.
 * @param list The list to configure.
 * @param compare_fn Function pointer for comparing elements.
 */
void list_set_compare_function(LinkedList* list, CompareFunction compare_fn) {
    if (list)
        list->compare_function = compare_fn;
}

/**
 * @brief Sets the free function for the list.
 * @param list The list to configure.
 * @param free_fn Function pointer for freeing complex elements.
 */
void list_set_free_function(LinkedList* list, FreeFunction free_fn) {
    if (list)
        list->free_function = free_fn;
}

/**
 * @brief Sets the copy function for the list.
 * @param list The list to configure.
 * @param copy_fn Function pointer for deep copying complex elements.
 */
void list_set_copy_function(LinkedList* list, CopyFunction copy_fn) {
    if (list)
        list->copy_function = copy_fn;
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              Lifecycle Functions              ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Creates and initializes a new linked list.
 * @param element_size The size of the data type that will be stored in the list.
 * @param print_fn Optional function pointer for printing elements.
 * @param compare_fn Optional function pointer for comparing elements (required for sorting/finding).
 * @param free_fn Optional function pointer for freeing complex data types.
 * @param copy_fn Optional function pointer for deep copying complex data types.
 * @return A pointer to the newly created LinkedList, or NULL on failure.
 */
LinkedList* list_create(size_t element_size, PrintFunction print_fn,
                        CompareFunction compare_fn, FreeFunction free_fn, CopyFunction copy_fn) {
    return list_create_with_limits(element_size, 0, false, print_fn, compare_fn, free_fn, copy_fn);
}

/**
 * @brief Creates and initializes a new linked list with size constraints.
 * @param element_size The size of the data type that will be stored in the list.
 * @param max_size Maximum number of elements (0 = unlimited).
 * @param allow_overwrite Whether to overwrite oldest elements when full.
 * @param print_fn Optional function pointer for printing elements.
 * @param compare_fn Optional function pointer for comparing elements.
 * @param free_fn Optional function pointer for freeing complex data types.
 * @param copy_fn Optional function pointer for deep copying complex data types.
 * @return A pointer to the newly created LinkedList, or NULL on failure.
 */
LinkedList* list_create_with_limits(size_t element_size, size_t max_size, bool allow_overwrite,
                                   PrintFunction print_fn, CompareFunction compare_fn, 
                                   FreeFunction free_fn, CopyFunction copy_fn) {
    
    // Allocate memory for the list manager structure
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (!list) {
        return NULL; // Allocation failed
    }

    // Initialize all fields
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    list->element_size = element_size;
    list->max_size = max_size;
    list->allow_overwrite = allow_overwrite;
    list->print_function = print_fn;
    list->compare_function = compare_fn;
    list->free_function = free_fn;
    list->copy_function = copy_fn;

    // Create dummy nodes
    list->head = (Node*)calloc(1, sizeof(Node)); // Dummy head
    if (!list->head) {
        free(list);
        return NULL;
    }
    list->tail = (Node*)calloc(1, sizeof(Node)); // Dummy tail
    if (!list->tail) {
        free(list->head);
        free(list);
        return NULL;
    }

    // Link dummy nodes
    list->head->next = list->tail;
    list->tail->prev = list->head;
    
    return list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃          Internal helper functions            ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */ 

static ListResult handle_size_limit(LinkedList* list) {
    if (list->max_size == 0) return LIST_SUCCESS; // No limit
    
    if (list->length >= list->max_size) {
        if (!list->allow_overwrite) {
            return LIST_ERROR_LIST_FULL;
        }
        
        // Remove oldest element (from head) to make room for FIFO behavior
        ListResult result = list_delete_from_head(list);
        if (result != LIST_SUCCESS) return result;
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Creates a new node with data (internal helper function).
 * @param list The list (for element_size and copy_function).
 * @param data The data to copy into the new node.
 * @return Pointer to new node on success, NULL on failure.
 */
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
    if (list->copy_function) {
        list->copy_function(new_node->data, data);
    } else {
        memcpy(new_node->data, data, list->element_size);
    }
    
    // Initialize pointers
    new_node->next = NULL;
    new_node->prev = NULL;
    
    return new_node;
}


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃           Insertion in Linked List            ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Inserts a new element at the head of the list.
 * @param list The list to insert into.
 * @param data A pointer to the data to be inserted. The data is copied into the list.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_at_head(LinkedList* list, void* data) {
    if (!list || !data) return LIST_ERROR_NULL_POINTER;

    // Check size limits before insertion
    ListResult size_check = handle_size_limit(list);
    if (size_check != LIST_SUCCESS) return size_check;

    // Create new node with data
    Node* new_node = create_node_with_data(list, data);
    if (!new_node) return LIST_ERROR_MEMORY_ALLOC;

    // Link the new node into the list (always with dummy nodes)
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
    if (!list || !data) return LIST_ERROR_NULL_POINTER;

    // Check size limits before insertion
    ListResult size_check = handle_size_limit(list);
    if (size_check != LIST_SUCCESS) return size_check;

    // Create new node with data
    Node* new_node = create_node_with_data(list, data);
    if (!new_node) return LIST_ERROR_MEMORY_ALLOC;

    // Link the new node into the list (always with dummy nodes)
    Node* old_last = list->tail->prev;
    list->tail->prev = new_node;
    new_node->next = list->tail;
    new_node->prev = old_last;
    old_last->next = new_node;
    
    list->length++;
    return LIST_SUCCESS;
}

/**
 * @brief Appends an element to the end of the list (like Python's append).
 * @param list The list to append to.
 * @param data The data to append.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_append(LinkedList* list, void* data) {
    return list_insert_at_tail(list, data);
}

/**
 * @brief Inserts an element at a specific index (like Python's insert).
 * @param list The list to insert into.
 * @param index The index to insert at (0-based).
 * @param data The data to insert.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_at_index(LinkedList* list, size_t index, void* data) {
    if (!list || !data) return LIST_ERROR_NULL_POINTER;
    
    if (index == 0) return list_insert_at_head(list, data);
    if (index >= list->length) return list_append(list, data);
    
    // Check size limits before insertion
    ListResult size_check = handle_size_limit(list);
    if (size_check != LIST_SUCCESS) return size_check;
    
    // Create new node with data
    Node* new_node = create_node_with_data(list, data);
    if (!new_node) return LIST_ERROR_MEMORY_ALLOC;
    
    Node* current;
    
    // Optimization: choose direction based on which half the index is in
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

// Legacy Node function for inserting after a specific node
bool insert_after(Node *node, int data) {
    if (!node) return false;
    
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return false;
    
    new_node->data = malloc(sizeof(int));
    if (!new_node->data) {
        free(new_node);
        return false;
    }
    
    *(int*)new_node->data = data;
    new_node->next = node->next;
    new_node->prev = node;
    
    if (node->next) {
        node->next->prev = new_node;
    }
    node->next = new_node;
    
    return true;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃              Deletion Functions               ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Deletes the element at the head of the list.
 * @param list The list to delete from.
 * @return LIST_SUCCESS on success, error code if the list is empty.
 */
ListResult list_delete_from_head(LinkedList* list) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Identify the node to delete (always dummy nodes)
    Node* node_to_delete = list->head->next;

    // Re-link the list
    Node* prev_node = node_to_delete->prev;
    Node* next_node = node_to_delete->next;
    
    prev_node->next = next_node;
    next_node->prev = prev_node;
    
    // Free the data and the node
    if (list->free_function) {
        list->free_function(node_to_delete->data);
    }
    free(node_to_delete->data);
    free(node_to_delete);
    
    list->length--;
    return LIST_SUCCESS;
}


/**
 * @brief Deletes the element at the tail of the list.
 * @param list The list to delete from.
 * @return LIST_SUCCESS on success, error code if the list is empty.
 */
ListResult list_delete_from_tail(LinkedList* list) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;

    // Identify the node to delete (always dummy nodes)
    Node* node_to_delete = list->tail->prev;
    
    // Re-link the list
    Node* prev_node = node_to_delete->prev;
    Node* next_node = node_to_delete->next;
    
    prev_node->next = next_node;
    next_node->prev = prev_node;
    
    // Free the data and the node
    if (list->free_function) {
        list->free_function(node_to_delete->data);
    }
    free(node_to_delete->data);
    free(node_to_delete);
    
    list->length--;
    return LIST_SUCCESS;
}

/**
 * @brief Removes and returns the element at the given index (like Python's pop).
 * @param list The list to pop from.
 * @param index The index to pop (if -1 or list->length, pops last element).
 * @param out_data Buffer to store the popped data (optional).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_pop(LinkedList* list, int index, void* out_data) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    size_t actual_index;
    if (index < 0 || (size_t)index >= list->length) {
        actual_index = list->length - 1; // Pop last element
    } else {
        actual_index = (size_t)index;
    }
    
    // Find the node
    Node* current = list->head->next;
    for (size_t i = 0; i < actual_index; i++) {
        current = current->next;
    }
    
    // Copy data if requested
    if (out_data) {
        if (list->copy_function) {
            list->copy_function(out_data, current->data);
        } else {
            memcpy(out_data, current->data, list->element_size);
        }
    }
    
    // Remove node
    current->prev->next = current->next;
    current->next->prev = current->prev;
    
    if (list->free_function) {
        list->free_function(current->data);
    }
    free(current->data);
    free(current);
    
    list->length--;
    return LIST_SUCCESS;
}

/**
 * @brief Removes the first occurrence of a value (like Python's remove).
 * @param list The list to remove from.
 * @param data The data to find and remove.
 * @return LIST_SUCCESS if found and removed, error code otherwise.
 */
ListResult list_remove(LinkedList* list, void* data) {
    if (!list || !data) return LIST_ERROR_NULL_POINTER;
    if (!list->compare_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    
    Node* current = list->head->next;
    while (current != list->tail) {
        if (list->compare_function(current->data, data) == 0) {
            // Found it, remove
            current->prev->next = current->next;
            current->next->prev = current->prev;
            
            if (list->free_function) {
                list->free_function(current->data);
            }
            free(current->data);
            free(current);
            
            list->length--;
            return LIST_SUCCESS;
        }
        current = current->next;
    }
    return LIST_ERROR_ELEMENT_NOT_FOUND;
}


/**
 * @brief Clears all elements from the list (like Python's clear).
 * @param list The list to clear.
 */
void list_clear(LinkedList* list) {
    
    if (!list) return;
    
    while (!list_is_empty(list)) {
        list_delete_from_head(list);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Legacy Node function for removing all matching values
void delete_all_matches(Node **head, int value_to_delete) {
    Node *current = *head;
    Node *next;

    while (current) {
        next = current->next;
        if (*(int*)current->data == value_to_delete) {
            if (current == *head) {
                *head = current->next;
            } else {
                current->prev->next = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            }
            free(current);
        }
        current = next;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Legacy Node function for efficient single match deletion
bool efficient_delete_match(Node **head, int value) {
    Node *current = *head;
    
    while (current) {
        if (*(int*)current->data == value) {
            if (current == *head) {
                *head = current->next;
            } else {
                current->prev->next = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            }
            free(current);
            return true;
        }
        current = current->next;
    }
    return false;
}


/**
 * @brief Frees all memory associated with the list.
 * This includes all nodes and the data within them (using the free_function if provided).
 * @param list A pointer to the LinkedList to be destroyed.
 */
void list_destroy(LinkedList* list) {
    if (!list) return;

    // Start from the first actual node (always dummy nodes now)
    Node* current = list->head->next;
    Node* end_sentinel = list->tail;

    // Iterate through all nodes and free them
    while (current != end_sentinel) {
        Node* next = current->next;
        
        // If a custom free function is provided, use it to free the data
        if (list->free_function) {
            list->free_function(current->data);
        }
        // Always free the data pointer itself and the node
        free(current->data);
        free(current);
        
        current = next;
    }

    // Free dummy nodes
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
 * @return True if the list is empty, false otherwise.
 */
bool list_is_empty(const LinkedList* list) {
    return !list || list->length == 0;
}

/**
 * @brief Prints all elements in the list using the provided print function.
 * @param list The list to print.
 */
void list_print(const LinkedList* list) {
    if (!list) {
        printf("List is NULL.\n");
        return;
    }
    if (!list->print_function) {
        printf("No print function provided for this list.\n");
        return;
    }
    if (list_is_empty(list)) {
        printf("List is empty.\n");
        return;
    }

    // Determine starting and ending points (always dummy nodes)
    Node* current = list->head->next;
    Node* end_sentinel = list->tail;

    printf("List (length %zu):\n", list->length);
    int index = 0;
    while(current != end_sentinel) {
        printf("  [%d]: ", index++);
        list->print_function(current->data);
        printf("\n");
        current = current->next;
    }
}

// Legacy Node utility function
int recursive_length(Node *node) {
    if (!node) return 0;
    return 1 + recursive_length(node->next);
}

/**
 * @brief Gets a pointer to the first node for forward iteration.
 * @param list The list to iterate.
 * @return Pointer to the first node, or NULL if empty.
 */
Node* list_begin(const LinkedList* list) {
    if (!list || list_is_empty(list)) return NULL;
    return list->head->next;
}

/**
 * @brief Gets a pointer to the last node for backward iteration.
 * @param list The list to iterate.
 * @return Pointer to the last node, or NULL if empty.
 */
Node* list_rbegin(const LinkedList* list) {
    if (!list || list_is_empty(list)) return NULL;
    return list->tail->prev;
}

/**
 * @brief Gets the end sentinel for forward iteration.
 * @param list The list to iterate.
 * @return Pointer to the end sentinel (dummy tail).
 */
Node* list_end(const LinkedList* list) {
    if (!list) return NULL;
    return list->tail;
}

/**
 * @brief Gets the reverse end sentinel for backward iteration.
 * @param list The list to iterate.
 * @return Pointer to the reverse end sentinel (dummy head).
 */
Node* list_rend(const LinkedList* list) {
    if (!list) return NULL;
    return list->head;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃         Search and Access Functions           ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

/**
 * @brief Gets an element at a specific index (like Python's list[index]).
 * @param list The list to get from.
 * @param index The index to get.
 * @param out_data Buffer to store the retrieved data.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_get(const LinkedList* list, size_t index, void* out_data) {
    if (!list || !out_data) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    Node* current;
    
    // Optimization: choose direction based on which half the index is in
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
    
    if (list->copy_function) {
        list->copy_function(out_data, current->data);
    } else {
        memcpy(out_data, current->data, list->element_size);
    }
    
    return LIST_SUCCESS;
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
    
    Node* current;
    
    // Optimization: choose direction based on which half the index is in
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
    
    // Free old data if needed
    if (list->free_function) {
        list->free_function(current->data);
    }
    
    // Set new data
    if (list->copy_function) {
        list->copy_function(current->data, data);
    } else {
        memcpy(current->data, data, list->element_size);
    }
    
    return LIST_SUCCESS;
}

/**
 * @brief Returns the index of the first occurrence of a value (like Python's index).
 * @param list The list to search in.
 * @param data The data to find.
 * @return The index if found, -1 if not found.
 */
int list_index(const LinkedList* list, void* data) {
    if (!list || !data || !list->compare_function) return -1;
    
    Node* current = list->head->next;
    int index = 0;
    while (current != list->tail) {
        if (list->compare_function(current->data, data) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

/**
 * @brief Counts the number of occurrences of a value (like Python's count).
 * @param list The list to count in.
 * @param data The data to count.
 * @return The number of occurrences.
 */
size_t list_count(const LinkedList* list, void* data) {
    if (!list || !data || !list->compare_function) return 0;
    
    size_t count = 0;
    Node* current = list->head->next;
    while (current != list->tail) {
        if (list->compare_function(current->data, data) == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

// Legacy Node function for value replacement
void replace_matches(Node *node, int find_value, int replace_value) {
    while (node) {
        if (*(int*)node->data == find_value) *(int*)node->data = replace_value;
        node = node->next;
    }
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

/**
 * @brief Sorts the list in place (like Python's sort).
 * @param list The list to sort.
 * @param reverse If true, sorts in descending order.
 */
void list_sort(LinkedList* list, bool reverse_order) {
    if (!list || !list->compare_function || list->length <= 1) return;
    
    // Simple bubble sort for now (can be optimized later)
    bool swapped;
    do {
        swapped = false;
        Node* current = list->head->next;
        
        while (current->next != list->tail) {
            Node* next_node = current->next;
            int cmp = list->compare_function(current->data, next_node->data);
            
            if ((reverse_order && cmp < 0) || (!reverse_order && cmp > 0)) {
                // Swap data
                void* temp = current->data;
                current->data = next_node->data;
                next_node->data = temp;
                swapped = true;
            }
            current = current->next;
        }
    } while (swapped);
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
        ListResult result = list_append(list, current->data);
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
    
    LinkedList* new_list = list_create(list->element_size, list->print_function,
                                      list->compare_function, list->free_function,
                                      list->copy_function);
    if (!new_list) return NULL;
    
    if (!list_extend(new_list, list)) {
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
    if (!list->compare_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    Node* current = list->head->next;
    
    // Initialize with first element
    if (list->copy_function) {
        list->copy_function(out_min, current->data);
    } else {
        memcpy(out_min, current->data, list->element_size);
    }
    
    current = current->next;
    
    // Compare with rest of elements
    while (current != list->tail) {
        if (list->compare_function(current->data, out_min) < 0) {
            if (list->copy_function) {
                list->copy_function(out_min, current->data);
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
    if (!list->compare_function) return LIST_ERROR_NO_COMPARE_FUNCTION;
    if (list_is_empty(list)) return LIST_ERROR_INVALID_OPERATION;
    
    Node* current = list->head->next;
    
    // Initialize with first element
    if (list->copy_function) {
        list->copy_function(out_max, current->data);
    } else {
        memcpy(out_max, current->data, list->element_size);
    }
    
    current = current->next;
    
    // Compare with rest of elements
    while (current != list->tail) {
        if (list->compare_function(current->data, out_max) > 0) {
            if (list->copy_function) {
                list->copy_function(out_max, current->data);
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
    
    LinkedList* filtered = list_create(list->element_size, list->print_function,
                                     list->compare_function, list->free_function, 
                                     list->copy_function);
    if (!filtered) return NULL;
    
    Node* current = list->head->next;
    while (current != list->tail) {
        if (filter_fn(current->data)) {
            if (list_append(filtered, current->data) != LIST_SUCCESS) {
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
    
    LinkedList* mapped = list_create(new_element_size, NULL, NULL, 
                                   list->free_function, list->copy_function);
    if (!mapped) return NULL;
    
    Node* current = list->head->next;
    while (current != list->tail) {
        void* transformed = malloc(new_element_size);
        if (!transformed) {
            list_destroy(mapped);
            return NULL;
        }
        
        map_fn(transformed, current->data);
        
        if (list_append(mapped, transformed) != LIST_SUCCESS) {
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
    
    LinkedList* sliced = list_create(list->element_size, list->print_function,
                                   list->compare_function, list->free_function,
                                   list->copy_function);
    if (!sliced) return NULL;
    
    Node* current = list->head->next;
    
    // Skip to start position
    for (size_t i = 0; i < start && current != list->tail; i++) {
        current = current->next;
    }
    
    // Copy elements from start to end
    for (size_t i = start; i < end && current != list->tail; i++) {
        if (list_append(sliced, current->data) != LIST_SUCCESS) {
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
    
    LinkedList* concatenated = list_create(list1->element_size, list1->print_function,
                                         list1->compare_function, list1->free_function,
                                         list1->copy_function);
    if (!concatenated) return NULL;
    
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
    if (!list->print_function) return NULL;
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
    
    LinkedList* list = list_create(element_size, print_fn, compare_fn, free_fn, copy_fn);
    if (!list) {
        fclose(file);
        return NULL;
    }
    
    // Read elements
    for (size_t i = 0; i < saved_length; i++) {
        void* element = malloc(element_size);
        if (!element || fread(element, element_size, 1, file) != 1) {
            free(element);
            list_destroy(list);
            fclose(file);
            return NULL;
        }
        
        if (list_append(list, element) != LIST_SUCCESS) {
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
    if (!list || !list->compare_function) return NULL;
    
    LinkedList* unique = list_create(list->element_size, list->print_function,
                                   list->compare_function, list->free_function,
                                   list->copy_function);
    if (!unique) return NULL;
    
    Node* current = list->head->next;
    while (current != list->tail) {
        // Check if element already exists in unique list
        if (list_index(unique, current->data) == -1) {
            if (list_append(unique, current->data) != LIST_SUCCESS) {
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
    if (!list1 || !list2 || !list1->compare_function) return NULL;
    if (list1->element_size != list2->element_size) return NULL;
    
    LinkedList* intersection = list_create(list1->element_size, list1->print_function,
                                         list1->compare_function, list1->free_function,
                                         list1->copy_function);
    if (!intersection) return NULL;
    
    Node* current = list1->head->next;
    while (current != list1->tail) {
        // If element exists in both lists and not already in result
        if (list_index(list2, current->data) != -1 && 
            list_index(intersection, current->data) == -1) {
            if (list_append(intersection, current->data) != LIST_SUCCESS) {
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
            if (list_append(union_list, current->data) != LIST_SUCCESS) {
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
        if (list->copy_function) {
            list->copy_function(dest, current->data);
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
 * @brief Creates a new integer list.
 * @return A new LinkedList configured for integers, or NULL on failure.
 */
LinkedList* list_create_int(void) {
    return list_create(sizeof(int), print_int, compare_int, NULL, NULL);
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
 * @brief Creates a new double list.
 * @return A new LinkedList configured for doubles, or NULL on failure.
 */
LinkedList* list_create_double(void) {
    return list_create(sizeof(double), print_double, compare_double, NULL, NULL);
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
 * @brief Creates a new character list.
 * @return A new LinkedList configured for characters, or NULL on failure.
 */
LinkedList* list_create_char(void) {
    return list_create(sizeof(char), print_char, compare_char, NULL, NULL);
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
 * @brief Creates a new string list.
 * @return A new LinkedList configured for strings, or NULL on failure.
 */
LinkedList* list_create_string(void) {
    return list_create(sizeof(char*), print_string_builtin, compare_string_builtin, 
                      free_string_builtin, copy_string_builtin);
}