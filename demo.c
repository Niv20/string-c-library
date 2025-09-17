#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
void print_int(void* d);

// Function declarations for heap-based Person
void print_person(void* data);
int compare_person_id(const void* a, const void* b);
int compare_person_name(const void* a, const void* b);
int compare_age(const void* a, const void* b);
void free_person(void* data);
bool has_name(const void *element, void *arg);
bool is_teenager(const void *element, void *arg);
bool is_adult_filter(const void* data);
void map_person_to_age(void* dest, const void* src);
Person create_person(int id, const char* name, int age);
int compare_name_len(const void* a, const void* b);
void copy_person(void* dest, const void* src);

// -------------------- Demo domain type --------------------
typedef struct {
    int id;            
    char name;
    int age;   
} Person;

// Print function for Person
void print_person(void* data) {
    const Person* p = (const Person*)data;
    if (p && p->name)
        printf("{ID:%04d, Name:\"%s\", Age:%d}", p->id, p->name, p->age);
    else if (p)
        printf("{ID:%04d, Name:NULL, Age:%d}", p->id, p->age);
}

// Compare two Persons by ID
int compare_person_id(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return (p1->id - p2->id);
}

// Compare two Persons by name
int compare_person_name(const void* a, const void* b) {
    const Person* pa = (const Person*)a;
    const Person* A = (const Person*)a;
    const Person* B = (const Person*)b;
    if (!A->name && !B->name) return 0;
    if (!A->name) return -1;
    if (!B->name) return 1;
    return strcmp(A->name, B->name);
    const Person* pa = (const Person*)a;
    const Person* pb = (const Person*)b;
    return (pa->age - pb->age);
}

// Free function for Person (no-op – struct holds no heap members)
void free_person(void* data) {
    (void)data; // nothing to free inside the Person itself
}

// Helpers used by math/filter/map demos
bool has_name(const void *element, void *arg) {
    const Person *p = (const Person*)element;
    const char *target_name = (const char*)arg;
    return strcmp(p->name, target_name) == 0;
}

bool is_teenager(const void *element, void *arg) {
    (void)arg;
    const Person *p = (const Person*)element;
    return p->age >= 13 && p->age <= 18;
}

bool is_adult_filter(const void* data) {
    const Person* p = (const Person*)data;
    return p->age >= 18;
}

void map_person_to_age(void* dest, const void* src) {
    const Person* p = (const Person*)src;
    *(int*)dest = p->age;
}

// Create a person
Person create_person(int id, const char* name, int age) {
    Person p;
    p.id = id;
    p.age = age;
    if (name) {
        p.name = malloc(strlen(name) + 1);
        if (p.name) strcpy(p.name, name);
    } else {
        p.name = NULL;
    }
    return p;
}

// Pretty section header
static void banner(const char* title) { printf("\n========== %s ==========%s", title, "\n"); }

// Helper functions for various demos
int compare_name_len(const void* a, const void* b) {
    const Person* A = (const Person*)a;
    const Person* B = (const Person*)b;
    if (!A->name && !B->name) return 0;
    if (!A->name) return -1;
    if (!B->name) return 1;
    return (int)strlen(A->name) - (int)strlen(B->name);
}

int cmp_int(const void* A, const void* B) { 
    return (*(const int*)A - *(const int*)B); 
}

void free_int(void* d) { 
    (void)d; 
}

