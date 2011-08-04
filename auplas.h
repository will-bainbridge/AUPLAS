////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//return values
#define SUCCESS 1
#define ERROR -1

//maximum extents for memory allocation
#define MAX_CELL_FACES 5
#define MAX_NEIGHBOURS 10
#define MAX_STENCIL 100
#define MAX_ZONES 100
#define MAX_STRING_CHARACTERS 128

//accuracy
#define EPS 1e-15

//id macros
#define ID_TO_ZONE(id) ((id) % MAX_ZONES)
#define ID_TO_INDEX(id) ((id) / MAX_ZONES)
#define INDEX_AND_ZONE_TO_ID(i,z) ((i)*MAX_ZONES + (z))

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
	struct ZONE **zone;
	int n_zones;
};

struct CELL
{
	struct FACE **face;
	struct CELL **border;
	int n_faces;
	struct ZONE **zone;
	int n_zones;
};

struct ZONE
{
	int variable;
	char condition[MAX_STRING_CHARACTERS];
	double value;
};

////////////////////////////////////////////////////////////////////////////////

int read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
int read_zones(char *filename, struct FACE *face, struct CELL *cell, int *n_zones, struct ZONE **zone);

int generate_connectivity(int n_variables, char **connectivity, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

void free_mesh_structures(int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

////////////////////////////////////////////////////////////////////////////////
