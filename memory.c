////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

int allocate_integer_vector(int **vector, int length)
{
	*vector = (int *)malloc(length * sizeof(int));
	if(*vector == NULL) { return ALLOCATE_ERROR; }
	return ALLOCATE_SUCCESS;
}

int allocate_integer_zero_vector(int **vector, int length)
{
        *vector = (int *)calloc(length, sizeof(int));
        if(*vector == NULL) { return ALLOCATE_ERROR; }
        return ALLOCATE_SUCCESS;
}

int allocate_double_vector(double **vector, int length)
{
	*vector = (double *)malloc(length * sizeof(double));
	if(*vector == NULL) { return ALLOCATE_ERROR; }
	return ALLOCATE_SUCCESS;
}

int allocate_character_vector(char **vector, int length)
{
	*vector = (char *)malloc(length * sizeof(char));
	if(*vector == NULL) { return ALLOCATE_ERROR; }
	return ALLOCATE_SUCCESS;
}

void free_vector(void *vector)
{
	free(vector);
}

int allocate_integer_matrix(int ***matrix, int height, int width)
{
	*matrix = (int **)malloc(height * sizeof(int *));
	if(*matrix == NULL) { return ALLOCATE_ERROR; }
	(*matrix)[0] = (int *)malloc(height * width * sizeof(int));
	if((*matrix)[0] == NULL) { return ALLOCATE_ERROR; }
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return ALLOCATE_SUCCESS;
}

int allocate_integer_zero_matrix(int ***matrix, int height, int width)
{
        *matrix = (int **)malloc(height * sizeof(int *));
        if(*matrix == NULL) { return ALLOCATE_ERROR; }
        (*matrix)[0] = (int *)calloc(height * width, sizeof(int));
        if((*matrix)[0] == NULL) { return ALLOCATE_ERROR; }
        int i;
        for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
        return ALLOCATE_SUCCESS;
}

int allocate_double_matrix(double ***matrix, int height, int width)
{
	*matrix = (double **)malloc(height * sizeof(double *));
	if(*matrix == NULL) { return ALLOCATE_ERROR; }
	(*matrix)[0] = (double *)malloc(height * width * sizeof(double));
	if((*matrix)[0] == NULL) { return ALLOCATE_ERROR; }
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return ALLOCATE_SUCCESS;
}

int allocate_double_pointer_matrix(double ****matrix, int height, int width)
{
	*matrix = (double ***)malloc(height * sizeof(double **));
	if(*matrix == NULL) { return ALLOCATE_ERROR; }
	(*matrix)[0] = (double **)malloc(height * width * sizeof(double *));
	if((*matrix)[0] == NULL) { return ALLOCATE_ERROR; }
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return ALLOCATE_SUCCESS;
}

int allocate_character_matrix(char ***matrix, int height, int width)
{
	*matrix = (char **)malloc(height * sizeof(char *));
	if(*matrix == NULL) { return ALLOCATE_ERROR; }
	(*matrix)[0] = (char *)malloc(height * width * sizeof(char));
	if((*matrix)[0] == NULL) { return ALLOCATE_ERROR; }
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return ALLOCATE_SUCCESS;
}

void free_matrix(void **matrix)
{
	free(matrix[0]);
	free(matrix);	
}

//////////////////////////////////////////////////////////////////
