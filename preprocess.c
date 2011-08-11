////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	int n_variables = 0, *maximum_order = NULL;
	char *geometry_filename = NULL, *case_filename = NULL, **connectivity = NULL;
	double *weight_exponent = NULL;
	read_instructions(input_filename, &n_variables, &geometry_filename, &case_filename, &maximum_order, &weight_exponent, &connectivity);

	int n_nodes = 0, n_faces = 0, n_cells = 0;
	struct NODE *node = NULL;
	struct FACE *face = NULL;
	struct CELL *cell = NULL;
	read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell);

	int n_zones = 0;
	struct ZONE *zone = NULL;
	read_zones(input_filename, n_faces, face, n_cells, cell, &n_zones, &zone);

	generate_connectivity(n_variables, connectivity, maximum_order, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	generate_face_orientations(n_faces, face, n_cells, cell);
	
	calculate_control_volume_geometry(n_faces, face, n_cells, cell);

	calculate_cell_reconstruction_matrices(n_variables, weight_exponent, maximum_order, face, n_cells, cell, zone);

	//--------------------------------------------------------------------//
	int c = 5, u = 1, i, j;
	for(i = 0; i < ORDER_TO_POWERS(cell[c].order[u]); i ++) {
		for(j = 0; j < cell[c].n_stencil[u]; j ++) {
			printf("%+10.4lf ",cell[c].matrix[u][i][j]);
		}
		printf("\n");
	}
	//--------------------------------------------------------------------//

	write_case(case_filename, n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	free_instructions(n_variables, geometry_filename, case_filename, maximum_order, weight_exponent, connectivity);
	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
