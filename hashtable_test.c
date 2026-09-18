/*
This test file requires intdefs.h for compilation and
for words.txt to be in the same directory of it's executable.
*/
#include <stdlib.h>
#include <stdio.h>
#include "intdefs.h"

#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"


int main(void) {
    HashTable table = {0};
    table_new(&table, 0);

    FILE *words_txt = fopen("./words.txt", "r");
    if (!words_txt) {
        fprintf(stderr, "Error opening words.txt");
        exit(EXIT_FAILURE);
    }

    FILE *keys_before_txt = fopen("keys_before.txt", "w");
    if (!keys_before_txt) {
        fprintf(stderr, "Error opening keys_before.txt");
        exit(EXIT_FAILURE);
    }

    FILE *keys_after_txt  = fopen("keys_after.txt", "w");
    if (!keys_after_txt) {
        fprintf(stderr, "Error opening keys_after.txt");
        exit(EXIT_FAILURE);
    }

    fseek(words_txt, 0, SEEK_END);
    Size words_txt_size = ftell(words_txt);
    rewind(words_txt);

    char *words = calloc(words_txt_size, 1);
    if (!words) { perror("main"); exit(EXIT_FAILURE); }
    if (fread(words, 1, words_txt_size, words_txt) == 0) {
        fprintf(stderr, "Error reading file words.txt\n");
        exit(EXIT_FAILURE);
    }

    for (Size i = 0, last_line_start = 0, lines = 1; i < words_txt_size; i++) {
        if (words[i] == '\n') {
            String word = { .chars = words + last_line_start, .length = i - last_line_start};
            table_set(&table, word, (double) lines);
            last_line_start = i + 1;
            lines++;
        }
    }

    for (Size i = 0; i < table.capacity; i++) {
        if (table.entries[i].str.chars) {
            fprintf(
                keys_before_txt,
                "%zu: \"%.*s\" = %.0f; %d\n",
                i,
                (int) table.entries[i].str.length, // int instead of size_t or at least int64_t >:(
                table.entries[i].str.chars,
                table.values[i],
                table.entries[i].marked
            );
        }
    }

    // TESTING TABLE REMOVAL

    // should work without doing anything when encountering empty entries
    for (int i = 0; i < table.capacity; i++) {
        table_remove(&table, table.entries[i].str);
    }

    for (Size i = 0, last_line_start = 0, lines = 1; i < words_txt_size; i++) {
        if (words[i] == '\n') {
            String word = { .chars = words + last_line_start, .length = i - last_line_start};
            table_set(&table, word, (double) lines);
            last_line_start = i + 1;
            lines++;
        }
    }

    for (Size i = 0; i < table.capacity; i++) {
        if (table.entries[i].str.chars) {
            fprintf(
                keys_after_txt,
                "%zu: \"%.*s\" = %.0f; %d\n",
                i,
                (int) table.entries[i].str.length, // int instead of size_t or at least int64_t >:(
                table.entries[i].str.chars,
                table.values[i],
                table.entries[i].marked
            );
        }
    }

    printf("Number of items is %zu;\n", table.item_count);
    printf("Capacity is %zu;\n\n", table.capacity);
    printf("Compare files to check table values before and after removal, the diff should be empty\n");

    free(words);
    table_destroy(&table);

    return 0;
}
