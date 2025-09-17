
4. `void CopyFunction(void* dest, const void* src)` - This function performs a "deep copy" of elements. When you insert data, the list makes a copy. By default, this is a `memcpy` (a shallow copy). If your struct contains pointers, a shallow copy would mean both the original and the list's copy point to the same memory, which is dangerous. A deep copy function allocates new memory for these pointers. לרוב זה מיותר, כי אם נבצע שינוי במקור אנחנו נרצה שזה ישתנה. אבל אם לא, אז תוכלו להגדיר את זה. זה לא חובה והספריה תעבוד גם בלי זה.

```c
// 4. Function to perform a deep copy of a Person struct
void copy_person(void* dest, const void* src) {
    if (!dest || !src) return;
    Person* dest_p = (Person*)dest;
    const Person* src_p = (const Person*)src;

    // Allocate new memory for the name and copy it
    dest_p->name = malloc(strlen(src_p->name) + 1);
    if (dest_p->name) {
        strcpy(dest_p->name, src_p->name);
    }
    dest_p->age = src_p->age;
}
```

If you want to filter or transform the elements in the list, you can define additional function pointers:

5. `bool FilterFunction(const void* data)` – Returns `TRUE` to keep the element, `FALSE` to discard it.
6. `void MapFunction(void* dest, const void* src)` – Transforms an element from the source to the destination.

For example, with our `Person` struct:

```c
// 5. Filter function: returns TRUE if age > 18
bool is_adult(const void* data) {
    const Person* p = (const Person*)data;
    return p->age > 18;
}

// Usage:
LinkedList* adults = list_filter(person_list, is_adult);
// 'adults' now contains only Persons older than 18
```

```c
// Map function: adds 1 to the age
void increment_age(void* dest, const void* src) {
    const Person* src_p = (const Person*)src;
    Person* dest_p = (Person*)dest;
    dest_p->id = src_p->id;
    dest_p->name = strdup(src_p->name); // Deep copy name
    dest_p->age = src_p->age + 1;
}

// Usage:
LinkedList* older_people = list_map(person_list, increment_age, sizeof(Person));
// 'older_people' contains Persons with age increased by 1
```

