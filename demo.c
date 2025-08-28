#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a Person struct for demonstration
typedef struct {
    int id;
    char name[50];
    int age;
} Person;

// Function to print a Person
void print_person(void* data) {
    const Person* p = (const Person*)data;
    if (p) {
        printf("{ID: %d, Name: \"%s\", Age: %d}", p->id, p->name, p->age);
    }
}

// Function to compare two Persons by ID (for general list operations)
int compare_person_id(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return p1->id - p2->id;
}

// Function to copy a Person
void copy_person(void* dest, const void* src) {
    memcpy(dest, src, sizeof(Person));
}

// --- New Generic Helper Functions ---

// Predicate function for list_count_if: checks if a person has a specific name
bool has_name(const void *element, void *arg) {
    const Person *p = (const Person*)element;
    const char *target_name = (const char*)arg;
    return strcmp(p->name, target_name) == 0;
}

// Comparison function for list_min_by/list_max_by: compares persons by age
int compare_age(const void *a, const void *b) {
    const Person *pa = (const Person*)a;
    const Person *pb = (const Person*)b;
    return pa->age - pb->age; // a < b -> negative, a > b -> positive
}

// Comparison function for list_min_by/list_max_by: compares persons by name length
int compare_name_length(const void *a, const void *b) {
    const Person *pa = (const Person*)a;
    const Person *pb = (const Person*)b;
    return (int)strlen(pa->name) - (int)strlen(pb->name);
}

// Comparison function for list_unique_advanced: compares persons by name
int compare_person_name(const void *a, const void *b) {
    const Person *pa = (const Person*)a;
    const Person *pb = (const Person*)b;
    return strcmp(pa->name, pb->name);
}

// Predicate function for list_count_if: checks if a person is a teenager
bool is_teenager(const void *element, void *arg) {
    const Person *p = (const Person*)element;
    (void)arg; // Unused argument
    return p->age >= 13 && p->age <= 18;
}

// Helper function to create and initialize a Person struct
Person create_person(int id, const char* name, int age) {
    Person p;
    p.id = id;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0'; // Ensure null-termination
    p.age = age;
    return p;
}

// Helper function to print separator
void print_separator(const char* title) {
    printf("\n======== %s ========\n", title);
}

void demonstrate_generic_functions() {
    print_separator("DEMONSTRATING GENERIC COUNT/MIN/MAX");

    LinkedList* people_list = list_create(sizeof(Person));
    list_set_print_function(people_list, print_person);
    list_set_compare_function(people_list, compare_person_id);
    list_set_copy_function(people_list, copy_person);

    // Create and add people to the list
    Person p1 = create_person(1, "Alice", 30);
    Person p2 = create_person(2, "Bob", 25);
    Person p3 = create_person(3, "Charlie", 35);
    Person p4 = create_person(4, "Alice", 40);
    Person p5 = create_person(5, "David", 15);
    Person p6 = create_person(6, "Eve", 18);

    list_insert_at_tail(people_list, &p1);
    list_insert_at_tail(people_list, &p2);
    list_insert_at_tail(people_list, &p3);
    list_insert_at_tail(people_list, &p4);
    list_insert_at_tail(people_list, &p5);
    list_insert_at_tail(people_list, &p6);

    printf("Initial list of people:\n");
    list_print(people_list);

    // --- Demonstrate list_count_if ---
    printf("\n--- Using list_count_if ---\n");
    char* name_to_count = "Alice";
    size_t count = list_count_if(people_list, has_name, name_to_count);
    printf("Number of people named '%s': %zu\n", name_to_count, count);

    size_t teenager_count = list_count_if(people_list, is_teenager, NULL);
    printf("Number of teenagers (13-18): %zu\n", teenager_count);

    
    // --- Demonstrate list_unique_advanced ---
    print_separator("DEMONSTRATING UNIQUE (ADVANCED)");
    
    // Add a duplicate person to test uniqueness
    Person p7 = create_person(7, "Bob", 26);
    list_insert_at_tail(people_list, &p7);
    printf("\nList with a duplicate Bob:\n");
    list_print(people_list);

    // Unique by name, preserving first
    LinkedList* unique_names_first = list_unique_advanced(people_list, compare_person_name, START_FROM_HEAD);
    printf("\nUnique names (preserving first occurrence):\n");
    list_print(unique_names_first);
    list_destroy(unique_names_first);

    // Unique by name, preserving last
    LinkedList* unique_names_last = list_unique_advanced(people_list, compare_person_name, START_FROM_TAIL);
    printf("\nUnique names (preserving last occurrence):\n");
    list_print(unique_names_last);
    list_destroy(unique_names_last);


    // --- Demonstrate list_min_by ---
    printf("\n--- Using list_min_by ---\n");
    Person* youngest = (Person*)list_min_by(people_list, compare_age);
    if (youngest) {
        printf("The youngest person is: ");
        print_person(youngest);
        printf("\n");
    }

    // --- Demonstrate list_max_by ---
    printf("\n--- Using list_max_by ---\n");
    
    Person* longest_name_person = (Person*)list_max_by(people_list, compare_name_length);
    if (longest_name_person) {
        printf("The person with the longest name is: ");
        print_person(longest_name_person);
        printf("\n");
    }

    list_destroy(people_list);
}

int main() {
    demonstrate_generic_functions();
    return 0;
}