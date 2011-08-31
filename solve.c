////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(1,argc == 2,"wrong number of input arguments");
	char *input_filename = argv[1];

	printf("reading case filename from the input file\n");
	char *case_filename;
	FILE *file = fopen(input_filename,"r");
	handle(1,file != NULL,"opening the input file");
	handle(1,allocate_character_vector(&case_filename,MAX_STRING_LENGTH),"allocating the case filename");
	handle(1,fetch_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	fclose(file);

	printf("reading divergences from the input file\n");
	int n_divergences;
	struct DIVERGENCE *divergence;
	divergences_input(input_filename,&n_divergences,&divergence);

	printf("reading the mesh and zones from the case file\n");
	int n_variables, n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone);

	printf("generating lists of unknowns\n");
	int n_ids, *id_to_unknown, n_unknowns, *unknown_to_id;
	generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone);

	printf("allocating and initialising the system\n");
	double *x, *x1, *residual;
	CSR matrix = csr_new();
	handle(1,allocate_double_vector(&x,n_unknowns),"allocating system left hand side vector");
	handle(1,allocate_double_vector(&x1,n_unknowns),"allocating system right hand side vector");
	handle(1,allocate_double_vector(&residual,n_variables),"allocating residuals");
	initialise_unknowns(n_ids, id_to_unknown, zone, x);

	printf("iterating\n");
	{
		int i, j, iterations_per_step = 20;

		for(i = 0; i < iterations_per_step; i ++)
		{
			printf("    %4i -> ",i); fflush(stdout);

			assemble_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, x, x1, n_faces, face, n_cells, cell, n_zones, zone, n_divergences, divergence);

			handle(1,csr_solve_umfpack(matrix, x1) == CSR_SUCCESS,"solving the system");

			calculate_residuals(n_variables, n_unknowns, unknown_to_id, x, x1, residual, n_zones, zone);

			for(j = 0; j < n_variables; j ++) printf(" %15.10e",residual[j]);
			printf("\n");

			for(j = 0; j < n_unknowns; j ++) x[j] = x1[j];
		}
	}

	printf("writing out plot data\n");
	write_gnuplot(n_unknowns, unknown_to_id, x, n_faces, face, n_cells, cell, n_zones, zone);

	/*{
		printf("writing out zone data\n");

		int u, id, z, i;
		double ***polygon;
		handle(1,allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2),"allocating polygon memory");

		FILE **file;
		file = (FILE **)malloc(n_zones * sizeof(FILE *));
		char *filename;
		allocate_character_vector(&filename,MAX_STRING_LENGTH);

		for(z = 0; z < n_zones; z ++) {
			if(zone[z].condition[0] == 'u') {
				sprintf(filename,"zone-%i.gnuplot",z);
				file[z] = fopen(filename,"w");
			}
		}

		for(u = 0; u < n_unknowns; u ++)
		{
			id = unknown_to_id[u];

			i = ID_TO_INDEX(id);
			z = ID_TO_ZONE(id);

			generate_control_volume_polygon(polygon, i, zone[z].location, face, cell);

			fprintf(file[z],"%lf %lf %lf\n",polygon[0][0][0],polygon[0][0][1],x1[u]);
			fprintf(file[z],"%lf %lf %lf\n\n",polygon[0][1][0],polygon[0][1][1],x1[u]);
			fprintf(file[z],"%lf %lf %lf\n",polygon[2][1][0],polygon[2][1][1],x1[u]);
			fprintf(file[z],"%lf %lf %lf\n\n\n",polygon[2][0][0],polygon[2][0][1],x1[u]);
		}

		for(z = 0; z < n_zones; z ++) if(zone[z].condition[0] == 'u') fclose(file[z]);

		free_matrix((void **)polygon);
		free(file);
		free_vector(filename);
	}*/

	printf("cleaning up\n");
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

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
