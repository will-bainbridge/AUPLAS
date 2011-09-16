////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

#define GNUPLOT 0
#define VTK 1

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	exit_if_false(argc > 2,"wrong number of input arguments");
	char *input_filename = argv[1];

	FILE *file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");
	char *case_filename;
	exit_if_false(allocate_character_vector(&case_filename,MAX_STRING_LENGTH),"allocating the case filename");
	exit_if_false(fetch_value(file,"case_filename",'s',case_filename)==FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	char *output_filename;
	exit_if_false(allocate_character_vector(&output_filename,MAX_STRING_LENGTH),"allocating the data filename");
	exit_if_false(fetch_value(file,"output_filename",'s',output_filename)==FETCH_SUCCESS,"reading \"output_filename\" from the input file");
	fclose(file);

	int output_type;
	if(strcmp(&output_filename[strlen(output_filename)-8],".gnuplot") == 0) output_type = GNUPLOT;
	else if(strcmp(&output_filename[strlen(output_filename)-4],".vtu"    ) == 0) output_type = VTK;
	else exit_if_false(0,"recognising output format");

	int n_variables, n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone);

	file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");
	char **variable_name;
	exit_if_false(allocate_character_matrix(&variable_name,n_variables,MAX_STRING_LENGTH),"allocating variable names");
	warn_if_false(fetch_vector(file,"variable_names",'s',n_variables,variable_name) == FETCH_SUCCESS,"reading \"variable_names\" from the input file");
	fclose(file);

	int n_unknowns, *unknown_to_id;
	generate_lists_of_unknowns(&n_unknowns, &unknown_to_id, n_variables, n_faces, face, n_cells, cell, n_zones, zone);

	double *x;
	exit_if_false(allocate_double_vector(&x,n_unknowns),"allocating unknown vector");

	int i;
	double time;
	for(i = 2; i < argc; i ++)
	{
		printf("\npost-processing \"%s\"",argv[i]);
		read_data(argv[i], &time, n_unknowns, x);

		if(output_type == GNUPLOT)
		{
			write_gnuplot(output_filename, time, n_variables, variable_name, n_unknowns, unknown_to_id, x, n_faces, face, n_cells, cell, n_zones, zone);
		}

		if(output_type == VTK)
		{
			write_vtk(output_filename, time, n_variables, variable_name, n_unknowns, unknown_to_id, x, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);
		}
	}

	printf("\n\n");
	
	free_vector(case_filename);
	free_vector(output_filename);
	free_matrix((void**)variable_name);
	free_vector(unknown_to_id);
	free_vector(x);
	nodes_destroy(n_nodes,node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
