////////////////////////////////////////////////////////////////////////////////

#define MAX_STRING_LENGTH 128

#define FETCH_FILE_ERROR -1
#define FETCH_MEMORY_ERROR -2

////////////////////////////////////////////////////////////////////////////////

void **fetch_allocate(char *format, int max_n_lines);
int fetch_read(FILE *file, char *label, char *format, int max_n_lines, void **data);
void fetch_get(char *format, void **data, int line_index, int value_index, void *value);
void fetch_print(char *format, int n_lines, void **data);
void fetch_free(char *format, int max_n_lines, void **data);

////////////////////////////////////////////////////////////////////////////////

