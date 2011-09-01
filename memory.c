////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

struct NODE * nodes_new(int n_nodes, struct NODE *node)
{
        node = (struct NODE *)realloc(node, n_nodes * sizeof(struct NODE));
        if(node == NULL) return NULL;

        int i;
        for(i = 0; i < n_nodes; i ++)
        {
                node[i].n_borders = 0;
                node[i].border = NULL;
        }

        return node;
}

int node_border_new(struct NODE *node)
{
        node->border = (struct CELL **)realloc(node->border, node->n_borders * sizeof(struct CELL *));
        if(node->border == NULL) return 0;

        return 1;
}

void nodes_destroy(int n_nodes, struct NODE *node)
{
        int i;
        for(i = 0; i < n_nodes; i ++)
        {
                free(node[i].border);
        }

        free(node);
}

////////////////////////////////////////////////////////////////////////////////

struct FACE * faces_new(int n_faces, struct FACE *face)
{
        face = (struct FACE *)realloc(face, n_faces * sizeof(struct FACE));
        if(face == NULL) return NULL;

        int i;
        for(i = 0; i < n_faces; i ++)
        {
                face[i].n_nodes = 0;
                face[i].node = NULL;
                face[i].n_borders = 0;
                face[i].border = NULL;
                face[i].oriented = NULL;
                face[i].n_zones = 0;
                face[i].zone = NULL;
        }

        return face;
}

int face_node_new(struct FACE *face)
{
        face->node = (struct NODE **)realloc(face->node, face->n_nodes * sizeof(struct NODE *));
        if(face->node == NULL) return 0;

        return 1;
}

int face_border_new(struct FACE *face)
{
        face->border = (struct CELL **)realloc(face->border, face->n_borders * sizeof(struct CELL *));
        if(face->border == NULL) return 0;

        return 1;
}

int face_oriented_new(struct FACE *face)
{
        face->oriented = (int *)realloc(face->oriented, face->n_borders * sizeof(int));
        if(face->oriented == NULL) return 0;

        return 1;
}

int face_zone_new(struct FACE *face)
{
        face->zone = (struct ZONE **)realloc(face->zone, face->n_zones * sizeof(struct ZONE *));
        if(face->zone == NULL) return 0;

        return 1;
}

int face_zone_add(struct FACE *face, struct ZONE *zone)
{
        face->zone = (struct ZONE **)realloc(face->zone, (face->n_zones + 1) * sizeof(struct ZONE *));
        if(face->zone == NULL) return 0;

        face->zone[face->n_zones ++] = zone;

        return 1;
}

void faces_destroy(int n_faces, struct FACE *face)
{
        int i;
        for(i = 0; i < n_faces; i ++)
        {
                free(face[i].node);
                free(face[i].border);
                free(face[i].oriented);
                free(face[i].zone);
        }
        free(face);
}

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

