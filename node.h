#ifndef NODE_H
#define NODE_H

struct NODE
{
	double x[2];

	int n_borders;
	struct CELL **border;
};

struct NODE * nodes_new(int n_nodes, struct NODE *node);
void node_geometry_get(FILE *file, struct NODE *node);
void node_case_write(FILE *file, struct NODE *node);
void node_case_get(FILE *file, struct NODE *node);
void node_generate_border(struct NODE *node, struct CELL *cell);
struct CELL ** node_add_borders_to_list(struct NODE *node, int *n_list, struct CELL **list);
void nodes_destroy(int n_nodes, struct NODE *node);

#endif
