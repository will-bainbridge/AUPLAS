#ifndef NODE_H
#define NODE_H

struct NODE
{
	double x[2];
};

struct NODE * nodes_new(int n_nodes, struct NODE *node);
void node_geometry_get(FILE *file, struct NODE *node);
void nodes_destroy(struct NODE *node);

#endif
