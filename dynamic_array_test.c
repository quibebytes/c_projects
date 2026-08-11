#include<stdlib.h>
#include<math.h>
#include<stdio.h>

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include"dynamic_array.h"

int main(void) {
    DynamicArray array;
    initialize_array(&array);
    push(&array, 3.14159);
    push(&array, sqrt(2.0));
    push(&array, 2.71828);

    printf("Euler: %f \n", pop(&array));

    push(&array, 1.4448);
    push(&array, get(&array, 0) * 2);

    grow_array(&array, 6);
    set(&array, 8, sqrt(343.0));

    for (size_t i = 0; i < array.length; i++) {
        printf("%f ", get(&array, i));
    }

    printf("\n");

    free_array(&array);

    exit(EXIT_SUCCESS);
}
