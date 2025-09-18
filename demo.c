#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Person structure - storing name in dynamically allocated memory
typedef struct {
    int id;
    char* name;  // Dynamically allocated string
    int age;
} Person;

// Helper function prototypes
void print_person(void* data);
int compare_person_id(const void* a, const void* b);
int compare_person_name(const void* a, const void* b);
int compare_person_age(const void* a, const void* b);
void free_person(void* data);
void copy_person(void* dest, const void* src);
Person create_person(int id, const char* name, int age);
// Predicate for count_if (signature requires (element, arg))
bool is_adult_predicate(const void* element, void* arg);
// Simple filter function (signature requires only data)
bool is_adult(const void* data);
bool is_adult_filter(const void* data); // TODO: remove when replacing with is_adult usage
bool has_name(const void* element, void* arg);
void map_person_to_age(void* dest, const void* src);
void banner(const char* title);
void increment_age(void* dest, const void* src); // Map function to increment age (deep copy)

// Implementation of helper functions
void print_person(void* data) {
    const Person* p = (const Person*)data;
    printf("{ID:%04d, Name:\"%s\", Age:%d}", p->id, p->name, p->age);
}


int compare_person_id(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->id - p2->id);
}

int compare_person_name(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    if (!p1->name && !p2->name) return 0;
    if (!p1->name) return -1;
    if (!p2->name) return 1;
    return strcmp(p1->name, p2->name);
}

int compare_person_age(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->age - p2->age);
}

void free_person(void* data) {
    Person* p = (Person*)data;
    if (p && p->name) {
        free(p->name);
        p->name = NULL;
    }
}

void copy_person(void* dest, const void* src) {
    if (!dest || !src) return;
    Person* dest_p = (Person*)dest;
    const Person* src_p = (const Person*)src;
    
    dest_p->id = src_p->id;
    dest_p->age = src_p->age;
    
    if (src_p->name) {
        dest_p->name = malloc(strlen(src_p->name) + 1);
        if (dest_p->name) {
            strcpy(dest_p->name, src_p->name);
        }
    } else {
        dest_p->name = NULL;
    }
}

Person create_person(int id, const char* name, int age) {
    Person p;
    p.id = id;
    p.age = age;
    p.name = malloc(strlen(name) + 1);
    if (p.name) strcpy(p.name, name);
    return p;
}

bool is_adult_predicate(const void* element, void* arg) {
    (void)arg; // Unused parameter
    const Person* p = (const Person*)element;
    return p->age >= 18;
}

bool is_adult(const void* data) {
    const Person* p = (const Person*)data;
    return p->age > 18; // strictly older than 18 per docs example
}

bool is_adult_filter(const void* data) {
    const Person* p = (const Person*)data;
    return p->age >= 18;
}

bool has_name(const void* element, void* arg) {
    const Person* p = (const Person*)element;
    const char* target_name = (const char*)arg;
    if (!p->name || !target_name) return false;
    return strcmp(p->name, target_name) == 0;
}

void map_person_to_age(void* dest, const void* src) {
    const Person* p = (const Person*)src;
    *(int*)dest = p->age;
}

void increment_age(void* dest, const void* src) {
    const Person* src_p = (const Person*)src;
    Person* dest_p = (Person*)dest;
    dest_p->id = src_p->id;
    dest_p->name = src_p->name ? strdup(src_p->name) : NULL; // Deep copy name (safe if NULL)
    dest_p->age = src_p->age + 1;
}

void banner(const char* title) {
    printf("\n\n============= %s =============\n", title);
}

// Predicate: returns true if Person's name contains 'E' or 'e'
bool is_person_name_contains_e(const void* element) {
    const Person* p = (const Person*)element;
    for (const char* c = p->name; *c; ++c) {
        if (*c == 'E' || *c == 'e') return true;
    }
    return false;
}

// Predicate: returns true if Person's id divisible by 2
bool is_person_id_divisible_by_2(const void* element) {
    const Person* p = (const Person*)element;
    return (p->id % 2) == 0;
}

