//////////////////////////////////////////////////////////////////

#define ALLOCATE_SUCCESS 1
#define ALLOCATE_ERROR 0

//////////////////////////////////////////////////////////////////

//vector allocate functions

int allocate_integer_vector(int **vector, int length);
int allocate_integer_zero_vector(int **vector, int length);
int allocate_integer_pointer_vector(int ***vector, int length);
int allocate_float_vector(float **vector, int length);
int allocate_double_vector(double **vector, int length);
int allocate_character_vector(char **vector, int length);
int allocate_character_pointer_vector(char ***vector, int length);

//////////////////////////////////////////////////////////////////

//matrix allocate functions
	
int allocate_integer_matrix(int ***matrix, int height, int width);
int allocate_integer_zero_matrix(int ***matrix, int height, int width);
int allocate_float_matrix(float ***matrix, int height, int width);
int allocate_double_matrix(double ***matrix, int height, int width);
int allocate_character_matrix(char ***matrix, int height, int width);

//////////////////////////////////////////////////////////////////

//tensor allocate functions
int allocate_integer_tensor(int ****tensor, int height, int width, int depth);
int allocate_integer_zero_tensor(int ****tensor, int height, int width, int depth);
int allocate_float_tensor(float ****tensor, int height, int width, int depth);
int allocate_double_tensor(double ****tensor, int height, int width, int depth);
int allocate_character_tensor(char ****tensor, int height, int width, int depth);

//////////////////////////////////////////////////////////////////

//free functions

void free_vector(void *vector);
void free_matrix(void **matrix);
void free_tensor(void ***tensor);

//////////////////////////////////////////////////////////////////
