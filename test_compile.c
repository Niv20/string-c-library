#include "linked_list.h"
#include <stdio.h>

int main() {
    LinkedList* list = list_create_int();
    if (list) {
        printf("List created successfully!\n");
        
        int value = 42;
        list_append(list, &value);
        
        printf("List length: %zu\n", list_get_length(list));
        list_print(list);
        
        list_destroy(list);
        printf("List destroyed successfully!\n");
    }
    return 0;
}
