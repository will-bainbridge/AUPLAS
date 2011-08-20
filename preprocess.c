////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	//--------------------------------------------------------------------//

	FILE *file = fopen(input_filename,"r");
	handle(file != NULL,"opening the input file");

	char *geometry_filename, *case_filename;
	handle(allocate_character_vector(&geometry_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the geometry filename");
	handle(allocate_character_vector(&case_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the case filename");
	handle(fetch_single_value(file, "geometry_filename", 's', geometry_filename) == FETCH_SUCCESS,"reading \"geometry_filename\" from the input file");
	handle(fetch_single_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");

	int n_variables;
	handle(fetch_single_value(file, "number_of_variables", 'i', &n_variables) == FETCH_SUCCESS,"reading \"number_of_variables\" from the input file");

	fclose(file);

	//--------------------------------------------------------------------//

	int *maximum_order;
	char **connectivity;
	double *weight_exponent;

	handle(allocate_integer_vector(&maximum_order,n_variables) == ALLOCATE_SUCCESS,"allocating the maximum orders");
	handle(allocate_character_matrix(&connectivity,n_variables,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS,"allocating the connectivity");
	handle(allocate_double_vector(&weight_exponent,n_variables) == ALLOCATE_SUCCESS,"allocating the weight exponents");

	read_instructions(input_filename, n_variables, maximum_order, weight_exponent, connectivity);

	//--------------------------------------------------------------------//

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

	write_case(case_filename, n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	//--------------------------------------------------------------------//

	free_vector(geometry_filename);
	free_vector(case_filename);
	free_vector(maximum_order);
	free_vector(weight_exponent);
	free_matrix((void **)connectivity);

	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
