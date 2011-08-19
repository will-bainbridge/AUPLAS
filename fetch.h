#ifndef FETCH_H
#define FETCH_H

#define FETCH_FILE_ERROR -1
#define FETCH_MEMORY_ERROR -2

typedef struct _fetch * fetch;

fetch fetch_new(char *format, int max_n_lines);
int fetch_read(FILE *file, char *label, fetch input);
void fetch_get(fetch input, int line_index, int value_index, void *value);
void fetch_print(fetch input);
void fetch_destroy(fetch input);

#endif