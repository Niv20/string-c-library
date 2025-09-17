#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple structure for demo
typedef struct {
    int id;
    char name[50];
} Person;

void print_person(void* data) {
    const Person* p = (const Person*)data;
    printf("{ID:%d, Name:\"%s\"}", p->id, p->name);
}

int compare_person(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return p1->id - p2->id;
}

int main() {
    printf("=== Demonstration of Value Mode vs Pointer Mode ===\n\n");
    
    // Create two lists
    LinkedList* value_list = create_list(sizeof(Person));
    LinkedList* ptr_list = create_list(sizeof(Person));
    
    set_print_function(value_list, print_person);
    set_compare_function(value_list, compare_person);
    set_print_function(ptr_list, print_person);
    set_compare_function(ptr_list, compare_person);
    
    // Demo 1: Value Mode - Local Variables (SAFE)
    printf("1. VALUE MODE with local variables:\n");
    {
        Person alice = {1, "Alice"};
        Person bob = {2, "Bob"};
        
        printf("   Inserting local variables using insert_*_value...\n");
        insert_tail_value(value_list, alice);
        insert_tail_value(value_list, bob);
        
        printf("   Data is copied into list's memory, local variables can be destroyed\n");
        // alice and bob go out of scope here, but list data is safe
    }
    
    printf("   List contents: ");
    print_advanced(value_list, false, ", ");
    printf("\n\n");
    
    // Demo 2: Pointer Mode - Heap Variables (SAFE)
    printf("2. POINTER MODE with heap-allocated variables:\n");
    Person* charlie = malloc(sizeof(Person));
    charlie->id = 3;
    strcpy(charlie->name, "Charlie");
    
    Person* diana = malloc(sizeof(Person));
    diana->id = 4;
    strcpy(diana->name, "Diana");
    
    printf("   Inserting heap-allocated variables using insert_*_ptr...\n");
    insert_tail_ptr(ptr_list, charlie);
    insert_tail_ptr(ptr_list, diana);
    
    printf("   List stores pointers to your data, you manage the memory\n");
    printf("   List contents: ");
    print_advanced(ptr_list, false, ", ");
    printf("\n\n");
    
    // Demo 3: What NOT to do - Pointer Mode with Local Variables (DANGEROUS!)
    printf("3. DANGEROUS - POINTER MODE with local variables:\n");
    LinkedList* dangerous_list = create_list(sizeof(Person));
    set_print_function(dangerous_list, print_person);
    
    {
        Person eve = {5, "Eve"};
        printf("   Inserting local variable using insert_*_ptr (BAD IDEA!)...\n");
        insert_tail_ptr(dangerous_list, &eve);
        // eve goes out of scope here - creates dangling pointer!
    }
    
    printf("   Trying to print list with dangling pointer (undefined behavior):\n");
    printf("   List contents: ");
    // This might crash or print garbage!
    print_advanced(dangerous_list, false, ", ");
    printf("\n\n");
    
    // Cleanup
    printf("4. Cleanup:\n");
    printf("   Value list: Library manages all memory - just destroy\n");
    destroy(value_list);
    
    printf("   Pointer list: You must free your data first, then destroy list\n");
    free(charlie);
    free(diana);
    destroy(ptr_list);
    
    printf("   Dangerous list: Destroy without accessing dangling pointers\n");
    destroy(dangerous_list);
    
    printf("\n=== Summary ===\n");
    printf("• Use VALUE MODE when you have local variables or want the list to manage memory\n");
    printf("• Use POINTER MODE when you have heap-allocated data and want to manage memory yourself\n");
    printf("• NEVER use POINTER MODE with local variables - creates dangling pointers!\n");
    
    return 0;
}