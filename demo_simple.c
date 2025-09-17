#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Helper function prototypes for integers
void print_int(void* data);
int compare_int(const void* a, const void* b);
void banner(const char* title);

// Implementation of helper functions
void print_int(void* data) {
    if (data)
        printf("%d", *(int*)data);
}

int compare_int(const void* a, const void* b) {
    return (*(const int*)a - *(const int*)b);
}

void banner(const char* title) {
    printf("\n\n========== %s ==========\n", title);
}

// Main simple demo function
void simple_linked_list_demo(void) {
    
    banner("SIMPLE LINKED LIST DEMO");
    printf("This demo showcases basic features using simple integer types.\n");
    printf("Perfect for understanding the core concepts without complex memory management.\n\n");
    
    // ===== Create List =====
    banner("Create List");
    printf("Creating a new linked list for integers...\n");
    
    LinkedList* numbers_list = create_list(sizeof(int));
    if (!numbers_list) {
        printf("Error: Failed to create list!\n");
        return;
    }
    printf("✓ List created successfully\n");
    
    // ===== List Configuration =====
    banner("List Configuration");
    printf("Configuring list with helper functions...\n");
    set_print_function(numbers_list, print_int);
    set_compare_function(numbers_list, compare_int);
    printf("✓ List configured with print and compare functions\n");
    
    printf("Initial list status: Empty=%s, Length=%zu\n", 
           is_empty(numbers_list) ? "Yes" : "No", get_length(numbers_list));

    // ===== Insertion in Linked List =====
    banner("Insertion in Linked List");
    printf("Demonstrating both traditional and convenient insertion methods...\n");
    
    // Traditional way (pointer-based)
    printf("Traditional insertion using pointers:\n");
    int num1 = 100;
    int num2 = 200; 
    int num3 = 300;
    
    insert_tail_ptr(numbers_list, &num1);
    insert_head_ptr(numbers_list, &num2);
    insert_index_ptr(numbers_list, 1, &num3);
    
    printf("List after traditional insertions: ");
    print_advanced(numbers_list, false, ", ");
    
    // New convenient way - pass values directly!
    printf("Convenient insertion using value-based macros:\n");
    insert_tail_val(numbers_list, 400);
    insert_head_val(numbers_list, 50);
    insert_index_val(numbers_list, 2, 150);
    
    printf("List after value-based insertions: ");
    print_advanced(numbers_list, false, ", ");
    
    printf("✓ Value-based insertion macros work perfectly for simple types!\n");
    printf("For simple types: insert_tail_val(list, 42)\n");
    printf("For complex types: insert_tail_ptr(list, &your_struct)\n");
    
    // ===== Search and Access Functions =====
    banner("Search and Access Functions");
    
    // Get element by index
    printf("Accessing element at index 2:\n");
    int* value_at_2 = (int*)get(numbers_list, 2);
    if (value_at_2) {
        printf("Value at index 2: %d\n", *value_at_2);
    }
    
    // Search for element
    printf("Searching for value 300...\n");
    int search_value = 300;
    int found_index = index_of(numbers_list, &search_value);
    if (found_index >= 0) {
        printf("Value 300 found at index: %d\n", found_index);
    } else {
        printf("Value 300 not found (error code: %d)\n", found_index);
    }
    
    // ===== Sorting Functions =====
    banner("Sorting Functions");
    
    printf("Current list: ");
    print_advanced(numbers_list, false, ", ");
    
    printf("Sorting in ascending order...\n");
    sort(numbers_list, false);
    printf("List sorted (ascending): ");
    print_advanced(numbers_list, false, ", ");
    
    printf("Sorting in descending order...\n");
    sort(numbers_list, true);
    printf("List sorted (descending): ");
    print_advanced(numbers_list, false, ", ");
    
    // ===== Mathematical Functions =====
    banner("Mathematical Functions");
    
    // Find min and max
    int* min_value = (int*)min_by(numbers_list, compare_int);
    int* max_value = (int*)max_by(numbers_list, compare_int);
    
    if (min_value) {
        printf("Minimum value: %d\n", *min_value);
    }
    if (max_value) {
        printf("Maximum value: %d\n", *max_value);
    }
    
    // ===== Utility Functions =====
    banner("Utility Functions");
    
    printf("List length: %zu\n", get_length(numbers_list));
    printf("Is empty: %s\n", is_empty(numbers_list) ? "Yes" : "No");
    
    printf("Printing with different formats:\n");
    printf("With indices: ");
    print_advanced(numbers_list, true, " | ");
    printf("Comma separated: ");
    print_advanced(numbers_list, false, ", ");
    printf("Arrow separated: ");
    print_advanced(numbers_list, false, " -> ");
    
    // ===== List <--> Array =====
    banner("List <--> Array");
    
    // Convert list to array
    printf("Converting list to array...\n");
    size_t array_size;
    int* number_array = (int*)to_array(numbers_list, &array_size);
    if (number_array) {
        printf("Array created with %zu elements: ", array_size);
        for (size_t i = 0; i < array_size; i++) {
            printf("%d", number_array[i]);
            if (i < array_size - 1) printf(", ");
        }
        printf("\n");
        free(number_array);
    }
    
    // Convert array to new list
    printf("Creating new list from array...\n");
    LinkedList* array_list = create_list(sizeof(int));
    set_print_function(array_list, print_int);
    set_compare_function(array_list, compare_int);
    
    int source_array[] = {10, 20, 30, 40, 50};
    from_array(array_list, source_array, 5);
    printf("New list created from array: ");
    print_advanced(array_list, false, ", ");
    
    // ===== Deletion Functions =====
    banner("Deletion Functions");
    
    printf("List before deletions: ");
    print_advanced(numbers_list, false, ", ");
    
    // Delete from different positions
    printf("Deleting from head...\n");
    delete_head(numbers_list);
    printf("After deleting head: ");
    print_advanced(numbers_list, false, ", ");
    
    printf("Deleting from tail...\n");
    delete_tail(numbers_list);
    printf("After deleting tail: ");
    print_advanced(numbers_list, false, ", ");
    
    if (get_length(numbers_list) > 1) {
        printf("Deleting at index 1...\n");
        delete_index(numbers_list, 1);
        printf("After deleting index 1: ");
        print_advanced(numbers_list, false, ", ");
    }
    
    // ===== List <--> String (file) =====
    banner("List <--> String (file)");
    
    printf("Converting list to string representation...\n");
    char* list_string = to_string(numbers_list, ", ");
    if (list_string) {
        printf("List as string: [%s]\n", list_string);
        free(list_string);
    }
    
    printf("Saving list to file...\n");
    ListResult save_result = save_to_file(numbers_list, "simple_demo_numbers.bin");
    printf("Save result: %s\n", error_string(save_result));
    
    printf("Loading list from file...\n");
    LinkedList* loaded_list = load_from_file("simple_demo_numbers.bin", sizeof(int),
                                           print_int, compare_int, NULL, NULL);
    if (loaded_list) {
        printf("List loaded from file: ");
        print_advanced(loaded_list, false, ", ");
        destroy(loaded_list);
    }
    
    // ===== Error Handling =====
    banner("Error Handling");
    
    printf("Demonstrating error handling with simple operations...\n");
    
    // Try to access out of bounds
    int* invalid = (int*)get(numbers_list, 999);
    printf("Accessing index 999: %s\n", invalid ? "Success" : "Failed (as expected)");
    
    // Try to delete from empty list
    LinkedList* empty_list = create_list(sizeof(int));
    ListResult result = delete_head(empty_list);
    printf("Delete from empty list: %s\n", error_string(result));
    destroy(empty_list);
    
    // ===== CLEANUP =====
    banner("CLEANUP");
    printf("Cleaning up all allocated memory...\n");
    
    destroy(numbers_list);
    destroy(array_list);
    
    printf("✓ All memory cleaned up successfully\n");
    
    banner("SIMPLE DEMO COMPLETED");
    printf("This simple demo showcased basic linked list operations:\n");
    printf("• List creation and configuration\n");
    printf("• Insertion operations (traditional and value-based)\n");
    printf("• Search and access operations\n");
    printf("• Sorting operations\n");
    printf("• Mathematical operations\n");
    printf("• Utility functions\n");
    printf("• Array conversions\n");
    printf("• Deletion operations\n");
    printf("• String representation and file I/O\n");
    printf("• Error handling\n");
    printf("\nPerfect for learning the basics with simple integer types!\n");
}

// Main function
int main(void) {
    printf("Generic Linked List Library - Simple Demo\n");
    printf("==========================================\n");
    printf("This demo showcases basic features using simple integer types.\n");
    printf("No complex memory management - perfect for learning!\n");
    
    simple_linked_list_demo();
    
    return 0;
}