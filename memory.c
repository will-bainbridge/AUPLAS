////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

int allocate_mesh(int n_variables, int n_nodes, struct NODE **node, int n_faces, struct FACE **face, int n_cells, struct CELL **cell, int n_zones, struct ZONE **zone)
{
	int i, j, k, height, width;

	if(n_nodes > 0 && *node == NULL) {
		*node = (struct NODE *)malloc(n_nodes * sizeof(struct NODE));
		if(*node == NULL) { printf("\nERROR - allocate_mesh - allocating nodes"); return ERROR; }
	}
	if(n_faces > 0 && *face == NULL) {
		*face = (struct FACE *)malloc(n_faces * sizeof(struct FACE));
		if(*face == NULL) { printf("\nERROR - allocate_mesh - allocating faces"); return ERROR; }
	}
	if(n_cells > 0 && *cell == NULL) {
		*cell = (struct CELL *)malloc(n_cells * sizeof(struct CELL));
		if(*cell == NULL) { printf("\nERROR - allocate_mesh - allocating cells"); return ERROR; }
	}
	if(n_zones > 0 && *zone == NULL) {
		*zone = (struct ZONE *)malloc(n_zones * sizeof(struct ZONE));
		if(*zone == NULL) { printf("\nERROR - allocate_mesh - allocating zones"); return ERROR; }
	}

	//-------------------------------------------------------------------//
	
	for(i = 0; i < n_faces; i ++)
	{
		if((*face)[i].n_nodes > 0 && (*face)[i].node == NULL) {
			(*face)[i].node = (struct NODE **)malloc((*face)[i].n_nodes * sizeof(struct NODE *));
			if((*face)[i].node == NULL) { printf("\nERROR - allocate_mesh - allocating face nodes"); return ERROR; }
		}
		if((*face)[i].n_borders > 0 && (*face)[i].border == NULL) {
			(*face)[i].border = (struct CELL **)malloc((*face)[i].n_borders * sizeof(struct CELL *));
			if((*face)[i].border == NULL) { printf("\nERROR - allocate_mesh - allocating face borders"); return ERROR; }
		}
		if((*face)[i].n_borders > 0 && (*face)[i].oriented == NULL) {
			(*face)[i].oriented = (int *)malloc((*face)[i].n_borders * sizeof(int));
			if((*face)[i].oriented == NULL) { printf("\nERROR - allocate_mesh - allocating face orientations"); return ERROR; }
		}
		if((*face)[i].n_zones > 0 && (*face)[i].zone == NULL) {
			(*face)[i].zone = (struct ZONE **)malloc((*face)[i].n_zones * sizeof(struct ZONE *));
			if((*face)[i].zone == NULL) { printf("\nERROR - allocate_mesh - allocating face zones"); return ERROR; }
		}
	}

	//-------------------------------------------------------------------//

	for(i = 0; i < n_cells; i ++)
	{
		if((*cell)[i].n_faces > 0 && (*cell)[i].face == NULL) {
			(*cell)[i].face = (struct FACE **)malloc((*cell)[i].n_faces * sizeof(struct FACE *));
			if((*cell)[i].face == NULL) { printf("\nERROR - allocate_mesh - allocating cell faces"); return ERROR; }
		}
		if((*cell)[i].n_faces > 0 && (*cell)[i].oriented == NULL) {
			(*cell)[i].oriented = (int *)malloc((*cell)[i].n_faces * sizeof(int));
			if((*cell)[i].oriented == NULL) { printf("\nERROR - allocate_mesh - allocating cell orientations"); return ERROR; }
		}
		if((*cell)[i].n_zones > 0 && (*cell)[i].zone == NULL) {
			(*cell)[i].zone = (struct ZONE **)malloc((*cell)[i].n_zones * sizeof(struct ZONE *));
			if((*cell)[i].zone == NULL) { printf("\nERROR - allocate_mesh - allocating cell zones"); return ERROR; }
		}

		if(n_variables > 0) {
			if((*cell)[i].order == NULL) {
				(*cell)[i].order = (int *)malloc(n_variables * sizeof(int));
				if((*cell)[i].order == NULL) { printf("\nERROR - allocate_mesh - allocating cell orders"); return ERROR; }
				for(j = 0; j < n_variables; j ++) (*cell)[i].order[j] = 0;
			}
			if((*cell)[i].n_stencil == NULL) {
				(*cell)[i].n_stencil = (int *)malloc(n_variables * sizeof(int));
				if((*cell)[i].n_stencil == NULL) { printf("\nERROR - allocate_mesh - allocating cell stencil numbers"); return ERROR; }
				for(j = 0; j < n_variables; j ++) (*cell)[i].n_stencil[j] = 0;
			}
			if((*cell)[i].stencil == NULL) {
				(*cell)[i].stencil = (int **)malloc(n_variables * sizeof(int *));
				if((*cell)[i].stencil == NULL) { printf("\nERROR - allocate_mesh - allocating cell stencils"); return ERROR; }
				for(j = 0; j < n_variables; j ++) (*cell)[i].stencil[j] = NULL;
			}
			if((*cell)[i].matrix == NULL) {
				(*cell)[i].matrix = (double ***)malloc(n_variables * sizeof(double **));
				if((*cell)[i].matrix == NULL) { printf("\nERROR - allocate_mesh - allocating cell matrices"); return ERROR; }
				for(j = 0; j < n_variables; j ++) (*cell)[i].matrix[j] = NULL;
			}
		}

		if((*cell)[i].n_stencil != NULL) {
			for(j = 0; j < n_variables; j ++) {
				if((*cell)[i].n_stencil[j] > 0 && (*cell)[i].stencil[j] == NULL) {
					(*cell)[i].stencil[j] = (int *)malloc((*cell)[i].n_stencil[j] * sizeof(int));
					if((*cell)[i].stencil[j] == NULL) { printf("\nERROR - allocate_mesh - allocating cell stencil"); return ERROR; }
				}
			}
		}

		if((*cell)[i].order != NULL && (*cell)[i].n_stencil != NULL) {
			for(j = 0; j < n_variables; j ++) {
				if((*cell)[i].order[j] > 0 && (*cell)[i].n_stencil[j] > 0 && (*cell)[i].matrix[j] == NULL) {

					height = ORDER_TO_POWERS((*cell)[i].order[j]);
					width = (*cell)[i].n_stencil[j];

					(*cell)[i].matrix[j] = (double **)malloc(height * sizeof(double *));
					if((*cell)[i].matrix[j] == NULL) { printf("\nERROR - allocate_mesh - allocating cell matrix pointers"); return ERROR; }

					(*cell)[i].matrix[j][0] = (double *)malloc(height * width * sizeof(double));
					if((*cell)[i].matrix[j][0] == NULL) { printf("\nERROR - allocate_mesh - allocating cell matrix values"); return ERROR; }

					for(k = 1; k < height; k ++) (*cell)[i].matrix[j][k] = (*cell)[i].matrix[j][k-1] + width;
				}
			}
		}
	}

	//-------------------------------------------------------------------//
	
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int allocate_instructions(int n_variables, char **geometry_filename, int **maximum_order, double **weight_exponent, char ***connectivity)
{
	int i;

	if(*geometry_filename == NULL) {
		*geometry_filename = (char *)malloc(MAX_STRING_CHARACTERS * sizeof(char));
		if(*geometry_filename == NULL) { printf("\nERROR - allocate_instructions - allocating geometry filename"); return ERROR; }
	}
	if(n_variables > 0 && *maximum_order == NULL) {
		*maximum_order = (int *)malloc(n_variables * sizeof(int));
		if(*maximum_order == NULL) { printf("\nERROR - allocate_instructions - allocating maximum orders"); return ERROR; }
	}
	if(n_variables > 0 && *weight_exponent == NULL) {
		*weight_exponent = (double *)malloc(n_variables * sizeof(double));
		if(*weight_exponent == NULL) { printf("\nERROR - allocate_instructions - allocating weight exponents"); return ERROR; }
	}
	if(n_variables > 0 && *connectivity == NULL) {
		*connectivity = (char **)malloc(n_variables * sizeof(char *));
		if(*connectivity == NULL) { printf("\nERROR - allocate_instructions - allocating connectivity pointers"); return ERROR; }
		(*connectivity)[0] = (char *)malloc(n_variables * MAX_STRING_CHARACTERS * sizeof(char));
		if(*connectivity == NULL) { printf("\nERROR - allocate_instructions - allocating connectivity pointers"); return ERROR; }
		for(i = 1; i < n_variables; i ++) (*connectivity)[i] = (*connectivity)[i-1] + MAX_STRING_CHARACTERS;
	}

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void free_mesh(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i, j;

	free(node);

	for(i = 0; i < n_faces; i ++)
	{
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

void free_instructions(int n_variables, char *geometry_filename, int *maximum_order, double *weight_exponent, char **connectivity)
{
	free(geometry_filename);
	free(maximum_order);
	free(weight_exponent);
	free(connectivity[0]);
	free(connectivity);
}

////////////////////////////////////////////////////////////////////////////////