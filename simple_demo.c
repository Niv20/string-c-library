#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

// פונקציית print לmצביעים לint
void print_int_ptr(void* data) {
    int* ptr = *(int**)data;
    printf("%d", *ptr);
}

// פונקציית compare למצביעים לint
int compare_int_ptr(const void* a, const void* b) {
    int* ptr_a = *(int**)a;
    int* ptr_b = *(int**)b;
    return (*ptr_a > *ptr_b) - (*ptr_a < *ptr_b);
}

// פונקציית free למצביעים לint
void free_int_ptr(void* data) {
    int* ptr = *(int**)data;
    free(ptr);
}

int main() {
    printf("=== דמו של list_create החדש ===\n\n");
    
    // יצירת רשימה חדשה עם element_size בלבד
    LinkedList* list = list_create(sizeof(void*));
    if (!list) {
        printf("שגיאה ביצירת הרשימה\n");
        return 1;
    }
    
    printf("רשימה נוצרה בהצלחה!\n");
    printf("גודל אלמנט: %zu\n", list->element_size);
    printf("מצב אחסון: %s\n", list->stores_pointers ? "מצביעים" : "ערכים");
    printf("בעלות על נתונים: %s\n", list->owns_data ? "כן" : "לא");
    
    // הגדרת פונקציות עזר
    list_set_print_function(list, print_int_ptr);
    list_set_compare_function(list, compare_int_ptr);
    list_set_free_function(list, free_int_ptr);
    
    printf("\nפונקציות עזר הוגדרו בהצלחה!\n");
    
    // הוספת מספרים לרשימה
    for (int i = 1; i <= 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i * 10;
        list_append(list, num);
    }
    
    printf("\nרשימה לאחר הוספת ערכים:\n");
    list_print(list);
    
    // הגדרת גבלים לרשימה
    printf("\nהגדרת גבול מקסימלי של 3 אלמנטים עם אפשרות החלפה:\n");
    list_set_max_size(list, 3, true);
    
    // הוספת ערך נוסף (יגרום להסרת הראשון)
    int* num = malloc(sizeof(int));
    *num = 60;
    list_append(list, num);
    
    printf("רשימה לאחר הוספת ערך נוסף:\n");
    list_print(list);
    
    // שינוי לאחסון ערכים במקום מצביעים
    printf("\nשינוי לאחסון ערכים (value-based):\n");
    list_set_storage_mode(list, false, false);
    list_set_element_size(list, sizeof(int));
    
    printf("מצב אחסון חדש: %s\n", list->stores_pointers ? "מצביעים" : "ערכים");
    printf("גודל אלמנט חדש: %zu\n", list->element_size);
    
    // שחרור זיכרון
    list_destroy(list);
    printf("\nהרשימה שוחררה בהצלחה!\n");
    
    return 0;
}