void demonstrate_generic_functions() {
    banner("DEMONSTRATING GENERIC COUNT/MIN/MAX");

    LinkedList* people_list = list_create(sizeof(Person));
    list_set_print_function(people_list, print_person);
    list_set_compare_function(people_list, compare_person_id);
    list_set_copy_function(people_list, copy_person);

    Person p1 = create_person(1001, "Alice", 30);
    Person p2 = create_person(1002, "Bob", 25);
    Person p3 = create_person(1003, "Charlie", 35);
    Person p4 = create_person(1004, "Alice", 40);
    Person p5 = create_person(1005, "David", 15);
    Person p6 = create_person(1006, "Eve", 18);

    list_insert_at_tail(people_list, &p1);
    list_insert_at_tail(people_list, &p2);
    list_insert_at_tail(people_list, &p3);
    list_insert_at_tail(people_list, &p4);
    list_insert_at_tail(people_list, &p5);
    list_insert_at_tail(people_list, &p6);

    printf("Initial list of people:\n");
    list_print(people_list);

    printf("\n--- Using list_count_if ---\n");
    char* name_to_count = "Alice";
    size_t count = list_count_if(people_list, has_name, name_to_count);
    printf("Number of people named '%s': %zu\n", name_to_count, count);

    size_t teenager_count = list_count_if(people_list, is_teenager, NULL);
    printf("Number of teenagers (13-18): %zu\n", teenager_count);

    banner("DEMONSTRATING UNIQUE (ADVANCED)");
    Person p7 = create_person(2329, "Bob", 26);
    list_insert_at_tail(people_list, &p7);
    printf("\nList with a duplicate Bob:\n");
    list_print(people_list);

    LinkedList* unique_names_first = list_unique_advanced(people_list, compare_person_name, START_FROM_HEAD);
    printf("\nUnique names (preserving first occurrence):\n");
    list_print(unique_names_first);
    list_destroy(unique_names_first);

    LinkedList* unique_names_last = list_unique_advanced(people_list, compare_person_name, START_FROM_TAIL);
    printf("\nUnique names (preserving last occurrence):\n");
    list_print(unique_names_last);
    list_destroy(unique_names_last);

    printf("\n--- Using list_min_by ---\n");
    Person* youngest = (Person*)list_min_by(people_list, compare_age);
    if (youngest) { printf("The youngest person is: "); print_person(youngest); printf("\n"); }

    printf("\n--- Using list_max_by ---\n");
    // Use the global compare_name_len function
    Person* longest_name_person = (Person*)list_max_by(people_list, compare_name_len);
    if (longest_name_person) { printf("The person with the longest name is: "); print_person(longest_name_person); printf("\n"); }

    list_destroy(people_list);
}

// -------------------- FULL ORDERED DEMO --------------------

/// Show a human-friendly error string for a few codes
static void demo_error_handling(void) {
    banner("Error Handling");
        char* name;
    printf("LIST_ERROR_INDEX_OUT_OF_BOUNDS => %s\n", list_error_string(LIST_ERROR_INDEX_OUT_OF_BOUNDS));
}

/// Create and configure a list of Person with all setters
static LinkedList* demo_lifecycle_and_setters(void) {
    banner("Create & Configure List");
    LinkedList* L = list_create(sizeof(Person));
    list_set_print_function(L, print_person);
    list_set_compare_function(L, compare_person_id);
    list_set_free_function(L, free_person);   // required by list_set()
    list_set_copy_function(L, copy_person);
    printf("Created list (empty=%s, len=%zu)\n", list_is_empty(L)?"true":"false", list_get_length(L));
    return L;

// Demonstrate size cap and overwrite behavior
static void demo_size_and_overwrite(LinkedList* L) {
    banner("Size & Overwrite");
    list_set_max_size(L, 3, DELETE_OLD_WHEN_FULL); // FIFO when full
    Person a = create_person(2244, "Adam", 22);
    Person b = create_person(2245, "Bella", 31);
    Person c = create_person(2246, "Carl", 28);
    Person d = create_person(2247, "Dana", 26);
    list_insert_at_tail(L, &a);
    list_insert_at_tail(L, &b);
    list_insert_at_tail(L, &c);
    printf("After 3 inserts (cap=3):\n"); list_print(L);
    list_insert_at_tail(L, &d); // forces removal of the oldest (Adam)
    printf("After pushing when full (FIFO remove oldest):\n"); list_print(L);
        if (!p->name || !target_name) return false;
        return strcmp(p->name, target_name) == 0;
    list_set_max_size(L, UNLIMITED, REJECT_NEW_WHEN_FULL);
}

/// Demonstrate insertion functions
static void demo_insertions(LinkedList* L) {
    banner("Insertions");
    Person e = create_person(2248, "Eden", 24);
        if (name) {
            p.name = malloc(strlen(name) + 1);
            if (p.name) strcpy(p.name, name);
        } else {
            p.name = NULL;
        }
    list_insert_at_head(L, &e);
    list_insert_at_tail(L, &f);
    list_insert_at_index(L, 1, &g);
    list_print(L);
}

/// Demonstrate deletions & advanced remove
    banner("Deletions");
    // Delete at head and tail (safe guards inside if empty)
    list_delete_from_head(L);
    list_delete_from_tail(L);
    // Insert duplicates for remove_advanced
    Person b = create_person(7434, "Bella", 31);
    Person b2 = create_person(7435, "Bella", 31);
    list_insert_at_tail(L, &b);
    list_insert_at_tail(L, &b2);
    printf("Before remove_advanced:\n"); list_print(L);
    // Remove only one occurrence (from tail direction)
    list_set_compare_function(L, compare_person_name);
    list_remove_advanced(L, &b, 1, START_FROM_TAIL);
    printf("After remove one 'Bella' from tail:\n"); list_print(L);
    // Remove all remaining 'Bella'
    list_remove_advanced(L, &b, DELETE_ALL_OCCURRENCES, START_FROM_HEAD);
    printf("After remove all 'Bella':\n"); list_print(L);
    // Restore default comparator (by id) for later operations
    list_set_compare_function(L, compare_person_id);
    // Delete at index if possible
    if (list_get_length(L) > 0) list_delete_at_index(L, 0);
    printf("After delete_at_index(0):\n"); list_print(L);
}

