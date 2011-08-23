////////////////////////////////////////////////////////////////////////////////

#ifndef MESH_H
#define MESH_H

#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

//maximum extents for memory allocation
#define MAX_FACE_NODES 2
#define MAX_CELL_FACES 5
#define MAX_STENCIL 100
#define MAX_NEIGHBOURS 20
#define MAX_ZONES 100
#define MAX_CONDITION_CHARACTERS 8
#define MAX_INDICES 100

////////////////////////////////////////////////////////////////////////////////

struct NODE
{
	double x[2];
};

////////////////////////////////////////////////////////////////////////////////

struct FACE
{
	int n_nodes;
	struct NODE **node;

	int n_borders;
	struct CELL **border;
	int *oriented;

	int n_zones;
	struct ZONE **zone;

	double centroid[2];
};

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

struct ZONE
{
	char location;
	int variable;
	char condition[MAX_CONDITION_CHARACTERS];
	double value;
};

struct ZONE * zones_new(struct ZONE *zone, int n_zones);
void zones_read(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone);
void zones_destroy(struct ZONE *zone);

////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////
