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

	printf("\nreading divergences from the input file ...");
	int n_divergences;
	struct DIVERGENCE *divergence;
	print_time(" done in %lf seconds",divergences_input(input_filename,&n_divergences,&divergence));

	printf("\nreading the mesh and zones from the case file ...");
	int n_variables, n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	print_time(" done in %lf seconds",read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone));

	printf("\ngenerating lists of unknowns ...");
	int n_ids, *id_to_unknown, n_unknowns, *unknown_to_id;
	print_time(" done in %lf seconds",generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\nallocating and initialising the system ...");
	double *x, *x1, *residual;
	CSR matrix = csr_new();
	exit_if_false(allocate_double_vector(&x,n_unknowns),"allocating system left hand side vector");
	exit_if_false(allocate_double_vector(&x1,n_unknowns),"allocating system right hand side vector");
	exit_if_false(allocate_double_vector(&residual,n_variables),"allocating residuals");
	print_time(" done in %lf seconds",initialise_unknowns(n_ids, id_to_unknown, zone, x));

	{
		int i, j, iterations_per_step = 20;

		printf("\niteration > assembly solution >");
		for(i = 0; i < n_variables; i ++) printf(" variable-%-2i     ",i);

		for(i = 0; i < iterations_per_step; i ++)
		{
			printf("\n%9i >",i); fflush(stdout);

			print_time(" %7.3lfs", assemble_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, x, x1,
						n_faces, face, n_cells, cell, n_zones, zone, n_divergences, divergence));

			print_time(" %7.3lfs >", exit_if_false(csr_solve_umfpack(matrix, x1) == CSR_SUCCESS,"solving the system"));

			calculate_residuals(n_variables, n_unknowns, unknown_to_id, x, x1, residual, n_zones, zone);

			for(j = 0; j < n_variables; j ++) printf(" %15.10e",residual[j]);

			for(j = 0; j < n_unknowns; j ++) x[j] = x1[j];
		}
	}

	printf("\nwriting out plot data ...");
	print_time(" done in %lf seconds",write_gnuplot(n_unknowns, unknown_to_id, x, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\ncleaning up");
	free_vector(case_filename);
	free_vector(id_to_unknown);
	free_vector(unknown_to_id);
	free_vector(x);
	free_vector(x1);
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
