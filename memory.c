////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

int allocate_mesh(int n_variables, int n_nodes, struct NODE **node, int n_faces, struct FACE **face, int n_cells, struct CELL **cell, int n_zones, struct ZONE **zone)
{
	int i, j, k, height, width;

	if(n_nodes > 0 && *node == NULL) {
		*node = (struct NODE *)malloc(n_nodes * sizeof(struct NODE));
		if(*node == NULL) return ALLOCATE_ERROR;
	}
	if(n_faces > 0 && *face == NULL) {
		*face = (struct FACE *)malloc(n_faces * sizeof(struct FACE));
		if(*face == NULL) return ALLOCATE_ERROR;
		for(i = 0; i < n_faces; i ++) {
			(*face)[i].n_nodes = 0; (*face)[i].n_borders = 0; (*face)[i].n_zones = 0;
			(*face)[i].node = NULL; (*face)[i].border = NULL; (*face)[i].oriented = NULL; (*face)[i].zone = NULL;
		}
	}
	if(n_cells > 0 && *cell == NULL) {
		*cell = (struct CELL *)malloc(n_cells * sizeof(struct CELL));
		if(*cell == NULL) return ALLOCATE_ERROR;
		for(i = 0; i < n_cells; i ++) {
			(*cell)[i].n_faces = 0; (*cell)[i].n_zones = 0;
			(*cell)[i].face = NULL; (*cell)[i].oriented = NULL; (*cell)[i].zone = NULL;
			(*cell)[i].order = NULL; (*cell)[i].n_stencil = NULL; (*cell)[i].stencil = NULL; (*cell)[i].matrix = NULL;
		}
	}
	if(n_zones > 0 && *zone == NULL) {
		*zone = (struct ZONE *)malloc(n_zones * sizeof(struct ZONE));
		if(*zone == NULL) return ALLOCATE_ERROR;
	}

	//-------------------------------------------------------------------//
	
	for(i = 0; i < n_faces; i ++)
	{
		if((*face)[i].n_nodes > 0 && (*face)[i].node == NULL) {
			(*face)[i].node = (struct NODE **)malloc((*face)[i].n_nodes * sizeof(struct NODE *));
			if((*face)[i].node == NULL) return ALLOCATE_ERROR;
		}
		if((*face)[i].n_borders > 0 && (*face)[i].border == NULL) {
			(*face)[i].border = (struct CELL **)malloc((*face)[i].n_borders * sizeof(struct CELL *));
			if((*face)[i].border == NULL) return ALLOCATE_ERROR;
		}
		if((*face)[i].n_borders > 0 && (*face)[i].oriented == NULL) {
			(*face)[i].oriented = (int *)malloc((*face)[i].n_borders * sizeof(int));
			if((*face)[i].oriented == NULL) return ALLOCATE_ERROR;
		}
		if((*face)[i].n_zones > 0 && (*face)[i].zone == NULL) {
			(*face)[i].zone = (struct ZONE **)malloc((*face)[i].n_zones * sizeof(struct ZONE *));
			if((*face)[i].zone == NULL) return ALLOCATE_ERROR;
		}
	}

	//-------------------------------------------------------------------//

	for(i = 0; i < n_cells; i ++)
	{
		if((*cell)[i].n_faces > 0 && (*cell)[i].face == NULL) {
			(*cell)[i].face = (struct FACE **)malloc((*cell)[i].n_faces * sizeof(struct FACE *));
			if((*cell)[i].face == NULL) return ALLOCATE_ERROR;
		}
		if((*cell)[i].n_faces > 0 && (*cell)[i].oriented == NULL) {
			(*cell)[i].oriented = (int *)malloc((*cell)[i].n_faces * sizeof(int));
			if((*cell)[i].oriented == NULL) return ALLOCATE_ERROR;
		}
		if((*cell)[i].n_zones > 0 && (*cell)[i].zone == NULL) {
			(*cell)[i].zone = (struct ZONE **)malloc((*cell)[i].n_zones * sizeof(struct ZONE *));
			if((*cell)[i].zone == NULL) return ALLOCATE_ERROR;
		}

		if(n_variables > 0) {
			if((*cell)[i].order == NULL) {
				(*cell)[i].order = (int *)malloc(n_variables * sizeof(int));
				if((*cell)[i].order == NULL) return ALLOCATE_ERROR;
				for(j = 0; j < n_variables; j ++) (*cell)[i].order[j] = 0;
			}
			if((*cell)[i].n_stencil == NULL) {
				(*cell)[i].n_stencil = (int *)malloc(n_variables * sizeof(int));
				if((*cell)[i].n_stencil == NULL) return ALLOCATE_ERROR;
				for(j = 0; j < n_variables; j ++) (*cell)[i].n_stencil[j] = 0;
			}
			if((*cell)[i].stencil == NULL) {
				(*cell)[i].stencil = (int **)malloc(n_variables * sizeof(int *));
				if((*cell)[i].stencil == NULL) return ALLOCATE_ERROR;
				for(j = 0; j < n_variables; j ++) (*cell)[i].stencil[j] = NULL;
			}
			if((*cell)[i].matrix == NULL) {
				(*cell)[i].matrix = (double ***)malloc(n_variables * sizeof(double **));
				if((*cell)[i].matrix == NULL) return ALLOCATE_ERROR;
				for(j = 0; j < n_variables; j ++) (*cell)[i].matrix[j] = NULL;
			}
		}

		if((*cell)[i].n_stencil != NULL) {
			for(j = 0; j < n_variables; j ++) {
				if((*cell)[i].n_stencil[j] > 0 && (*cell)[i].stencil[j] == NULL) {
					(*cell)[i].stencil[j] = (int *)malloc((*cell)[i].n_stencil[j] * sizeof(int));
					if((*cell)[i].stencil[j] == NULL) return ALLOCATE_ERROR;
				}
			}
		}

		if((*cell)[i].order != NULL && (*cell)[i].n_stencil != NULL) {
			for(j = 0; j < n_variables; j ++) {
				if((*cell)[i].order[j] > 0 && (*cell)[i].n_stencil[j] > 0 && (*cell)[i].matrix[j] == NULL) {

					height = ORDER_TO_POWERS((*cell)[i].order[j]);
					width = (*cell)[i].n_stencil[j];

					(*cell)[i].matrix[j] = (double **)malloc(height * sizeof(double *));
					if((*cell)[i].matrix[j] == NULL) return ALLOCATE_ERROR;

					(*cell)[i].matrix[j][0] = (double *)malloc(height * width * sizeof(double));
					if((*cell)[i].matrix[j][0] == NULL) return ALLOCATE_ERROR;

					for(k = 1; k < height; k ++) (*cell)[i].matrix[j][k] = (*cell)[i].matrix[j][k-1] + width;
				}
			}
		}
	}

	//-------------------------------------------------------------------//
	
	return ALLOCATE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void free_mesh(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i, j;

	free(node);

	for(i = 0; i < n_faces; i ++)
	{
		free(face[i].node);
		free(face[i].border);
		free(face[i].oriented);
		free(face[i].zone);
	}
	free(face);

	for(i = 0; i < n_cells; i ++)
	{
		free(cell[i].face);
		free(cell[i].oriented);
		free(cell[i].zone);
		free(cell[i].order);
		free(cell[i].n_stencil);
		for(j = 0; j < n_variables; j ++) { free(cell[i].stencil[j]); }
		free(cell[i].stencil);
		for(j = 0; j < n_variables; j ++) { free(cell[i].matrix[j][0]); free(cell[i].matrix[j]); }
		free(cell[i].matrix);
	}
	free(cell);

	free(zone);
}

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
