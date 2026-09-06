#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>


#define ARENA_IMPLEMENTATION
#include "arena.h"

void print_memory16(unsigned char *memory, size_t size) {
    for (int i = 0; i < size / 16; i++) {
        for (int j = 0; j < 16; j++) {
            printf("%.2x ", memory[j + (i * 16)]);
        }

        for (int j = 0; j < 16; j++) {
            putchar(isgraph(memory[j + (i * 16)])? memory[j + (i * 16)]: ' ');
        }
    
        putchar('\n');
    }

    putchar('\n');
    fflush(stdout);
}

int main () {
    puts("Sparse memory stuff - Use a task manager to see the memory used or reserved");
    fflush(stdout);

    Arena sparse;    
    arena_create(&sparse);
    assert(sparse.buffer);

    getchar();

    const char things[] = "The angles you don't plan for, the things you might have missed, those things exist.";
    char *things2;

    for (size_t i = 0; i < 1024LL * 1024LL; i++) {
        things2 = (char *) arena_allocate(&sparse, sizeof(things));
        assert(things2);
        memcpy(things2, things, sizeof(things));
    }   
    assert((uintptr_t) things2 == (uintptr_t) sparse.buffer + (uintptr_t) sparse.previous_offset);
    print_memory16(sparse.buffer, 256);

    char *things3 = (char *) arena_resize(&sparse, things2, sizeof(things), 4096);
    assert(things3);
    getchar();
    puts("cleared arena");
    fflush(stdout);
    arena_clear(&sparse);
    getchar();

    return 0;
}
