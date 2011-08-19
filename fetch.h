#ifndef FETCH_H
#define FETCH_H

#define FETCH_FILE_ERROR -1
#define FETCH_MEMORY_ERROR -2

typedef struct _FETCH * FETCH;

FETCH fetch_new(char *format, int max_n_lines);
int fetch_read(FILE *file, char *label, FETCH input);
void fetch_get(FETCH input, int line_index, int value_index, void *value);
void fetch_print(FETCH input);
void fetch_destroy(FETCH input);

#endif
