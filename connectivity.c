////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

void generate_connectivity(int n_variables, char **connectivity, int *maximum_order, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int c, u, z;
	int i, j, k, l;

	//generate list of cells surrounding each face
	int **face_borders;
	handle(1,allocate_integer_matrix(&face_borders,n_faces,MAX_NEIGHBOURS) == ALLOCATE_SUCCESS,"allocating list of face borders");
	for(i = 0; i < n_faces; i ++) face[i].n_borders = 0;
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_faces; j ++) face_borders[(int)(cell[i].face[j] - &face[0])][cell[i].face[j]->n_borders ++] = i;

	//allocate face borders and copy over the data
	handle(1,allocate_mesh(0, 0, NULL, n_faces, &face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS,"allocating face borders");
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_borders; j ++) face[i].border[j] = &cell[face_borders[i][j]];

	//generate list of cells surrounding each node
	int index;
	int *n_node_surround, **node_surround;
	handle(1,allocate_integer_zero_vector(&n_node_surround,n_nodes) == ALLOCATE_SUCCESS,"allocating list of the numbers of nodes surrounding cells");
	handle(1,allocate_integer_matrix(&node_surround,n_nodes,MAX_NEIGHBOURS) == ALLOCATE_SUCCESS,"allocating list of the nodes surrounding cells");

	for(c = 0; c < n_cells; c ++)
	{
		for(i = 0; i < cell[c].n_faces; i ++)
		{
			for(j = 0; j < 2; j ++)
			{
				index = (int)(cell[c].face[i]->node[j] - &node[0]);

				for(k = 0; k < n_node_surround[index]; k ++)
					if(node_surround[index][k] == c) break;

				if(k == n_node_surround[index])
					node_surround[index][n_node_surround[index]++] = c;
			}
		}
	}

	//generate lists of cells surrounding each cell
	int *n_cell_face_neighbours, **cell_face_neighbours, *n_cell_node_neighbours, **cell_node_neighbours;
	handle(1,allocate_integer_zero_vector(&n_cell_face_neighbours,n_cells) == ALLOCATE_SUCCESS,"allocating list of the numbers of cell face neighbours");
	handle(1,allocate_integer_matrix(&cell_face_neighbours,n_cells,MAX_NEIGHBOURS) == ALLOCATE_SUCCESS,"allocating list of cell face neighbours");
	handle(1,allocate_integer_zero_vector(&n_cell_node_neighbours,n_cells) == ALLOCATE_SUCCESS,"allocating list of the numbers of cell node neighbours");
	handle(1,allocate_integer_matrix(&cell_node_neighbours,n_cells,MAX_NEIGHBOURS) == ALLOCATE_SUCCESS,"allocating list of cell node neighbours");

	for(c = 0; c < n_cells; c ++)
	{
		for(i = 0; i < cell[c].n_faces; i ++)
		{
			//cells sharing a face
			for(j = 0; j < cell[c].face[i]->n_borders; j ++)
			{
				index = (int)(cell[c].face[i]->border[j] - &cell[0]);
				if(index != c) cell_face_neighbours[c][n_cell_face_neighbours[c]++] = index;
			}

			//cells sharing a node
			for(j = 0; j < 2; j ++)
			{
				index = (int)(cell[c].face[i]->node[j] - &node[0]);

				for(k = 0; k < n_node_surround[index]; k ++)
				{
					for(l = 0; l < n_cell_node_neighbours[c]; l ++)
						if(cell_node_neighbours[c][l] == node_surround[index][k]) break;

					if(l == n_cell_node_neighbours[c])
						cell_node_neighbours[c][n_cell_node_neighbours[c]++] = node_surround[index][k];
				}
			}
		}
	}

	//allocate the stencil numbers and pointers
	handle(1,allocate_mesh(n_variables, 0, NULL, 0, NULL, n_cells, &cell, 0, NULL) == ALLOCATE_SUCCESS,"allocating cell stencils");

	//generate the stencils
	int *n_cell_neighbours, **cell_neighbours;
	int old_cell, new_cell, n_stencil_cells, n_old_stencil_cells, n_stencil_faces;
	int is_variable, is_unknown, is_in_cell;
	int *stencil_cells, *stencil_faces, *stencil;

	handle(1,allocate_integer_vector(&stencil_cells,MAX_STENCIL) == ALLOCATE_SUCCESS,"allocating stencil cells");
	handle(1,allocate_integer_vector(&stencil_faces,MAX_STENCIL) == ALLOCATE_SUCCESS,"allocating stencil faces");
	handle(1,allocate_integer_vector(&stencil,MAX_STENCIL) == ALLOCATE_SUCCESS,"allocating stencil ids");

	for(c = 0; c < n_cells; c ++)
	{
		for(u = 0; u < n_variables; u ++)
		{
			//initial stencil contains the centre cell only
			n_stencil_cells = 1;
			stencil_cells[0] = c;

			//loop over each character in the connectivity string
			for(i = 0; i < strlen(connectivity[u]); i ++)
			{
				n_old_stencil_cells = n_stencil_cells;

				//use appropriate neighbour list
				if(connectivity[u][i] == 'n') {
					cell_neighbours = cell_node_neighbours;
					n_cell_neighbours = n_cell_node_neighbours;
				} else if(connectivity[u][i] == 'f') {
					cell_neighbours = cell_face_neighbours;
					n_cell_neighbours = n_cell_face_neighbours;
				} else { handle(1,0,"reconising the connectivity"); }

				//loop over and add the neighbours of all the existing stencil cells
				for(j = 0; j < n_old_stencil_cells; j ++)
				{
					old_cell = stencil_cells[j];

					for(k = 0; k < n_cell_neighbours[old_cell]; k ++)
					{
						new_cell = cell_neighbours[old_cell][k];

						for(l = 0; l < n_stencil_cells; l ++)
							if(stencil_cells[l] == new_cell) break;

						if(l == n_stencil_cells)
							stencil_cells[n_stencil_cells++] = new_cell;
					}
				}
			}

			//generate stencil faces from all faces around the current stencil cells
			n_stencil_faces = 0;
			for(i = 0; i < n_stencil_cells; i ++)
			{
				for(j = 0; j < cell[stencil_cells[i]].n_faces; j ++)
				{
					index = (int)(cell[stencil_cells[i]].face[j] - &face[0]);

					for(k = 0; k < n_stencil_faces; k ++)
						if(stencil_faces[k] == index) break;

					if(k == n_stencil_faces)
						stencil_faces[n_stencil_faces++] = index;
				}
			}

			//generate the stencil identifiers
			cell[c].n_stencil[u] = 0;
			for(i = 0; i < n_stencil_cells; i ++)
			{
				for(j = 0; j < cell[stencil_cells[i]].n_zones; j ++)
				{
					is_variable = cell[stencil_cells[i]].zone[j]->variable == u;
					is_unknown = cell[stencil_cells[i]].zone[j]->condition[0] == 'u';
					is_in_cell = c == stencil_cells[i];
					z = (int)(cell[stencil_cells[i]].zone[j] - &zone[0]);

					if(is_variable && (is_unknown || is_in_cell))
						stencil[cell[c].n_stencil[u]++] = INDEX_AND_ZONE_TO_ID(stencil_cells[i],z);
				}
			}
			for(i = 0; i < n_stencil_faces; i ++)
			{
				for(j = 0; j < face[stencil_faces[i]].n_zones; j ++)
				{
					is_variable = face[stencil_faces[i]].zone[j]->variable == u;
					is_unknown = face[stencil_faces[i]].zone[j]->condition[0] == 'u';

					is_in_cell = 0;
					for(k = 0; k < cell[c].n_faces; k ++)
						is_in_cell = is_in_cell || (int)(cell[c].face[k] - &face[0]) == stencil_faces[i];

					z = (int)(face[stencil_faces[i]].zone[j] - &zone[0]);

					if(is_variable && (is_unknown || is_in_cell))
						stencil[cell[c].n_stencil[u]++] = INDEX_AND_ZONE_TO_ID(stencil_faces[i],z);
				}
			}

			//allocate and store the stencils in the cell structure
			handle(1,allocate_mesh(n_variables, 0, NULL, 0, NULL, n_cells, &cell, 0, NULL),"allocating a cell stencil");
			for(i = 0; i < cell[c].n_stencil[u]; i ++) cell[c].stencil[u][i] = stencil[i];

			//generate the cell orders
			cell[c].order[u] = MIN(maximum_order[u],floor(-1.5 + sqrt(2.0*cell[c].n_stencil[u] + 0.25)));
		}
	}

	//clean up
	free_matrix((void**)face_borders);
	free_vector(n_node_surround);
	free_matrix((void**)node_surround);
	free_vector(n_cell_face_neighbours);
	free_matrix((void**)cell_face_neighbours);
	free_vector(n_cell_node_neighbours);
	free_matrix((void**)cell_node_neighbours);
	free_vector(stencil_cells);
	free_vector(stencil_faces);
	free_vector(stencil);
}

////////////////////////////////////////////////////////////////////////////////
