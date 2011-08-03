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

//maximum number of zones
#define MAX_N_ZONES 100

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
	struct CELL **border;
	int n_borders;
};

struct CELL
{
	struct FACE **face;
	struct CELL **border;
	int n_faces;
};

struct ZONE
{
	int id;
	char location;
	int *index;
	int variable;
	char condition[MAX_STRING_LENGTH];
	double value;
};

////////////////////////////////////////////////////////////////////////////////

int read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
int read_zones(char *filename, int *n_zones, struct ZONE **zone);

int generate_connectivity(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);

int generate_face_borders(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);

void free_mesh_structures(int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell);

////////////////////////////////////////////////////////////////////////////////
