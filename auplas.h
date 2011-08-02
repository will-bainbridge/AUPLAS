////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//return values
#define SUCCESS 1
#define ERROR -1

//maximum number of faces to a cell
#define MAX_CELL_FACES 5

//string length
#define MAX_STRING_LENGTH 128

//accuracy
#define EPS 1e-15

////////////////////////////////////////////////////////////////////////////////

struct NODE
{
	double x[2];
};

struct FACE
{
	struct NODE *node[2];
};

struct CELL
{
	struct FACE **face;
	int n_faces;
};

struct ZONE
{
	struct CELL *cell;
	int n_cells;
	struct FACE *face;
	int n_faces;
};

////////////////////////////////////////////////////////////////////////////////

int read_labelled_values(char *filename, char *label, char *type, void *value);
int read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);

void free_mesh_structures(int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell);

////////////////////////////////////////////////////////////////////////////////
