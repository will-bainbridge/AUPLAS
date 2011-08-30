////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(1,argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	//--------------------------------------------------------------------//

	FILE *file = fopen(input_filename,"r");
	handle(1,file != NULL,"opening the input file");

	int n_variables;
	handle(1,fetch_value(file, "number_of_variables", 'i', &n_variables) == FETCH_SUCCESS,"reading \"number_of_variables\" from the input file");

	char *geometry_filename;
	handle(1,allocate_character_vector(&geometry_filename,MAX_STRING_LENGTH),"allocating the geometry filename");
	handle(1,fetch_value(file, "geometry_filename", 's', geometry_filename) == FETCH_SUCCESS,"reading \"geometry_filename\" from the input file");
	
	char *case_filename;
	handle(1,allocate_character_vector(&case_filename,MAX_STRING_LENGTH),"allocating the case filename");
	handle(1,fetch_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");

	int *maximum_order;
	handle(1,allocate_integer_vector(&maximum_order,n_variables),"allocating the maximum orders");
	handle(1,fetch_vector(file,"maximum_order",'i',n_variables,maximum_order) == FETCH_SUCCESS,"reading \"maximum_order\" from the input file");
	{ int i; for(i = 0; i < n_variables; i ++) maximum_order[i] -= 1; }

	char **connectivity;
	handle(1,allocate_character_matrix(&connectivity,n_variables,MAX_STRING_LENGTH),"allocating the connectivity");
	handle(1,fetch_vector(file,"connectivity",'s',n_variables,connectivity) == FETCH_SUCCESS,"reading \"connectivity\" from the input file");

	double *weight_exponent;
	handle(1,allocate_double_vector(&weight_exponent,n_variables),"allocating the weight exponents");
	handle(1,fetch_vector(file,"weight_exponent",'d',n_variables,weight_exponent) == FETCH_SUCCESS,"reading \"weight_exponent\" from the input file");

	fclose(file);

	//--------------------------------------------------------------------//
	
	int n_nodes, n_faces, n_cells;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell);

	int n_zones;
	struct ZONE *zone;
	zones_input(input_filename, n_faces, face, n_cells, cell, &n_zones, &zone);

	/*{
		int i;

		for(i = 0; i < n_cells; i ++) cell_generate_border(&cell[i]);

		for(i = 0; i < n_cells; i ++)
			cell_generate_stencil(&cell[i], n_variables, maximum_order, connectivity, &face[0], &cell[0], &zone[0]);
	}*/

	generate_borders(n_cells, cell);

	generate_stencils(n_variables, connectivity, maximum_order, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);
	
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

	nodes_destroy(n_nodes,node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
