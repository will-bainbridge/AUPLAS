////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	exit_if_false(argc == 2,"wrong number of input arguments");
	char *input_filename = argv[1];

	printf("\nreading case filename from the input file");
	char *case_filename;
	FILE *file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");
	exit_if_false(allocate_character_vector(&case_filename,MAX_STRING_LENGTH),"allocating the case filename");
	exit_if_false(fetch_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	fclose(file);

	printf("\nreading the mesh and zones from the case file ...");
	int n_variables, n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	print_time(" done in %lf seconds",read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone));

	printf("\nreading control from the input file");
	file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");
	int n_iterations_per_step;
	exit_if_false(fetch_value(file,"iterations_per_step",'i',&n_iterations_per_step) == FETCH_SUCCESS,"reading \"iterations_per_step\" from the input file");
	char **variable_name;
	exit_if_false(allocate_character_matrix(&variable_name,n_variables,MAX_STRING_LENGTH),"allocating variable names");
	warn_if_false(fetch_vector(file,"variable_names",'s',n_variables,variable_name) == FETCH_SUCCESS,"reading \"variable_names\" from the input file");
	fclose(file);

	printf("\nreading divergences from the input file ...");
	int n_divergences;
	struct DIVERGENCE *divergence;
	print_time(" done in %lf seconds",divergences_input(input_filename,&n_divergences,&divergence));

	printf("\ngenerating lists of unknowns ...");
	int n_ids, *id_to_unknown, n_unknowns, *unknown_to_id;
	print_time(" done in %lf seconds",generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\nallocating and initialising the unknowns ...");
	double *x, *x1, *b, *residual;
	exit_if_false(allocate_double_vector(&x,n_unknowns),"allocating system old unknown vector");
	exit_if_false(allocate_double_vector(&x1,n_unknowns),"allocating system new unknownvector");
	exit_if_false(allocate_double_vector(&b,n_unknowns),"allocating system right hand side vector");
	exit_if_false(allocate_double_vector(&residual,n_variables),"allocating residuals");
	print_time(" done in %lf seconds",initialise_unknowns(n_ids, id_to_unknown, zone, x1));

	printf("\nassembling the system matrix ...");
	CSR matrix = csr_new();
	print_time(" done in %lf seconds",assemble_matrix(matrix, n_variables, id_to_unknown, n_unknowns, unknown_to_id, face, cell, zone));

	{
		int i, j;

		printf("\n\niteration > assembly solution >");
		for(i = 0; i < n_variables; i ++) printf(" %-15s",variable_name[i]);

		for(i = 1; i <= n_iterations_per_step; i ++)
		{
			for(j = 0; j < n_unknowns; j ++) x[j] = x1[j];

			printf("\n%9i >",i);

			print_time(" %7.3lfs", calculate_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, x, b,
						face, cell, zone, n_divergences, divergence));

			print_time(" %7.3lfs", exit_if_false(csr_solve_ilupack(matrix, x1, b) == CSR_SUCCESS,"solving the system"));

			printf(" >");

			calculate_residuals(n_variables, n_unknowns, unknown_to_id, x, x1, residual, n_zones, zone);

			for(j = 0; j < n_variables; j ++) printf(" %15.9e",residual[j]);
		}
	}

	printf("\n\nwriting out plot data ...");
	print_time(" done in %lf seconds",write_gnuplot(n_unknowns, unknown_to_id, x, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\ncleaning up");
	free_vector(case_filename);
	free_matrix((void**)variable_name);
	free_vector(id_to_unknown);
	free_vector(unknown_to_id);
	free_vector(x);
	free_vector(x1);
	free_vector(b);
	free_vector(residual);
	nodes_destroy(n_nodes,node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);
	divergences_destroy(n_divergences, divergence);
	csr_destroy(matrix);

	print_end();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
