//////////////////////////////////////////////////////////////////

#include<stdlib.h>
#include<stdio.h>

#include "allocate.h"

//////////////////////////////////////////////////////////////////

//vector allocate functions

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

int allocate_integer_pointer_vector(int ***vector, int length)
{
	*vector = (int **)malloc(length * sizeof(int *));
	if(*vector == NULL) { return ALLOCATE_ERROR; }
	return ALLOCATE_SUCCESS;
}

int allocate_float_vector(float **vector, int length)
{
	*vector = (float *)malloc(length * sizeof(float));
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

int allocate_character_pointer_vector(char ***vector, int length)
{
	*vector = (char **)malloc(length * sizeof(char *));
	if(*vector == NULL) { return ALLOCATE_ERROR; }
	return ALLOCATE_SUCCESS;
}

//////////////////////////////////////////////////////////////////

//matrix allocate functions
	
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

int allocate_float_matrix(float ***matrix, int height, int width)
{
	*matrix = (float **)malloc(height * sizeof(float *));
	if(*matrix == NULL) { return ALLOCATE_ERROR; }
	(*matrix)[0] = (float *)malloc(height * width * sizeof(float));
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

//////////////////////////////////////////////////////////////////

//tensor allocate functions

int allocate_integer_tensor(int ****tensor, int height, int width, int depth)
{
        *tensor = (int ***)malloc(height * sizeof(int **));
        if(*tensor == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0] = (int **)malloc(height * width * sizeof(int *));
        if((*tensor)[0] == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0][0] = (int *)malloc(height * width * depth * sizeof(int));
        if((*tensor)[0][0] == NULL) { return ALLOCATE_ERROR; }

        int i, j;

        for (i = 1; i < height; i ++) { (*tensor)[i] = (*tensor)[i-1] + width; }

        for (i = 1; i < height; i ++) { (*tensor)[i][0] = (*tensor)[i-1][0] + width*depth; }

        for (i = 0; i < height; i ++) {
                for(j = 1; j < width; j ++) {
                        (*tensor)[i][j] = (*tensor)[i][j-1] + depth;
                }
        }

        return ALLOCATE_SUCCESS;
}

int allocate_integer_zero_tensor(int ****tensor, int height, int width, int depth)
{
        *tensor = (int ***)malloc(height * sizeof(int **));
        if(*tensor == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0] = (int **)malloc(height * width * sizeof(int *));
        if((*tensor)[0] == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0][0] = (int *)calloc(height * width * depth, sizeof(int));
        if((*tensor)[0][0] == NULL) { return ALLOCATE_ERROR; }

        int i, j;

        for (i = 1; i < height; i ++) { (*tensor)[i] = (*tensor)[i-1] + width; }

        for (i = 1; i < height; i ++) { (*tensor)[i][0] = (*tensor)[i-1][0] + width*depth; }

        for (i = 0; i < height; i ++) {
                for(j = 1; j < width; j ++) {
                        (*tensor)[i][j] = (*tensor)[i][j-1] + depth;
                }
        }

        return ALLOCATE_SUCCESS;
}

int allocate_float_tensor(float ****tensor, int height, int width, int depth)
{
        *tensor = (float ***)malloc(height * sizeof(float **));
        if(*tensor == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0] = (float **)malloc(height * width * sizeof(float *));
        if((*tensor)[0] == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0][0] = (float *)malloc(height * width * depth * sizeof(float));
        if((*tensor)[0][0] == NULL) { return ALLOCATE_ERROR; }

        int i, j;

        for (i = 1; i < height; i ++) { (*tensor)[i] = (*tensor)[i-1] + width; }

        for (i = 1; i < height; i ++) { (*tensor)[i][0] = (*tensor)[i-1][0] + width*depth; }

        for (i = 0; i < height; i ++) {
                for(j = 1; j < width; j ++) {
                        (*tensor)[i][j] = (*tensor)[i][j-1] + depth;
                }
        }

        return ALLOCATE_SUCCESS;
}

int allocate_double_tensor(double ****tensor, int height, int width, int depth)
{
        *tensor = (double ***)malloc(height * sizeof(double **));
        if(*tensor == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0] = (double **)malloc(height * width * sizeof(double *));
        if((*tensor)[0] == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0][0] = (double *)malloc(height * width * depth * sizeof(double));
        if((*tensor)[0][0] == NULL) { return ALLOCATE_ERROR; }

        int i, j;

        for (i = 1; i < height; i ++) { (*tensor)[i] = (*tensor)[i-1] + width; }

        for (i = 1; i < height; i ++) { (*tensor)[i][0] = (*tensor)[i-1][0] + width*depth; }

        for (i = 0; i < height; i ++) {
                for(j = 1; j < width; j ++) {
                        (*tensor)[i][j] = (*tensor)[i][j-1] + depth;
                }
        }

        return ALLOCATE_SUCCESS;
}

int allocate_character_tensor(char ****tensor, int height, int width, int depth)
{
        *tensor = (char ***)malloc(height * sizeof(char **));
        if(*tensor == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0] = (char **)malloc(height * width * sizeof(char *));
        if((*tensor)[0] == NULL) { return ALLOCATE_ERROR; }

        (*tensor)[0][0] = (char *)malloc(height * width * depth * sizeof(char));
        if((*tensor)[0][0] == NULL) { return ALLOCATE_ERROR; }

        int i, j;

        for (i = 1; i < height; i ++) { (*tensor)[i] = (*tensor)[i-1] + width; }

        for (i = 1; i < height; i ++) { (*tensor)[i][0] = (*tensor)[i-1][0] + width*depth; }

        for (i = 0; i < height; i ++) {
                for(j = 1; j < width; j ++) {
                        (*tensor)[i][j] = (*tensor)[i][j-1] + depth;
                }
        }

        return ALLOCATE_SUCCESS;
}

//////////////////////////////////////////////////////////////////

//free functions

void free_vector(void *vector)
{
	free(vector);
}

void free_matrix(void **matrix)
{
	free(matrix[0]);
	free(matrix);	
}

void free_tensor(void ***tensor)
{
        free(tensor[0][0]);
        free(tensor[0]);
        free(tensor);
}

//////////////////////////////////////////////////////////////////
