#ifndef FACE_H
#define FACE_H

#define MAX_FACE_NODES 2

struct NODE;
struct CELL;
struct ZONE;

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

struct FACE * faces_new(int n_faces, struct FACE *face);
int face_node_new(struct FACE *face);
int face_border_new(struct FACE *face);
int face_oriented_new(struct FACE *face);
int face_zone_new(struct FACE *face);
void face_geometry_get(FILE *file, struct FACE *face, struct NODE *node);
void faces_destroy(int n_faces, struct FACE *face);

#endif
