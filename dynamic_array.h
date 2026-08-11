/*

File is a stb-style library and as such needs DYNAMIC_ARRAY_IMPLEMENTATION
to be defined somewhere for the function definitions to show up.

more information for stb-style libraries here
https://github.com/nothings/stb#faq

*/

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef INITIALIZED_ELEMENT
#define INITIALIZED_ELEMENT 0.0
#endif

#ifndef ELEMENT_TYPE
#define ELEMENT_TYPE double
#endif

typedef ELEMENT_TYPE Element;

typedef struct DynamicArray {
    size_t length;
    Element *contents;
} DynamicArray;

void initialize_array(DynamicArray *array);
void grow_array(DynamicArray *array, size_t amount);
void set(DynamicArray *array, size_t index, Element value);
Element get(DynamicArray *array, size_t index);
void push(DynamicArray *array, Element value);
Element pop(DynamicArray *array);
void free_array(DynamicArray *array);

#endif // DYNAMIC_ARRAY_H

#ifdef  DYNAMIC_ARRAY_IMPLEMENTATION

void initialize_array(DynamicArray *array) {
    array->length = 0;
    array->contents = NULL;
}

void grow_array(DynamicArray *array, size_t amount) {
    Element *new_array = (Element *) realloc(array->contents, sizeof(Element)*(array->length+amount));
    if (new_array == NULL) {
        free_array(array);
        fprintf(stderr, "Failed to grow DynamicArray by %ld", amount);
        exit(EXIT_FAILURE);
    }

    array->contents = new_array;

    for (size_t i = array->length; i < array->length+amount; i++) {
        array->contents[i] = INITIALIZED_ELEMENT;
    }

    array->length += amount;
}

void set(DynamicArray *array, size_t index, Element value) {
    array->contents[index] = value;
}

Element get(DynamicArray *array, size_t index) {
    return array->contents[index];
}

void push(DynamicArray *array, Element value) {
    grow_array(array, 1);
    array->contents[array->length-1] = value;
}

Element pop(DynamicArray *array) {
    if (array->length == 0) {
        fprintf(stderr, "Tried to pop an empty DynamicArray");
        exit(EXIT_FAILURE);
    }
    Element element = array->contents[array->length-1];
    Element *new_array = (Element *) realloc(array->contents, sizeof(Element)*(array->length--));
    if (new_array == NULL) {
        free_array(array);
        fprintf(stderr, "Failed to shrink DynamicArray");
        exit(EXIT_FAILURE);
    }

    array->contents = new_array;

    return element;
}

void free_array(DynamicArray *array) {
    free(array->contents);
    initialize_array(array);
}

#endif // DYNAMIC_ARRAY_IMPLEMENTATION