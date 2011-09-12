////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	exit_if_false(argc != 2 || argc != 3,"wrong number of input arguments");
	char *input_filename = argv[1], *initial_filename = (argc == 3) ? argv[2] : NULL;

	printf("\nreading case and data filenames from the input file");
	FILE *file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");
	char *case_filename;
	exit_if_false(allocate_character_vector(&case_filename,MAX_STRING_LENGTH),"allocating the case filename");
	exit_if_false(fetch_value(file,"case_filename",'s',case_filename)==FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	char *data_filename;
	exit_if_false(allocate_character_vector(&data_filename,MAX_STRING_LENGTH),"allocating the data filename");
	exit_if_false(fetch_value(file,"data_filename",'s',data_filename)==FETCH_SUCCESS,"reading \"data_filename\" from the input file");
	fclose(file);

	printf("\nreading the mesh and zones from the case file ...");
	int n_variables, n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	print_time(" done in %lf seconds",read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone));

	//--------------------------------------------------------------------//

	printf("\nreading control from the input file");

	file = fopen(input_filename,"r");
	exit_if_false(file != NULL,"opening the input file");

	char **variable_name;
	exit_if_false(allocate_character_matrix(&variable_name,n_variables,MAX_STRING_LENGTH),"allocating variable names");
	warn_if_false(fetch_vector(file,"variable_names",'s',n_variables,variable_name) == FETCH_SUCCESS,"reading \"variable_names\" from the input file");
	double *accumulation;
	exit_if_false(allocate_double_vector(&accumulation,n_variables),"allocating the accumulations");
	exit_if_false(fetch_vector(file,"accumulation",'d',n_variables,accumulation) == FETCH_SUCCESS,"reading \"accumulation\" from the input file");
	double timestep;
	exit_if_false(fetch_value(file,"timestep",'d',&timestep) == FETCH_SUCCESS,"reading \"timestep\" from the input file");
	int n_steps;
	exit_if_false(fetch_value(file,"number_of_steps",'i',&n_steps) == FETCH_SUCCESS,"reading \"number_of_steps\" from the input file");
	int n_steps_per_output;
	exit_if_false(fetch_value(file,"number_of_steps_per_output",'i',&n_steps_per_output) == FETCH_SUCCESS,"reading \"number_of_steps_per_output\" from the input file");
	int n_iterations_per_step;
	exit_if_false(fetch_value(file,"number_of_iterations_per_step",'i',&n_iterations_per_step) == FETCH_SUCCESS,"reading \"number_of_iterations_per_step\" from the input file");

	fclose(file);

	//--------------------------------------------------------------------//

	printf("\nreading divergences from the input file ...");
	int n_divergences;
	struct DIVERGENCE *divergence;
	print_time(" done in %lf seconds",divergences_input(input_filename,&n_divergences,&divergence));

	printf("\ngenerating lists of unknowns ...");
	int n_ids, *id_to_unknown, n_unknowns, *unknown_to_id;
	print_time(" done in %lf seconds",generate_lists_of_unknowns(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone));

	printf("\nallocating and initialising the unknowns ...");
	double *x, *dx, *f, *f_explicit, *residual;
	exit_if_false(allocate_double_vector(&x,n_unknowns),"allocating unknown vector");
	exit_if_false(allocate_double_vector(&dx,n_unknowns),"allocating unknown change vector");
	exit_if_false(allocate_double_vector(&f,n_unknowns),"allocating function vector");
	exit_if_false(allocate_double_vector(&f_explicit,n_unknowns),"allocating explicit part of function vector");
	exit_if_false(allocate_double_vector(&residual,n_variables),"allocating residuals");
	double time = 0.0;
	if(initial_filename == NULL) print_time(" done in %lf seconds",initialise_unknowns(n_ids, id_to_unknown, zone, x));
	else                         print_time(" done in %lf seconds",read_data(initial_filename, &time, n_unknowns, x));

	printf("\nassembling the divergence jacobians ...");
	CSR jacobian = csr_new();
	print_time(" done in %lf seconds",assemble_matrix(jacobian,n_variables,id_to_unknown,n_unknowns,unknown_to_id,face,cell,zone));

	double *mass;
	exit_if_false(allocate_double_vector(&mass,n_unknowns),"allocating mass vector");
	{
		int i, u, z;

		for(u = 0; u < n_unknowns; u ++)
		{
			i = ID_TO_INDEX(unknown_to_id[u]);
			z = ID_TO_ZONE(unknown_to_id[u]);

			if(zone[z].location == 'f')      mass[u] = accumulation[zone[z].variable] * face[i].area;
			else if(zone[z].location == 'c') mass[u] = accumulation[zone[z].variable] * cell[i].area;
			else exit_if_false(0,"recognising location");
		}
	}


	{
		int d, i, s, u, v;

		for(s = 1; s <= n_steps; s ++)
		{
			printf("\n\ntimestep > %i\n    time > %15.9e s\nresidual >",s,time);
			for(v = 0; v < n_variables; v ++) printf(" %-15s",variable_name[v]);

			for(d = 0; d < n_divergences; d ++)
				divergence[d].coefficient = - divergence[d].constant * timestep * (1.0 - divergence[d].implicit);

			print_time("\n%lfs\n",calculate_divergence(f_explicit, jacobian, x, n_variables, n_ids, id_to_unknown, n_unknowns, unknown_to_id,
					face, n_cells, cell, zone, n_divergences, divergence));

			for(u = 0; u < n_unknowns; u ++) f_explicit[u] += mass[u]*x[u];

			for(d = 0; d < n_divergences; d ++)
				divergence[d].coefficient = - divergence[d].constant * timestep * divergence[d].implicit;

			for(i = 1; i <= n_iterations_per_step; i ++)
			{
				printf("\n%8i >",i);

				calculate_divergence(f, jacobian, x, n_variables, n_ids, id_to_unknown, n_unknowns, unknown_to_id,
						face, n_cells, cell, zone, n_divergences, divergence);

				for(u = 0; u < n_unknowns; u ++) f[u] += f_explicit[u] - mass[u] * x[u];

				csr_add_to_diagonal(jacobian,mass);

				exit_if_false(csr_solve_umfpack(jacobian,dx,f) == CSR_SUCCESS,"solving the system");

				calculate_residuals(n_variables, n_unknowns, unknown_to_id, dx, x, residual, n_zones, zone);
				for(v = 0; v < n_variables; v ++) printf(" %15.9e",residual[v]);

				for(u = 0; u < n_unknowns; u ++) x[u] += dx[u];
			}

			time += timestep;

			//output
			if(s % n_steps_per_output == 0 || s == n_steps)
			{
				printf("\n\nwriting data ...");
				print_time(" done in %lf seconds",write_data(data_filename, time, n_unknowns, x));
			}
		}
	}

	printf("\ncleaning up");
	free_vector(case_filename);
	free_vector(data_filename);
	free_matrix((void**)variable_name);
	free_vector(accumulation);
	free_vector(id_to_unknown);
	free_vector(unknown_to_id);
	free_vector(x);
	free_vector(dx);
	free_vector(f);
	free_vector(f_explicit);
	free_vector(residual);
	free_vector(mass);
	nodes_destroy(n_nodes,node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);
	divergences_destroy(n_divergences, divergence);
	csr_destroy(jacobian);

	print_end();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
