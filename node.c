////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

struct NODE * nodes_new(int n_nodes, struct NODE *node)
{
        node = (struct NODE *)realloc(node, n_nodes * sizeof(struct NODE));
	if(node == NULL) return NULL;

	int i;
	for(i = 0; i < n_nodes; i ++)
	{
		node[i].n_borders = 0;
		node[i].border = NULL;
	}

	return node;
}

////////////////////////////////////////////////////////////////////////////////

int node_border_new(struct NODE *node)
{
	node->border = (struct CELL **)realloc(node->border, node->n_borders * sizeof(struct CELL *));
	if(node->border == NULL) return 0;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////

void node_geometry_get(FILE *file, struct NODE *node)
{
	int info = fscanf(file,"%lf %lf\n",&(node->x[0]),&(node->x[1]));
	handle(1,info == 2, "reading a node's coordinates");
}

////////////////////////////////////////////////////////////////////////////////

void node_case_write(FILE *file, struct NODE *node)
{
	handle(1,fwrite(node->x, sizeof(double), 2, file) == 2, "writing the node location");
}

void node_case_get(FILE *file, struct NODE *node)
{
	handle(1,fread(node->x, sizeof(double), 2, file) == 2, "reading the node location");
}

////////////////////////////////////////////////////////////////////////////////

void node_generate_border(struct NODE *node, struct CELL *cell)
{
	int i;
	for(i = 0; i < node->n_borders; i ++) if(node->border[i] == cell) return;

	node->n_borders ++;
	handle(1,node_border_new(node),"allocating node border");

	node->border[node->n_borders - 1] = cell;
}

struct CELL ** node_add_borders_to_list(struct NODE *node, int *n_list, struct CELL **list)
{
	int i, j, n_add = 0;

	int *add = (int *)malloc(node->n_borders * sizeof(int));
	handle(1,add != NULL,"allocating temporary storage");

	for(i = 0; i < node->n_borders; i ++)
	{
		add[i] = 1;

		for(j = 0; j < *n_list; j ++)
		{
			if(list[j] == node->border[i])
			{
				add[i] = 0;
				break;
			}
		}

		n_add += add[i];
	}

	if(n_add > 0) 
	{
		list = (struct CELL **)realloc(list, (*n_list + n_add)*sizeof(struct CELL *));
		handle(1,list != NULL,"re-allocating list");

		for(i = 0; i < node->n_borders; i ++)
		{
			if(add[i])
			{
				list[(*n_list) ++] = node->border[i];
			}
		}
	}

	free(add);

	return list;
}

////////////////////////////////////////////////////////////////////////////////

void nodes_destroy(int n_nodes, struct NODE *node)
{
	int i;
	for(i = 0; i < n_nodes; i ++)
	{
		free(node[i].border);
	}

	free(node);
}

////////////////////////////////////////////////////////////////////////////////
