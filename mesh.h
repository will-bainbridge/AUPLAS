#ifndef MESH_H
#define MESH_H

#include "fetch.h"

#define MAX_FACE_NODES 2
#define MAX_CELL_FACES 5
#define MAX_STENCIL 100
#define MAX_NEIGHBOURS 20
#define MAX_INDICES 100

struct ZONE;

struct NODE
{
	double x[2];
};

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

#endif
