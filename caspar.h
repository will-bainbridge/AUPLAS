////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//return values
#define SUCCESS 1
#define ERROR -1

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
	struct NODE node[2];
};

struct CELL
{
	struct FACE *face;
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

int read_labelled_values_from_file(FILE *file, char *label, char type, int n, void *value);
int read_geometry_file(char *filename, int *n_nodes, struct NODE *node, int *n_faces, struct FACE *face, int *n_cells, struct CELL *cell);

////////////////////////////////////////////////////////////////////////////////
