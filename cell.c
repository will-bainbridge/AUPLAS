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

void cell_case_write(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, n, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	handle(1,index != NULL,"allocating temporary storage");

	handle(1,fwrite(&(cell->n_faces), sizeof(int), 1, file) == 1, "writing the number of cell faces");
	for(i = 0; i < cell->n_faces; i ++) index[i] = (int)(cell->face[i] - &face[0]);
	handle(1,fwrite(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell faces");
	handle(1,fwrite(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell orientations");
	handle(1,fwrite(cell->centroid, sizeof(double), 2, file) == 2, "writing the cell centroid");
	handle(1,fwrite(&(cell->n_zones), sizeof(int), 1, file) == 1, "writing the number of cell zones");
	for(i = 0; i < cell->n_zones; i ++) index[i] = (int)(cell->zone[i] - &zone[0]);
	handle(1,fwrite(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "writing the cell zones");
	handle(1,fwrite(cell->order, sizeof(int), n_variables, file) == n_variables, "writing the cell orders");
	handle(1,fwrite(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "writing the cell stencil sizes");
	for(i = 0; i < n_variables; i ++)
	{
		handle(1,fwrite(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"writing the cell stencil");
		n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
		handle(1,fwrite(cell->matrix[i][0], sizeof(double), n, file) == n,"writing the cell matrix");
	}

	free(index);
}

void cell_case_get(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, n, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	handle(1,index != NULL,"allocating temporary storage");

	handle(1,fread(&(cell->n_faces), sizeof(int), 1, file) == 1, "reading the number of cell faces");
	handle(1,cell_face_new(cell),"allocating cell faces");
	handle(1,fread(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading the cell faces");
	for(i = 0; i < cell->n_faces; i ++) cell->face[i] = &face[index[i]];
	handle(1,cell_oriented_new(cell),"allocating cell orientations");
	handle(1,fread(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading cell orientations");
	handle(1,fread(cell->centroid, sizeof(double), 2, file) == 2, "reading the cell centroid");
	handle(1,fread(&(cell->n_zones), sizeof(int), 1, file) == 1, "reading the number of cell zones");
	handle(1,cell_zone_new(cell),"allocating cell zones");
	handle(1,fread(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "reading the cell zones");
	for(i = 0; i < cell->n_zones; i ++) cell->zone[i] = &zone[index[i]];
	handle(1,cell_order_new(n_variables,cell),"allocating cell orders");
	handle(1,fread(cell->order, sizeof(int), n_variables, file) == n_variables, "reading the cell orders");
	handle(1,cell_n_stencil_new(n_variables,cell),"allocating cell stencil sizes");
	handle(1,fread(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "reading the cell stencil sizes");
	handle(1,cell_stencil_new(n_variables,cell),"allocating cell stencils");
	handle(1,cell_matrix_new(n_variables,cell),"allocating cell matrices");
	for(i = 0; i < n_variables; i ++)
	{
		handle(1,fread(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"reading the cell stencil");
		n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
		handle(1,fread(cell->matrix[i][0], sizeof(double), n, file) == n,"reading the cell matrix");
	}

	free(index);
}

////////////////////////////////////////////////////////////////////////////////

void cell_generate_border(struct CELL *cell)
{
	int i;
	for(i = 0; i < cell->n_faces; i ++)
	{
		face_generate_border(cell->face[i],cell);
	}
}

void cell_generate_stencil(struct CELL *cell, int n_variables, int *maximum_order, char **connectivity, struct FACE *face_zero, struct CELL *cell_zero, struct ZONE *zone_zero)
{
	int u, z, i, j, k;

	int n_stencil_cells, n_existing_stencil_cells, n_stencil_faces, n_max_stencil_faces;
	struct CELL **stencil_cell = NULL;
	struct FACE **stencil_face = NULL;
	int *stencil = NULL;

	int add;
	int is_variable, is_unknown, is_in_cell;

	handle(1,cell_n_stencil_new(n_variables,cell),"allocating cell stencil numbers");
	handle(1,cell_order_new(n_variables,cell),"allocating a cell orders");

	for(u = 0; u < n_variables; u ++)
	{
		//initial stencil contains the centre cell only
		n_stencil_cells = 1;
		stencil_cell = (struct CELL **)realloc(stencil_cell, 1 * sizeof(struct CELL *));
		handle(1,stencil_cell != 0,"allocating stencil cell list");
		stencil_cell[0] = cell;

		//loop over each character in the connectivity string
		for(i = 0; i < strlen(connectivity[u]); i ++)
		{
			//loop over and add the neighbours of all the existing stencil cells
			n_existing_stencil_cells = n_stencil_cells;
			for(j = 0; j < n_existing_stencil_cells; j ++)
			{
				for(k = 0; k < stencil_cell[j]->n_faces; k ++)
				{
					if(connectivity[u][i] == 'n')
					{
						stencil_cell = face_add_node_borders_to_list(stencil_cell[j]->face[k],
								&n_stencil_cells, stencil_cell);
					}
					else if(connectivity[u][i] == 'f')
					{
						stencil_cell = face_add_face_borders_to_list(stencil_cell[j]->face[k],
								&n_stencil_cells, stencil_cell);
					}
					else { handle(1,0,"reconising the connectivity"); }
				}
			}
		}

		//generate stencil faces from all faces around the current stencil cells
		n_max_stencil_faces = 0;
		for(i = 0; i < n_stencil_cells; i ++) for(j = 0; j < stencil_cell[i]->n_faces; j ++) n_max_stencil_faces ++;

		stencil_face = (struct FACE **)realloc(stencil_face, n_max_stencil_faces * sizeof(struct FACE *));
		handle(1,stencil_face != 0,"allocating stencil face list");

		n_stencil_faces = 0;
		for(i = 0; i < n_stencil_cells; i ++)
		{
			for(j = 0; j < stencil_cell[i]->n_faces; j ++)
			{
				add = 1;

				for(k = 0; k < n_stencil_faces; k ++)
				{
					if(stencil_cell[i]->face[j] == stencil_face[k])
					{
						add = 0;
						break;
					}
				}

				if(add) stencil_face[n_stencil_faces ++] = stencil_cell[i]->face[j];
			}
		}

		//generate the stencil identifiers
		cell->n_stencil[u] = 0;

		stencil = (int *)realloc(stencil, (n_stencil_cells + n_stencil_faces) * sizeof(int));
		handle(1,stencil != NULL,"allocating temporary stencil");

		for(i = 0; i < n_stencil_cells; i ++)
		{
			is_in_cell = cell == stencil_cell[i];

			for(j = 0; j < stencil_cell[i]->n_zones; j ++)
			{
				is_variable = stencil_cell[i]->zone[j]->variable == u;
				is_unknown = stencil_cell[i]->zone[j]->condition[0] == 'u';

				z = (int)(stencil_cell[i]->zone[j] - zone_zero);

				if(is_variable && (is_unknown || is_in_cell))
					stencil[cell->n_stencil[u]++] = INDEX_AND_ZONE_TO_ID((int)(stencil_cell[i] - cell_zero),z);
			}
		}

		for(i = 0; i < n_stencil_faces; i ++)
		{
			is_in_cell = 0;
			for(k = 0; k < cell->n_faces; k ++) is_in_cell = is_in_cell || cell->face[k] == stencil_face[i];

			for(j = 0; j < stencil_face[i]->n_zones; j ++) //FACE NOT ABSTRACTED
			{
				is_variable = stencil_face[i]->zone[j]->variable == u;
				is_unknown = stencil_face[i]->zone[j]->condition[0] == 'u';

				z = (int)(stencil_face[i]->zone[j] - zone_zero);

				if(is_variable && (is_unknown || is_in_cell))
					stencil[cell->n_stencil[u]++] = INDEX_AND_ZONE_TO_ID((int)(stencil_face[i] - face_zero),z);
			}
		}

		//allocate and store the stencils in the cell structure
		handle(1,cell_stencil_new(n_variables,cell),"allocating a cell stencil");
		for(i = 0; i < cell->n_stencil[u]; i ++) cell->stencil[u][i] = stencil[i];

		//generate the order
		cell->order[u] = MIN(maximum_order[u],floor(-1.5 + sqrt(2.0*cell->n_stencil[u] + 0.25)));
	}

	free(stencil_cell);
	free(stencil_face);
	free(stencil);
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

		if(cell[i].stencil != NULL)
		{
			for(j = 0; j < n_variables; j ++)
			{
				free(cell[i].stencil[j]);
			}
		}

		free(cell[i].stencil);

		if(cell[i].matrix != NULL)
		{
			for(j = 0; j < n_variables; j ++)
			{
				if(cell[i].matrix[j] != NULL)
				{
					free(cell[i].matrix[j][0]);
					free(cell[i].matrix[j]);
				}
			}
		}

		free(cell[i].matrix);
	}
	free(cell);
}

////////////////////////////////////////////////////////////////////////////////
