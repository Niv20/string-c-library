גיבוי 


/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃     Helper Functions for Basic Data Types     ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// INTERNAL HELPER to set all function pointers for a list.
static void list_set_all_functions(LinkedList* list, PrintFunction print_fn, 
                                   CompareFunction compare_fn, FreeFunction free_fn, 
                                   CopyFunction copy_fn) {
    if (!list) return;
    list_set_print_function(list, print_fn);
    list_set_compare_function(list, compare_fn);
    list_set_free_function(list, free_fn);
    list_set_copy_function(list, copy_fn);
}

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
 * @brief Creates a new integer list (value-based for basic types).
 * @return A new LinkedList configured for integers, or NULL on failure.
 */
LinkedList* list_create_int(void) {
    LinkedList* list = list_create(sizeof(int));
    if (!list) return NULL;
    
    list_set_all_functions(list, print_int, compare_int, NULL, NULL);
    
    return list;
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
 * @brief Creates a new double list (value-based for basic types).
 * @return A new LinkedList configured for doubles, or NULL on failure.
 */
LinkedList* list_create_double(void) {
    LinkedList* list = list_create(sizeof(double));
    if (!list) return NULL;
    
    list_set_all_functions(list, print_double, compare_double, NULL, NULL);
    
    return list;
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
 * @brief Creates a new character list (value-based for basic types).
 * @return A new LinkedList configured for characters, or NULL on failure.
 */
LinkedList* list_create_char(void) {
    LinkedList* list = list_create(sizeof(char));
    if (!list) return NULL;
    
    list_set_all_functions(list, print_char, compare_char, NULL, NULL);
    
    return list;
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
 * @brief Creates a new string list (value-based with special string handling).
 * @return A new LinkedList configured for strings, or NULL on failure.
 */
LinkedList* list_create_string(void) {
    LinkedList* list = list_create(sizeof(char*));
    if (!list) return NULL;
    
    list_set_all_functions(list, print_string_builtin, compare_string_builtin, free_string_builtin, copy_string_builtin);
    
    return list;
}

/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                               ┃
┃          Convenient Wrapper Functions         ┃
┃        (for easy use with basic types)        ┃
┃                                               ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */

// =========== INTEGER LIST WRAPPERS ===========

/**
 * @brief Adds an integer value to the tail of an integer list.
 * @param list The integer list to add to.
 * @param value The integer value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_int(LinkedList* list, int value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_tail(list, &value);
}

/**
 * @brief Adds an integer value to the head of an integer list.
 * @param list The integer list to add to.
 * @param value The integer value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_int_to_head(LinkedList* list, int value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_head(list, &value);
}

/**
 * @brief Inserts an integer value at a specific index in an integer list.
 * @param list The integer list to insert into.
 * @param index The index to insert at.
 * @param value The integer value to insert.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_int_at(LinkedList* list, size_t index, int value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_index(list, index, &value);
}

/**
 * @brief Gets an integer value from a specific index (safe access).
 * @param list The integer list to get from.
 * @param index The index to get.
 * @param out_value Pointer to store the retrieved value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_get_int(LinkedList* list, size_t index, int* out_value) {
    if (!list || !out_value) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    int* data_ptr = (int*)list_get(list, index);
    if (!data_ptr) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    *out_value = *data_ptr;
    return LIST_SUCCESS;
}

/**
 * @brief Sets an integer value at a specific index.
 * @param list The integer list to modify.
 * @param index The index to set.
 * @param value The new integer value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_set_int(LinkedList* list, size_t index, int value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_set(list, index, &value);
}

/**
 * @brief Finds the index of an integer value in the list.
 * @param list The integer list to search.
 * @param value The integer value to find.
 * @return The index if found, negative error code if not found.
 */
int list_find_int(LinkedList* list, int value) {
    if (!list) return -LIST_ERROR_NULL_POINTER;
    return list_index(list, &value);
}

/**
 * @brief Removes all occurrences of an integer value from the list.
 * @param list The integer list to remove from.
 * @param value The integer value to remove.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult list_remove_int(LinkedList* list, int value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_remove_advanced(list, &value, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
}

// =========== DOUBLE LIST WRAPPERS ===========

/**
 * @brief Adds a double value to the tail of a double list.
 * @param list The double list to add to.
 * @param value The double value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_double(LinkedList* list, double value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_tail(list, &value);
}

/**
 * @brief Adds a double value to the head of a double list.
 * @param list The double list to add to.
 * @param value The double value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_double_to_head(LinkedList* list, double value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_head(list, &value);
}

/**
 * @brief Inserts a double value at a specific index in a double list.
 * @param list The double list to insert into.
 * @param index The index to insert at.
 * @param value The double value to insert.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_double_at(LinkedList* list, size_t index, double value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_index(list, index, &value);
}

/**
 * @brief Gets a double value from a specific index (safe access).
 * @param list The double list to get from.
 * @param index The index to get.
 * @param out_value Pointer to store the retrieved value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_get_double(LinkedList* list, size_t index, double* out_value) {
    if (!list || !out_value) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    double* data_ptr = (double*)list_get(list, index);
    if (!data_ptr) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    *out_value = *data_ptr;
    return LIST_SUCCESS;
}

/**
 * @brief Sets a double value at a specific index.
 * @param list The double list to modify.
 * @param index The index to set.
 * @param value The new double value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_set_double(LinkedList* list, size_t index, double value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_set(list, index, &value);
}

/**
 * @brief Finds the index of a double value in the list.
 * @param list The double list to search.
 * @param value The double value to find.
 * @return The index if found, negative error code if not found.
 */
int list_find_double(LinkedList* list, double value) {
    if (!list) return -LIST_ERROR_NULL_POINTER;
    return list_index(list, &value);
}

/**
 * @brief Removes all occurrences of a double value from the list.
 * @param list The double list to remove from.
 * @param value The double value to remove.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult list_remove_double(LinkedList* list, double value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_remove_advanced(list, &value, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
}

// =========== CHARACTER LIST WRAPPERS ===========

/**
 * @brief Adds a character value to the tail of a character list.
 * @param list The character list to add to.
 * @param value The character value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_char(LinkedList* list, char value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_tail(list, &value);
}

/**
 * @brief Adds a character value to the head of a character list.
 * @param list The character list to add to.
 * @param value The character value to add.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_char_to_head(LinkedList* list, char value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_head(list, &value);
}

/**
 * @brief Inserts a character value at a specific index in a character list.
 * @param list The character list to insert into.
 * @param index The index to insert at.
 * @param value The character value to insert.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_insert_char_at(LinkedList* list, size_t index, char value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_insert_at_index(list, index, &value);
}

/**
 * @brief Gets a character value from a specific index (safe access).
 * @param list The character list to get from.
 * @param index The index to get.
 * @param out_value Pointer to store the retrieved value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_get_char(LinkedList* list, size_t index, char* out_value) {
    if (!list || !out_value) return LIST_ERROR_NULL_POINTER;
    if (index >= list->length) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    char* data_ptr = (char*)list_get(list, index);
    if (!data_ptr) return LIST_ERROR_INDEX_OUT_OF_BOUNDS;
    
    *out_value = *data_ptr;
    return LIST_SUCCESS;
}

/**
 * @brief Sets a character value at a specific index.
 * @param list The character list to modify.
 * @param index The index to set.
 * @param value The new character value.
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_set_char(LinkedList* list, size_t index, char value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_set(list, index, &value);
}

/**
 * @brief Finds the index of a character value in the list.
 * @param list The character list to search.
 * @param value The character value to find.
 * @return The index if found, negative error code if not found.
 */
int list_find_char(LinkedList* list, char value) {
    if (!list) return -LIST_ERROR_NULL_POINTER;
    return list_index(list, &value);
}

/**
 * @brief Removes all occurrences of a character value from the list.
 * @param list The character list to remove from.
 * @param value The character value to remove.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult list_remove_char(LinkedList* list, char value) {
    if (!list) return LIST_ERROR_NULL_POINTER;
    return list_remove_advanced(list, &value, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
}

// =========== STRING LIST WRAPPERS ===========

/**
 * @brief Adds a string value to the tail of a string list.
 * @param list The string list to add to.
 * @param value The string value to add (will be copied).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_string(LinkedList* list, const char* value) {
    if (!list || !value) return LIST_ERROR_NULL_POINTER;
    char* str_copy = strdup(value);
    if (!str_copy) return LIST_ERROR_MEMORY_ALLOC;
    
    ListResult result = list_insert_at_tail(list, &str_copy);
    if (result != LIST_SUCCESS) {
        free(str_copy);  // Clean up on failure
    }
    return result;
}

/**
 * @brief Adds a string value to the head of a string list.
 * @param list The string list to add to.
 * @param value The string value to add (will be copied).
 * @return LIST_SUCCESS on success, error code on failure.
 */
ListResult list_add_string_to_head(LinkedList* list, const char* value) {
    if (!list || !value) return LIST_ERROR_NULL_POINTER;
    char* str_copy = strdup(value);
    if (!str_copy) return LIST_ERROR_MEMORY_ALLOC;
    
    ListResult result = list_insert_at_head(list, &str_copy);
    if (result != LIST_SUCCESS) {
        free(str_copy);  // Clean up on failure
    }
    return result;
}

/**
 * @brief Gets a string value from a specific index (returns pointer to internal string).
 * @param list The string list to get from.
 * @param index The index to get.
 * @return Pointer to the string at the specified index, or NULL on failure.
 * @warning Do not modify or free the returned string - it's managed by the list.
 */
const char* list_get_string(LinkedList* list, size_t index) {
    if (!list || index >= list->length) return NULL;
    
    char** data_ptr = (char**)list_get(list, index);
    return data_ptr ? *data_ptr : NULL;
}

/**
 * @brief Finds the index of a string value in the list.
 * @param list The string list to search.
 * @param value The string value to find.
 * @return The index if found, negative error code if not found.
 */
int list_find_string(LinkedList* list, const char* value) {
    if (!list || !value) return -LIST_ERROR_NULL_POINTER;
    char* temp_str = strdup(value);
    if (!temp_str) return -LIST_ERROR_MEMORY_ALLOC;
    
    int result = list_index(list, &temp_str);
    free(temp_str);
    return result;
}

/**
 * @brief Removes all occurrences of a string value from the list.
 * @param list The string list to remove from.
 * @param value The string value to remove.
 * @return LIST_SUCCESS if at least one element was removed, error code otherwise.
 */
ListResult list_remove_string(LinkedList* list, const char* value) {
    if (!list || !value) return LIST_ERROR_NULL_POINTER;
    char* temp_str = strdup(value);
    if (!temp_str) return LIST_ERROR_MEMORY_ALLOC;
    
    ListResult result = list_remove_advanced(list, &temp_str, DELETE_ALL_OCCURRENCES, SEARCH_FROM_HEAD);
    free(temp_str);
    return result;
}