/// Demonstrate utilities: length/empty, print advanced
static void demo_utilities(LinkedList* L) {
    banner("Utilities");
    printf("Empty? %s, Length=%zu\n", list_is_empty(L)?"true":"false", list_get_length(L));
    printf("Print advanced (comma separated, hide indices):\n");
    list_print_advanced(L, false, ", ");
}

/// Demonstrate access/search
static void demo_access_and_search(LinkedList* L) {
    banner("Search & Access");
    // Ensure some content
    if (list_is_empty(L)) {
        Person x = create_person(9494, "Xena", 29);
        Person y = create_person(9495, "Yossi", 17);
        list_insert_at_tail(L, &x);
        list_insert_at_tail(L, &y);
    }
    list_print(L);
    // list_get
    void* first = list_get(L, 0);
    if (first) { printf("First element: "); print_person(first); printf("\n"); }
    // list_set (requires free function to exist; we set one earlier)
    Person repl = create_person(5354, "Replaced", 45);
    ListResult r = list_set(L, 0, &repl);
    printf("list_set at index 0 => %s\n", list_error_string(r));
    list_print(L);
    // list_index + list_index_advanced
    Person probe = *(Person*)list_get(L, 0);
    int idx_head = list_index(L, &probe);
    int idx_tail = list_index_advanced(L, &probe, START_FROM_TAIL);
    printf("index(head)=%d, index(tail)=%d for first element copy\n", idx_head, idx_tail);
}

/// Demonstrate sorting
static void demo_sort(LinkedList* L) {
    banner("Sorting");
    list_set_compare_function(L, compare_age); // sort by age
    list_sort(L, false);
    printf("Sorted by age ascending:\n"); list_print(L);
    list_sort(L, true);
    printf("Sorted by age descending:\n"); list_print(L);
    // Restore comparator by id for later operations that expect it
    list_set_compare_function(L, compare_person_id);
}

/// Demonstrate copy/extend/concat/slice/rotate/reverse/filter/map
static void demo_structural_ops_and_transform(LinkedList* L) {
    banner("Copy / Extend / Concat / Slice / Rotate / Reverse / Filter / Map");
    // Ensure L has a few items
    if (list_get_length(L) < 3) {
        Person a = create_person(8888, "Ana", 20);
        Person b = create_person(8889, "Ben", 35);
        Person c = create_person(8890, "Cam", 16);
        list_insert_at_tail(L, &a); list_insert_at_tail(L, &b); list_insert_at_tail(L, &c);
    }
    printf("Base list L:\n"); list_print(L);

    // list_copy
    LinkedList* A = list_copy(L);
    printf("\nCopy A of L:\n"); list_print(A);

    // list_extend (append L into A)
    list_extend(A, L);
    printf("After extend(A, L):\n"); list_print(A);

    // list_concat
    LinkedList* C = list_concat(L, A);
    printf("\nConcat(L, A) => C:\n"); list_print(C);

    // list_slice
    LinkedList* S = list_slice(C, 1, 4);
    printf("\nSlice C[1:4] => S:\n"); list_print(S);

    // list_rotate (right by 2)
    list_rotate(S, 2);
    printf("After rotate(S, 2):\n"); list_print(S);

    // list_reverse
    list_reverse(S);
    printf("After reverse(S):\n"); list_print(S);

    // list_filter (adults only)
    LinkedList* F = list_filter(S, is_adult_filter);
    printf("\nFilter adults from S => F:\n"); list_print(F);

    // list_map (to ages array list of ints)
    LinkedList* M = list_map(F, map_person_to_age, sizeof(int));
    printf("Map F -> ages (int list). Will print with indices as raw ints are handled by list_to_string only; so here we show length: %zu\n", list_get_length(M));

    // Cleanup of temporaries
    list_destroy(A); list_destroy(C); list_destroy(S); list_destroy(F); list_destroy(M);
}

