////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

struct CELL * cells_new(int n_cells, struct CELL *cell)
{
	cell = (struct CELL *)realloc(cell, n_cells * sizeof(struct CELL));
	if(cell == NULL) return NULL;

	int i;
	for(i = 0; i < n_cells; i ++)
	{
		cell[i].n_faces = 0;
		cell[i].face = NULL;
		cell[i].oriented = NULL;
		cell[i].n_zones = 0;
		cell[i].zone = NULL;
		cell[i].order = NULL;
		cell[i].n_stencil = NULL;
		cell[i].stencil = NULL;
		cell[i].matrix = NULL;
	}

	return cell;
}

////////////////////////////////////////////////////////////////////////////////

int cell_face_new(struct CELL *cell)
{
	cell->face = (struct FACE **)realloc(cell->face, cell->n_faces * sizeof(struct FACE *));
	if(cell->face == NULL) return 0;
	
	return 1;
}

int cell_oriented_new(struct CELL *cell)
{
	cell->oriented = (int *)realloc(cell->oriented, cell->n_faces * sizeof(int));
	if(cell->oriented == NULL) return 0;
	
	return 1;
}

int cell_zone_new(struct CELL *cell)
{
	cell->zone = (struct ZONE **)realloc(cell->zone, cell->n_zones * sizeof(struct ZONE *));
	if(cell->zone == NULL) return 0;
	
	return 1;
}

int cell_order_new(int n_variables, struct CELL *cell)
{
	cell->order = (int *)realloc(cell->order, n_variables * sizeof(int));
	if(cell->order == NULL) return 0;

	int i;
	for(i = 0; i < n_variables; i ++) cell->order[i] = 0;

	return cell_matrix_new(n_variables, cell);
}

int cell_n_stencil_new(int n_variables, struct CELL *cell)
{
	cell->n_stencil = (int *)realloc(cell->n_stencil, n_variables * sizeof(int));
	if(cell->n_stencil == NULL) return 0;

	int i;
	for(i = 0; i < n_variables; i ++) cell->n_stencil[i] = 0;

	return cell_stencil_new(n_variables, cell);
}

int cell_stencil_new(int n_variables, struct CELL *cell)
{
	cell->stencil = (int **)realloc(cell->stencil, n_variables * sizeof(int *));
	if(cell->stencil == NULL) return 0;

	int i;
	for(i = 0; i < n_variables; i ++)
	{
		if(cell->n_stencil[i] > 0)
		{
			cell->stencil[i] = (int *)realloc(cell->stencil[i], cell->n_stencil[i] * sizeof(int));
			if(cell->stencil[i] == NULL) return 0;
		}
		else
		{
			cell->stencil[i] = NULL;
		}
	}

	return 1;
}

int cell_matrix_new(int n_variables, struct CELL *cell)
{
	cell->matrix = (double ***)realloc(cell->matrix, n_variables * sizeof(double **));
	if(cell->matrix == NULL) return 0;

	int i, j, height, width, new;
	for(i = 0; i < n_variables; i ++)
	{
		if(cell->order[i] > 0 && cell->n_stencil[i] > 0)
		{
			height = ORDER_TO_POWERS(cell->order[i]);
			width = cell->n_stencil[i];

			new = (cell->matrix[i] == NULL);

			cell->matrix[i] = (double **)realloc(cell->matrix[i], height * sizeof(double *));
			if(cell->matrix[i] == NULL) return 0;

			cell->matrix[i][0] = (double *)realloc(new ? NULL : cell->matrix[i][0], height * width * sizeof(double));
			if(cell->matrix[i][0] == NULL) return 0;

			for(j = 1; j < height; j ++) cell->matrix[i][j] = cell->matrix[i][j-1] + width;
		}
		else
		{
			cell->matrix[i] = NULL;
		}
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////

void cell_geometry_get(FILE *file, struct CELL *cell, struct FACE *face)
{
	int i;

	//temporary storage
	int *index, count, offset;
	char *line, *temp;
	index = (int *)malloc(MAX_CELL_FACES * sizeof(int));
	line = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	handle(1,index != NULL && line != NULL && temp != NULL ,"allocating temporary storage");

	//read the line
	handle(1,fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a cell line");

	//eat up whitespace and newlines
	for(i = strlen(line)-1; i >= 0; i --) if(line[i] != ' ' && line[i] != '\n') break;
	line[i+1] = '\0';

	//sequentially read the integers on the line
	count = offset = 0;
	while(offset < strlen(line))
	{
		sscanf(&line[offset],"%s",temp);
		sscanf(temp,"%i",&index[count]);
		count ++;
		offset += strlen(temp) + 1;
		while(line[offset] == ' ') offset ++;
	}

	//number of faces
	cell->n_faces = count;

	//allocate the faces
	handle(1,cell_face_new(cell),"allocating cell faces");

	//face pointers
	for(i = 0; i < count; i ++) cell->face[i] = &face[index[i]];

	//clean up
	free(index);
	free(line);
	free(temp);
}

////////////////////////////////////////////////////////////////////////////////

void cells_destroy(int n_variables, int n_cells, struct CELL *cell)
{
	int i, j;
	for(i = 0; i < n_cells; i ++)
	{
		free(cell[i].face);
		free(cell[i].oriented);
		free(cell[i].zone);
		free(cell[i].order);
		free(cell[i].n_stencil);
		if(cell[i].stencil != NULL) for(j = 0; j < n_variables; j ++) free(cell[i].stencil[j]);
		free(cell[i].stencil);
		if(cell[i].matrix != NULL) for(j = 0; j < n_variables; j ++) { free(cell[i].matrix[j][0]); free(cell[i].matrix[j]); }
		free(cell[i].matrix);
	}
	free(cell);
}

////////////////////////////////////////////////////////////////////////////////
