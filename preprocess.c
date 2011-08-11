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

	//instructions
	int n_variables = 0, *maximum_order = NULL;
	char *geometry_filename = NULL, **connectivity = NULL;
	double *weight_exponent = NULL;
	if(read_instructions(input_filename, &n_variables, &geometry_filename, &maximum_order, &weight_exponent, &connectivity) != SUCCESS)
	{ printf("\nERROR - reading instructions\n\n"); return ERROR; }

	//geometry
	int n_nodes = 0, n_faces = 0, n_cells = 0;
	struct NODE *node = NULL;
	struct FACE *face = NULL;
	struct CELL *cell = NULL;
	if(read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell) != SUCCESS)
	{ printf("\nERROR - reading geometry file\n\n"); return ERROR; }

	//read the input file for the zones
	int n_zones = 0;
	struct ZONE *zone = NULL;
	if(read_zones(input_filename, n_faces, face, n_cells, cell, &n_zones, &zone) != SUCCESS)
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
	free_instructions(n_variables, geometry_filename, maximum_order, weight_exponent, connectivity);
	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