/// Demonstrate math & set-like ops
static void demo_math_and_sets(LinkedList* L) {
    banner("Math & Set-like Functions");
    size_t n_alice = list_count_if(L, has_name, "Alice");
    printf("count_if name==Alice => %zu\n", n_alice);

    Person* min_age = (Person*)list_min_by(L, compare_age);
    Person* max_age = (Person*)list_max_by(L, compare_age);
    if (min_age) { printf("Min age: "); print_person(min_age); printf("\n"); }
    if (max_age) { printf("Max age: "); print_person(max_age); printf("\n"); }

    // unique / unique_advanced
    LinkedList* by_name_first = list_unique_advanced(L, compare_person_name, START_FROM_HEAD);
    printf("Unique by name (keep first):\n"); list_print(by_name_first);
    LinkedList* by_name_last  = list_unique_advanced(L, compare_person_name, START_FROM_TAIL);
    printf("Unique by name (keep last):\n"); list_print(by_name_last);

    // intersection/union (use two lists with some overlaps by name)
    LinkedList* L2 = list_create(sizeof(Person));
    list_set_print_function(L2, print_person);
    list_set_compare_function(L2, compare_person_name);
    list_set_free_function(L2, free_person);
    list_set_copy_function(L2, copy_person);
    Person a = create_person(4444, "Alice", 21);
    Person z = create_person(4445, "Ziv", 42);
    list_insert_at_tail(L2, &a); list_insert_at_tail(L2, &z);

    // For set ops we rely on compare function (here by name)
    list_set_compare_function(L, compare_person_name);
    LinkedList* inter = list_intersection(L, L2);
    LinkedList* uni   = list_union(L, L2);
    printf("Intersection by name with L2:\n"); list_print(inter);
    printf("Union by name with L2:\n"); list_print(uni);
    // Restore comparator on L
    list_set_compare_function(L, compare_person_id);

    list_destroy(by_name_first); list_destroy(by_name_last);
    list_destroy(inter); list_destroy(uni); list_destroy(L2);
}

/// Demonstrate list<->array and list_to_string, save/load
static void demo_array_and_io(void) {
    banner("Array / String / File I/O");

    // Work with an int list to showcase list_to_string formatting
    LinkedList* ints = list_create(sizeof(int));
    // For basic ints we don't need custom functions, but printing requires a print function
    list_set_print_function(ints, print_int);
    list_set_compare_function(ints, cmp_int);
    list_set_free_function(ints, free_int);

    // array_to_list
    int arr[] = {5, 2, 9, 1, 5};
    array_to_list(ints, arr, sizeof(arr)/sizeof(arr[0]));
    printf("Int list from array: \n"); list_print(ints);

    // list_to_array
    size_t out_sz = 0; int* back = (int*)list_to_array(ints, &out_sz);
    printf("list_to_array => size=%zu, first=%d\n", out_sz, out_sz?back[0]:-1);

    // list_to_string
    char* s = list_to_string(ints, ",");
    printf("list_to_string(ints, ',') => %s\n", s ? s : "(null)");

    // Save/load Persons to file
    LinkedList* ppl = list_create(sizeof(Person));
    list_set_print_function(ppl, print_person);
    list_set_compare_function(ppl, compare_person_id);
    list_set_free_function(ppl, free_person);
    list_set_copy_function(ppl, copy_person);
    Person pA = create_person(2341, "FileA", 20);
    Person pB = create_person(2342, "FileB", 30);
    list_insert_at_tail(ppl, &pA); list_insert_at_tail(ppl, &pB);
    list_save_to_file(ppl, "people.bin");

    LinkedList* loaded = list_load_from_file("people.bin", sizeof(Person), print_person, compare_person_id, free_person, copy_person);
    printf("Loaded from file: \n"); list_print(loaded);

    // Cleanup allocations
    free(s); free(back);
    list_destroy(ints); list_destroy(ppl); list_destroy(loaded);
}

/// Run the full ordered demo end-to-end
static void run_full_demo(void) {
    LinkedList* L = demo_lifecycle_and_setters();
    demo_error_handling();
    demo_size_and_overwrite(L);
    demo_insertions(L);
    demo_deletions(L);
    demo_utilities(L);
    demo_access_and_search(L);
    demo_sort(L);
    demo_structural_ops_and_transform(L);
    demo_math_and_sets(L);
    list_destroy(L);

    demo_array_and_io();
}

// -------------------- main --------------------
int main(void) {

    demonstrate_generic_functions();

    run_full_demo();

    return 0;
}