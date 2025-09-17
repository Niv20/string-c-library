#include "linked_list.h"
#include <stdio.h>

int main(void) {
    LinkedList* list = create_list(sizeof(int));
    if (!list) {
        printf("Failed to create list\n");
        return 1;
    }
    
    printf("List created successfully\n");
    
    // Simple insert test
    int value = 42;
    insert_tail_val(list, value);
    
    printf("Value inserted: %d\n", value);
    printf("List length: %zu\n", get_length(list));
    
    destroy(list);
    return 0;
}