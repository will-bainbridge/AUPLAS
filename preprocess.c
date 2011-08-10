////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	//input arguments
	if(argc != 2) { printf("\nERROR - need exactly one argument, the input fileame\n\n"); return ERROR; }
	char *input_filename = argv[1];

	//read the input file fo instructions
	int n_variables, *maximum_order;
	char *geometry_filename, **connectivity;
	double *weight_exponent;
	if(read_instructions(input_filename, &n_variables, &geometry_filename, &maximum_order, &weight_exponent, &connectivity) != SUCCESS)
	{ printf("\nERROR - reading instructions\n\n"); return ERROR; }

	//read the geometry file
	int n_nodes, n_faces, n_cells;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	if(read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell) != SUCCESS)
	{ printf("\nERROR - reading geometry file\n\n"); return ERROR; }

	//read the input file for the zones
	int n_zones;
	struct ZONE *zone;
	if(read_zones(input_filename, face, cell, &n_zones, &zone) != SUCCESS)
	{ printf("\nERROR - reading zones\n\n"); return ERROR; }

	//generate connectivity between the mesh structures
	if(generate_connectivity(n_variables, connectivity, maximum_order, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone) != SUCCESS)
	{ printf("\nERROR - generating connectivity\n\n"); return ERROR; }

	//generate cell face orientations
	if(generate_face_orientations(n_faces, face, n_cells, cell) != SUCCESS)
	{ printf("\nERROR - generating orientations\n\n"); return ERROR; }
	
	//calculate CV centroids
	if(calculate_control_volume_geometry(n_faces, face, n_cells, cell) != SUCCESS)
	{ printf("\nERROR - calculating control volume geometry\n\n"); return ERROR; }

	//calculate cell reconstruction matrices
	if(calculate_cell_reconstruction_matrices(n_variables, weight_exponent, maximum_order, face, n_cells, cell, zone) != SUCCESS)
	{ printf("\nERROR - calculating reconstruction matrices\n\n"); return ERROR; }

	int c = 5, u = 1, i, j;

	for(i = 0; i < ORDER_TO_POWERS(cell[c].order[u]); i ++) {
		for(j = 0; j < cell[c].n_stencil[u]; j ++) {
			printf("%+10.4lf ",cell[c].matrix[u][i][j]);
		}
		printf("\n");
	}


	//--------------------------------------------------------------------//

	//clean up
	free_vector(geometry_filename);
	free_vector(maximum_order);
	free_vector(weight_exponent);
	free_matrix((void*)connectivity);
	free_mesh_structures(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void free_mesh_structures(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i, j;

	free(node);

	for(i = 0; i < n_faces; i ++)
	{
		free(face[i].border);
		free(face[i].oriented);
		free(face[i].zone);
	}
	free(face);

	for(i = 0; i < n_cells; i ++)
	{
		free(cell[i].face);
		free(cell[i].oriented);
		free(cell[i].zone);
		free(cell[i].n_stencil);
		for(j = 0; j < n_variables; j ++) free_vector(cell[i].stencil[j]);
		free(cell[i].stencil);
		for(j = 0; j < n_variables; j ++) free_matrix((void**)cell[i].matrix[j]);
		free(cell[i].matrix);
		free(cell[i].order);
	}
	free(cell);

	free(zone);
}

////////////////////////////////////////////////////////////////////////////////
