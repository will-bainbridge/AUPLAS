#ifndef CELL_H
#define CELL_H

#define MAX_CELL_FACES 5
#define MAX_STENCIL 100
#define MAX_NEIGHBOURS 20

struct FACE;
struct ZONE;

struct CELL
{
	int n_faces;
	struct FACE **face;
	int *oriented;

	int n_zones;
	struct ZONE **zone;

	double centroid[2];

	int *order;
	int *n_stencil;
	int **stencil;
	double ***matrix;
};

struct CELL * cells_new(int n_cells, struct CELL *cell);
int cell_face_new(struct CELL *cell);
int cell_oriented_new(struct CELL *cell);
int cell_zone_new(struct CELL *cell);
int cell_order_new(int n_variables, struct CELL *cell);
int cell_n_stencil_new(int n_variables, struct CELL *cell);
int cell_stencil_new(int n_variables, struct CELL *cell);
int cell_matrix_new(int n_variables, struct CELL *cell);
void cell_geometry_get(FILE *file, struct CELL *cell, struct FACE *face);
void cells_destroy(int n_variables, int n_cells, struct CELL *cell);

#endif