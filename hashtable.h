/*

This is a stb-style library and as such needs HASHTABLE_IMPLEMENTATION
to be defined somewhere for the function definitions to show up.
More information on stb-style libraries here:
https://github.com/nothings/stb#faq

This library uses custom int type names for convenience,
so you need to either add intdefs.h to your include directory
OR define your own int types on the library file.

Feel free to edit the library to use the types you prefer.

TODO:  String interning?
TODO:  add generic interface?

Code used for reference:
    CORMEN, Thomas H. et al. Introduction to Algorithms. ISBN-13 9780262046305
    NYSTROM, Robert. Crafting Interpreters. ISBN-13 9780990582939
    KUTEPOV, Alexey. ht.h (Version 1.1.0) [computer software]. https://github.com/tsoding/ht.h/blob/main/ht.h
    BARRET, Sean T. stb_ds.h (Version 0.67) [computer software]. https://github.com/nothings/stb/blob/master/stb_ds.h
*/
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include <assert.h>

#include "intdefs.h"

#define NULL_STRING ((String) { .chars = NULL, .length = 0})

// TODO: Use SipHash instead of fnv-1a
// TODO: make things configurable per hash table maybe?
// Expects function to accept String and return U64
#ifndef HASHTABLE_HASH
#define HASHTABLE_HASH(str) fnv_1a(str)
#endif

#ifndef HASHTABLE_INITIAL_CAPACITY
#define HASHTABLE_INITIAL_CAPACITY 16
#endif

#ifndef HASHTABLE_GROWTH_FACTOR
#define HASHTABLE_GROWTH_FACTOR 2
#endif

#ifndef HASHTABLE_MAX_LOAD
#define HASHTABLE_MAX_LOAD 0.75f
#endif

typedef struct String {
    const char *chars;
    Size length;
} String;

typedef struct Entry {
    String str;
    bool   marked;
} Entry;

typedef struct HashTable {
    Size    item_count;
    Size    capacity;
    Entry  *entries;
    double *values;
} HashTable;

U64    fnv_1a(String str);
bool   string_equal(String str1, String str2);
void   table_destroy(HashTable *table);
bool   table_get(const HashTable *table, String str, Size *index, double *value);
bool   table_get_index(const HashTable *table, String str, Size *index);
bool   table_get_value(const HashTable *table, String str, double *value);
void   table_new(HashTable *table, Size capacity);
void   table_remove(HashTable *table, String str);
void   table_set(HashTable *table, String str, double value);
void   table_set_from_index(HashTable *table, Size index, String str, double value);
String to_string(const char *str);

#endif // HASHTABLE_H

#ifdef HASHTABLE_IMPLEMENTATION

// TODO vectorize
// strings with NULL chars and different lengths aren't equal
bool string_equal(String str1, String str2) {
    bool result = (str1.length == str2.length) && (str1.chars == str2.chars);

    if (str1.chars && str2.chars) {
        for (Size i = 0; result && i < str1.length; i++) {
            result = str1.chars[i] == str2.chars[i];
        }
    }

    return result;
}

// from a null terminated string
String to_string(const char *chars) {
    Size length = 0;
    while (chars[length] != '\0') { length++; }
    return (String) { .length = length, .chars = chars };
}

// FNV-1a hashing algorithm
U64 fnv_1a(String str) {
    assert(str.chars && "tried to hash a String pointing to NULL");

    U64 hash = 2166136261uLL;

    for (Size i = 0; i < str.length; i++) {
        hash ^= (U8) str.chars[i];
        hash *= 16777619uLL;
    }

    return hash;
}

void table_new(HashTable *table, Size capacity) {
    table->entries = calloc(capacity, sizeof(*table->entries));
    if (!table->entries) { perror("new table entries"); exit(EXIT_FAILURE); }
    table->values = calloc(capacity, sizeof(*table->values));
    if (!table->values) { perror("new table values"); exit(EXIT_FAILURE); }

    table->capacity= capacity;
    table->item_count = 0;
}

void table_destroy(HashTable *table) {
    free(table->entries);
    free(table->values);
    *table = (HashTable) {0};
}

bool table_get(const HashTable *table, String str, Size *index, double *value) {
    Size i = 0, marked_slot = 0;
    U64 hash = HASHTABLE_HASH(str);
    bool found = false, found_marked = false, finish = false;
    do {
        U64 slot = (hash + i) % table->capacity;

        if (!found_marked && table->entries[slot].marked) {
            marked_slot = slot;
            found_marked = true;
        } else if (!table->entries[slot].str.chars) {
            slot = found_marked? marked_slot: slot;
            if (index) { *index = slot; }
            if (value) { *value = table->values[slot]; }
            finish = true;
        } else if (string_equal(str, table->entries[slot].str)) {
            if (index) { *index = slot; }
            if (value) { *value = table->values[slot]; }
            found = true;
            finish = true;
        }
        
        i++;
    } while (!finish && i < table->capacity);

    assert(i < table->capacity && "HashTable is full");

    return found;
}

bool table_get_index(const HashTable *table, String str, Size *index) {
    return table_get(table, str, index, NULL);
}

bool table_get_value(const HashTable *table, String str, double *value) {
    return table_get(table, str, NULL, value);
}

void table_set_from_index(HashTable *table, Size index, String str, double value) {
    assert(index < table->capacity && "Index out of bounds of HashTable");

    table->item_count += !table->entries[index].str.chars || table->entries[index].marked;
    table->entries[index].marked = false;
    table->entries[index].str = str;
    table->values[index] = value;
}

void table_set(HashTable *table, String str, double value) {
    assert(str.chars && "Tried to add NULL String");
    if (!str.chars) { return; }

    if (
        table->capacity < 1 ||
        ((float) (table->item_count + 1) / (float) table->capacity) > HASHTABLE_MAX_LOAD
    ) {
        Size new_capacity = 0;
        if (table->capacity < HASHTABLE_INITIAL_CAPACITY) {
            new_capacity = HASHTABLE_INITIAL_CAPACITY;
        } else {
            new_capacity = table->capacity * HASHTABLE_GROWTH_FACTOR;
        }

        Entry *entries = calloc(new_capacity, sizeof(*table->entries));
        if (!entries) { perror("growing table entries"); exit(EXIT_FAILURE); }
        double *values = calloc(new_capacity, sizeof(*table->values));
        if (!values) { perror("growing table values"); exit(EXIT_FAILURE); }

        HashTable new_table = {
            .entries = entries,
            .values = values,
            .item_count = table->item_count,
            .capacity = new_capacity
        };

        for (Size i = 0; i < table->capacity; i++) {
            if (table->entries[i].str.chars && !table->entries[i].marked) {
                Size new_index = 0;
                table_get_index(&new_table, table->entries[i].str, &new_index);
                entries[new_index].str = table->entries[i].str;
                entries[new_index].marked = false;
                values[new_index] = table->values[i];
            }
        }

        free(table->entries);
        free(table->values);
        table->capacity = new_capacity;
        table->entries = entries;
        table->values = values;
    }

    Size index = 0;
    table_get_index(table, str, &index);
    table_set_from_index(table, index, str, value);
}

void table_remove(HashTable *table, String str) {
    if (table->capacity > 0 && str.chars) {
        Size index = 0;
        bool found = table_get_index(table, str, &index);
        if (found) {
            table->item_count--;
            table->entries[index].marked = true;
        }
    }
}

#endif // HASHTABLE_IMPLEMENTATION