int cell_zone_add(struct CELL *cell, struct ZONE *zone)
{
        cell->zone = (struct ZONE **)realloc(cell->zone, (cell->n_zones + 1) * sizeof(struct ZONE *));
        if(cell->zone == NULL) return 0;

        cell->zone[cell->n_zones ++] = zone;

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

struct ZONE * zones_new(int n_zones, struct ZONE *zone)
{
        int new = zone == NULL;

        zone = (struct ZONE *)realloc(zone, n_zones * sizeof(struct ZONE));
        if(zone == NULL) return NULL;

        if(new)
        {
                int i;
                for(i = 0; i < n_zones; i ++)
                {
                        zone[i].location = '\0';
                        zone[i].variable = -1;
                        memset(zone[i].condition,'\0',MAX_CONDITION_LENGTH);
                        zone[i].value = 0.0;
                }
        }

        return zone;
}

void zones_destroy(struct ZONE *zone)
{
        free(zone);
}

////////////////////////////////////////////////////////////////////////////////

struct DIVERGENCE * divergences_new(struct DIVERGENCE *divergence, int n_old, int n_new)
{
        int i;

        for(i = n_new; i < n_old; i ++)
        {
                free(divergence[i].variable);
                free(divergence[i].differential);
        }

        divergence = (struct DIVERGENCE *)realloc(divergence, n_new * sizeof(struct DIVERGENCE));
        if(divergence == NULL) return NULL;

        for(i = n_old; i < n_new; i ++)
        {
                divergence[i].n_variables = 0;
                divergence[i].variable = NULL;
                divergence[i].differential = NULL;
        }

        return divergence;
}

void divergences_destroy(int n_divergences, struct DIVERGENCE *divergence)
{
        int i;
        for(i = 0; i < n_divergences; i ++)
        {
                free(divergence[i].variable);
                free(divergence[i].differential);
        }
        free(divergence);
}

////////////////////////////////////////////////////////////////////////////////

int allocate_integer_vector(int **vector, int length)
{
	*vector = (int *)malloc(length * sizeof(int));
	if(*vector == NULL) return 0;
	return 1;
}

int allocate_integer_zero_vector(int **vector, int length)
{
        *vector = (int *)calloc(length, sizeof(int));
        if(*vector == NULL) return 0;
        return 1;
}

int allocate_double_vector(double **vector, int length)
{
	*vector = (double *)malloc(length * sizeof(double));
	if(*vector == NULL) return 0;
	return 1;
}

int allocate_character_vector(char **vector, int length)
{
	*vector = (char *)malloc(length * sizeof(char));
	if(*vector == NULL) return 0;
	return 1;
}

void free_vector(void *vector)
{
	free(vector);
}

int allocate_integer_matrix(int ***matrix, int height, int width)
{
	*matrix = (int **)malloc(height * sizeof(int *));
	if(*matrix == NULL) return 0;
	(*matrix)[0] = (int *)malloc(height * width * sizeof(int));
	if((*matrix)[0] == NULL) return 0;
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return 1;
}

int allocate_integer_zero_matrix(int ***matrix, int height, int width)
{
        *matrix = (int **)malloc(height * sizeof(int *));
        if(*matrix == NULL) return 0;
        (*matrix)[0] = (int *)calloc(height * width, sizeof(int));
        if((*matrix)[0] == NULL) return 0;
        int i;
        for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
        return 1;
}

int allocate_double_matrix(double ***matrix, int height, int width)
{
	*matrix = (double **)malloc(height * sizeof(double *));
	if(*matrix == NULL) return 0;
	(*matrix)[0] = (double *)malloc(height * width * sizeof(double));
	if((*matrix)[0] == NULL) return 0;
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return 1;
}

int allocate_double_pointer_matrix(double ****matrix, int height, int width)
{
	*matrix = (double ***)malloc(height * sizeof(double **));
	if(*matrix == NULL) return 0;
	(*matrix)[0] = (double **)malloc(height * width * sizeof(double *));
	if((*matrix)[0] == NULL) return 0;
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return 1;
}

int allocate_character_matrix(char ***matrix, int height, int width)
{
	*matrix = (char **)malloc(height * sizeof(char *));
	if(*matrix == NULL) return 0;
	(*matrix)[0] = (char *)malloc(height * width * sizeof(char));
	if((*matrix)[0] == NULL) return 0;
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return 1;
}

int allocate_cell_pointer_matrix(struct CELL ****matrix, int height, int width)
{
	*matrix = (struct CELL ***)malloc(height * sizeof(struct CELL **));
	if(*matrix == NULL) return 0;
	(*matrix)[0] = (struct CELL **)malloc(height * width * sizeof(struct CELL *));
	if((*matrix)[0] == NULL) return 0;
	int i;
	for (i = 1; i < height; i++) { (*matrix)[i] = (*matrix)[i-1] + width; }
	return 1;
}

void free_matrix(void **matrix)
{
	free(matrix[0]);
	free(matrix);	
}

//////////////////////////////////////////////////////////////////