// Main demo function for Person structures
int main(void) {

    printf("Generic Linked List Library - Person Demo\n");
    printf("==========================================\n");
    
    ///////
    // 1 //
    /////// 
    banner("1. Create List");
    printf("Creating a new linked list for Person structures...\n");

    LinkedList* people_list = create_list(sizeof(Person));
    if (!people_list) {
        printf("Error: Failed to create list!\n");
        return 1;
    }
    printf("✓ List created successfully\n");
    
    ///////
    // 2 //
    ///////
    banner("2. List Configuration");
    printf("Configuring list with helper functions...\n");

    set_print_function(people_list, print_person);
    // compare_person_id used per-call now
    set_free_function(people_list, free_person);
    set_copy_function(people_list, copy_person);

    printf("✓ List configured with print, compare, free, and copy functions\n");
    
    ///////
    // 3 //
    ///////
    banner("3. Insertion in Linked List");

    printf("Creating people...\n");

    // Stack allocated Persons for value-mode (you keep your stack variable)
    Person alice = create_person(1012, "Alice Johnson", 16);
    Person bob = create_person(1010, "Bob Smith", 35);
    Person charlie = create_person(1098, "Charlie Brown", 11);

    // Heap allocated Persons for pointer-mode (ownership transfers to list; do not free manually)
    Person* diana = (Person*)malloc(sizeof(Person));
    Person* emily = (Person*)malloc(sizeof(Person));
    Person* frank = (Person*)malloc(sizeof(Person));
    if (!diana || !emily || !frank) {
        printf("Memory allocation failed for heap Persons.\n");
        return 1;
    }
    *diana = create_person(1017, "Diana Prince", 13);
    *emily = create_person(1042, "Emily Davis", 22);
    *frank = create_person(1033, "Frank Wilson", 31);
    
    printf("Demonstrating all 6 insertion combinations:\n\n");
    
    printf("1. insert_head_value(list, alice)\n");
    insert_head_value(people_list, alice);

    printf("2. insert_head_ptr(list, diana)\n");
    insert_head_ptr(people_list, diana);

    printf("3. insert_tail_value(list, bob)\n");
    insert_tail_value(people_list, bob);

    printf("4. insert_tail_ptr(list, emily)\n");
    insert_tail_ptr(people_list, emily);

    printf("5. insert_index_value(list, 1, charlie)\n");
    insert_index_value(people_list, 1, charlie);
    
    printf("6. insert_index_ptr(list, 3, frank)\n");
    insert_index_ptr(people_list, 3, frank);
    
    printf("\n✓ All 6 insertion methods demonstrated!\n");
    
    // Current list order:
    // Diana > Charlie > Alice > Frank > Bob > Emily

    ///////////////////////////////////////////////////////////////////////////////////
    //    NOTE: The deletion functions (section 4) are placed at the end of main     //
    // because I still want to work with the Person structures before removing them. //
    ///////////////////////////////////////////////////////////////////////////////////
    
    /*
    ///////
    // 5 //
    ///////
    banner("5. Utility Functions");
    
    printf("List length: %zu\n", get_length(people_list));
    printf("Is empty: %s\n", is_empty(people_list) ? "Yes" : "No");
    
    printf("Printing with different formats:\n");
    printf("With indices:\n");
    print_list(people_list);
    printf("Comma separated: ");
    print_list_advanced(people_list, false, ", ");
    
    ///////
    // 6 //
    ///////
    banner("6. Search and Access Functions");
    
    // Get element by index
    printf("Accessing element at index 1:\n");
    Person* person_at_1 = (Person*)get(people_list, 1);
    if (person_at_1) {
        printf("Person at index 1: ");
        print_person(person_at_1);
        printf("\n");
    }
    
    // Search for element
    printf("Searching for Bob by ID...\n");
    int bob_index = index_of(people_list, &bob);
    if (bob_index >= 0) {
        printf("Bob found at index: %d\n", bob_index);
    } else {
        printf("Bob not found (error code: %d)\n", bob_index);
    }
    
    // Advanced search from tail
    printf("Searching from tail for Charlie...\n");
    int charlie_index = index_of_advanced(people_list, &charlie, START_FROM_TAIL);
    printf("Charlie found at index: %d (searching from tail)\n", charlie_index);
    
    ///////
    // 7 //
    ///////
    banner("7. Sorting Functions");
    
    printf("Sorting by age (ascending)...\n");
    sort(people_list, false, compare_person_age);
    printf("List sorted by age (ascending):\n");
    print_list(people_list);
    
    printf("Sorting by age (descending)...\n");
    sort(people_list, true, compare_person_age);
    printf("List sorted by age (descending):\n");
    print_list(people_list);
    
    printf("Sorting by name (alphabetical)...\n");
    sort(people_list, false, compare_person_name);
    printf("List sorted by name (alphabetical):\n");
    print_list(people_list);
    
    // Restore ID comparison for other operations
    // restore comparator concept not needed now
    
    ///////
    // 8 //
    ///////
    banner("8. Structural Transformations");
    
    // Debug: Print list before copy
    printf("List before copy:\n");
    print_list(people_list);
    
    // Copy the list
    printf("Creating a copy of the list...\n");
    LinkedList* copy_list = copy(people_list); // copy unaffected
    if (copy_list) {
        printf("Copy created successfully. Length: %zu\n", get_length(copy_list));
    }
    
    // Debug: Print both lists after copy
    printf("Original list after copy:\n");
    print_list(people_list);
    printf("Copy list after copy:\n");
    print_list(copy_list);
    
    // Reverse the original list
    printf("Reversing the original list...\n");
    reverse(people_list);
    printf("List after reversal:\n");
    print_list(people_list);
    
    // Rotate the list
    printf("Rotating list by 2 positions to the right...\n");
    rotate(people_list, 2);
    printf("List after rotation:\n");
    print_list(people_list);
    
    // Filter adults only (age > 18) using new is_adult (FilterFunction signature)
    printf("Creating filtered list (adults only, age>18)...\n");
    LinkedList* adults_only = filter(people_list, is_adult);
    if (adults_only) {
        printf("Adults-only list (age>18):\n");
        print_list(adults_only);
    }
    
    // Map to ages
    printf("Creating mapped list (ages only)...\n");
    LinkedList* ages_list = map(people_list, map_person_to_age, sizeof(int));
    if (ages_list) {
        set_print_function(ages_list, print_int);
        printf("Ages-only list: ");
        print_list_advanced(ages_list, false, ", ");
        destroy(ages_list);
    }

    // Map to older people (age + 1) demonstrating deep copy of name
    printf("Creating mapped list (older people, age+1)...\n");
    LinkedList* older_people = map(people_list, increment_age, sizeof(Person));
    if (older_people) {
        // Configure for printing & memory management
        set_print_function(older_people, print_person);
        set_free_function(older_people, free_person);
        set_copy_function(older_people, copy_person);
        printf("Older people list (age+1):\n");
        print_list(older_people);
    }
    
    ///////
    // 9 //
    ///////
    banner("9. Mathematical Functions");
    
    // Find min and max by age
    Person* youngest = (Person*)min_by(people_list, compare_person_age);
    Person* oldest = (Person*)max_by(people_list, compare_person_age);
    
    if (youngest) {
        printf("Youngest person: ");
        print_person(youngest);
        printf("\n");
    }
    if (oldest) {
        printf("Oldest person: ");
        print_person(oldest);
        printf("\n");
    }
    
    // Find min and max by name (alphabetical)
    Person* first_alphabetical = (Person*)min_by(people_list, compare_person_name);
    Person* last_alphabetical = (Person*)max_by(people_list, compare_person_name);
    
    if (first_alphabetical) {
        printf("First alphabetically: ");
        print_person(first_alphabetical);
        printf("\n");
    }
    if (last_alphabetical) {
        printf("Last alphabetically: ");
        print_person(last_alphabetical);
        printf("\n");
    }
    
    // Count adults (age >= 18)
    size_t adult_count = count_if(people_list, is_adult_predicate, NULL);
    printf("Number of adults (age >= 18): %zu\n", adult_count);
    
    // Count people named "Alice"
    char* alice_name = "Alice Johnson";
    size_t alice_count = count_if(people_list, has_name, alice_name);
    printf("Number of people named '%s': %zu\n", alice_name, alice_count);
    
    ////////
    // 10 //
    ////////
    banner("10. List <-> Array Conversion");
    printf("Demonstrating from_array() and to_array() with integers...\n");

    LinkedList* numbers_list = create_list(sizeof(int));
    if (numbers_list) {
        set_print_function(numbers_list, print_int);
        int nums[] = {10, 20, 30, 40, 50};
        printf("Filling numbers_list from C array {10,20,30,40,50}...\n");
        ListResult fr = from_array(numbers_list, nums, 5);
        printf("from_array result: %s\n", error_string(fr));
        printf("numbers_list contents: ");
        print_list_advanced(numbers_list, false, ", ");

        size_t out_n = 0;
        int* back_arr = (int*)to_array(numbers_list, &out_n);
        if (back_arr) {
            printf("Converted back to C array (%zu elements): ", out_n);
            for (size_t i = 0; i < out_n; ++i) {
                printf("%d%s", back_arr[i], (i + 1 < out_n) ? ", " : "\n");
            }
            free(back_arr);
        }
    }

    ////////
    // 11 //
    ////////
    banner("11. List <-> String & File I/O");
    printf("Using to_string() on numbers_list (separator=|)...\n");
    char* numbers_str = NULL;
    if (numbers_list) {
        numbers_str = to_string(numbers_list, " | ");
        if (numbers_str) {
            printf("numbers_list as string: %s\n", numbers_str);
        } else {
            printf("to_string failed (maybe empty list or no print function).\n");
        }
    }

    printf("Saving numbers_list to text file 'numbers.txt'...\n");
    LinkedList* loaded_numbers = NULL;
    if (numbers_list) {
    ListResult save_r = save_to_file(numbers_list, "numbers.txt", FILE_FORMAT_TEXT, "\n");
    printf("save_to_file (TEXT) result: %s\n", error_string(save_r));
        printf("Reloading from 'numbers.txt' (text)...\n");
    loaded_numbers = load_from_file("numbers.txt", sizeof(int), FILE_FORMAT_TEXT, "\n", print_int, NULL, NULL, NULL);
        if (loaded_numbers) {
            printf("Loaded list (text): ");
            print_list_advanced(loaded_numbers, false, ", ");
        } else {
            printf("Failed to load numbers.txt\n");
        }
    }
    printf("(Keeping numbers.txt so you can open it)\n");
    
    ///////
    // 4 //
    ///////
    
    // Delete from head
    printf("Deleting from head...\n");
    delete_head(people_list);

    // Delete from tail
    printf("Deleting from tail...\n");
    delete_tail(people_list);

    // Delete at specific index
    printf("Deleting at index 2...\n");
    delete_index(people_list, 2);

    printf("Deleting all people whose name contains 'E' or 'e'...\n");
    remove_advanced(people_list, DELETE_ALL_OCCURRENCES, START_FROM_HEAD, is_person_name_contains_e);    

    printf("Deleting last three persons (from end) whose ID divisible by 2...\n");
    remove_advanced(people_list, 3, START_FROM_TAIL, is_person_id_divisible_by_2);

    printf("Deleting all remaining persons...\n");
    clear(people_list);

    printf("Deleting the entire list...\n");
    destroy(people_list);
    
    people_list = NULL; // Good practice to avoid dangling pointer...



    */
    return 0;
}
