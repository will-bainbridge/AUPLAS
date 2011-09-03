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

	printf("\nreading accumulations from the input file ...");
	int n_accumulations;
	struct ACCUMULATION *accumulation;
	print_time(" done in %lf seconds",accumulations_input(input_filename,&n_accumulations,&accumulation));

	printf("\ngenerating lists of unknowns ...");
	int n_ids, *id_to_unknown, n_unknowns, *unknown_to_id;
	print_time(" done in %lf seconds",generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\nallocating and initialising the unknowns ...");
	double *x, *x1, *b, *bn, *residual;
	exit_if_false(allocate_double_vector(&x,n_unknowns),"allocating old unknown vector");
	exit_if_false(allocate_double_vector(&x1,n_unknowns),"allocating new unknown vector");
	exit_if_false(allocate_double_vector(&b,n_unknowns),"allocating right hand side vector");
	exit_if_false(allocate_double_vector(&bn,n_unknowns),"allocating last timestep right hand side vector");
	exit_if_false(allocate_double_vector(&residual,n_variables),"allocating residuals");
	print_time(" done in %lf seconds",initialise_unknowns(n_ids, id_to_unknown, zone, x1));

	printf("\nassembling the system matrix ...");
	CSR matrix = csr_new();
	print_time(" done in %lf seconds",assemble_matrix(matrix, n_variables, id_to_unknown, n_unknowns, unknown_to_id, face, cell, zone));

	double *implicit, *mass;
	exit_if_false(allocate_double_vector(&implicit,n_unknowns),"allocating implicit vector");
	exit_if_false(allocate_double_vector(&mass,n_unknowns),"allocating mass vector");

	double timestep = 1.0e-1;

	{
		int a, u, z;

		for(u = 0; u < n_unknowns; u ++)
		{
			z = ID_TO_ZONE(unknown_to_id[u]);

			implicit[u] = - timestep;
			mass[u] = 0.0;
			for(a = 0; a < n_accumulations; a ++)
			{
				if(accumulation[a].variable == zone[z].variable)
				{
					implicit[u] = - timestep * accumulation[a].implicit;
					mass[u] = accumulation[a].constant;
					if(zone[z].location == 'f')      mass[u] *= face[ID_TO_INDEX(unknown_to_id[u])].area;
					else if(zone[z].location == 'c') mass[u] *= cell[ID_TO_INDEX(unknown_to_id[u])].area;
					else exit_if_false(0,"recognising location");
				}
			}
		}
	}

	{
		int s, i, v, u;

		int n_steps = 5;
		
		for(s = 1; s <= n_steps; s ++)
		{
			printf("\n");
			printf("\n timestep > %i",s);
			printf("\n     time > %8.2e",timestep*(s-1));
			printf("\niteration > assembly solution >");
			for(v = 0; v < n_variables; v ++) printf(" %-15s",variable_name[v]);

			for(i = 1; i <= n_iterations_per_step; i ++)
			{
				for(u = 0; u < n_unknowns; u ++) x[u] = x1[u];

				printf("\n%9i >",i);

				//calculate the divergences
				print_time(" %7.1es",calculate_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, x, b,
							face, cell, zone, n_divergences, divergence));

				//explicit and old timestep part of RHS calculated on 1st iteration only
				if(i == 1)
				{
					for(u = 0; u < n_unknowns; u ++) bn[u] = b[u];
					csr_multiply_vector(matrix,x,bn);
					for(u = 0; u < n_unknowns; u ++) bn[u] = mass[u] * x[u] + (timestep + implicit[u]) * bn[u];
				}

				//implicit part of the RHS calculated every iteration
				for(u = 0; u < n_unknowns; u ++) b[u] = bn[u] - implicit[u] * b[u];
				csr_multiply_diagonal(matrix,implicit);
				csr_add_to_diagonal(matrix,mass);

				//solve the system
				print_time(" %7.1es",exit_if_false(csr_solve_umfpack(matrix,x1,b) == CSR_SUCCESS,"solving the system"));

				printf(" >");

				calculate_residuals(n_variables, n_unknowns, unknown_to_id, x, x1, residual, n_zones, zone);
				for(v = 0; v < n_variables; v ++) printf(" %15.9e",residual[v]);
			}

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
	free_vector(bn);
	free_vector(residual);
	free_vector(implicit);
	free_vector(mass);
	nodes_destroy(n_nodes,node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);
	divergences_destroy(n_divergences, divergence);
	accumulations_destroy(n_accumulations, accumulation);
	csr_destroy(matrix);

	print_end();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
