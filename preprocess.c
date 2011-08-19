////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	char *geometry_filename, *case_filename;
	handle(allocate_character_vector(&geometry_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the geometry filename");
	handle(allocate_character_vector(&case_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the case filename");

	FILE *file = fopen(input_filename,"r");
	handle(file != NULL, "opening the input file");
	fetch input = fetch_new("s",1);
	handle(input != NULL,"allocating filename inputs");
	handle(fetch_read(file, "geometry_filename", input) == 1,"reading \"geometry_filename\" from the input file");
	fetch_get(input, 0, 0, geometry_filename);
	handle(fetch_read(file, "case_filename", input) == 1,"reading \"case_filename\" from the input file");
	fetch_get(input, 0, 0, case_filename);
	fclose(file);
	fetch_destroy(input);

	int n_variables = 0, *maximum_order = NULL;
	char **connectivity = NULL;
	double *weight_exponent = NULL;
	read_instructions(input_filename, &n_variables, &maximum_order, &weight_exponent, &connectivity);

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

	free(geometry_filename);
	free(case_filename);
	free_instructions(n_variables, maximum_order, weight_exponent, connectivity);
	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
