////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"
#include "divergence.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(1,argc == 2,"wrong number of input arguments");
	char *input_filename = argv[1];

	printf("reading case filename from the input file\n");

	char *case_filename;
	FILE *file = fopen(input_filename,"r");
	handle(1,file != NULL,"opening the input file");
	handle(1,allocate_character_vector(&case_filename,MAX_STRING_LENGTH) == ALLOCATE_SUCCESS,"allocating the case filename");
	handle(1,fetch_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	fclose(file);

	printf("reading divergences from the input file\n");

	int n_divergences;
	DIVERGENCE *divergence;
	divergences_input(input_filename,&n_divergences,&divergence);

	printf("reading the mesh and zones from the case file\n");

	int n_variables;
	int n_nodes, n_faces, n_cells, n_zones;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	struct ZONE *zone;
	read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone);

	printf("generating lists of unknowns\n");

	int n_ids, *id_to_unknown;
	int n_unknowns, *unknown_to_id;
	generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone);

	printf("allocating and initialising the system\n");

	double *lhs, *rhs;
	handle(1,allocate_double_vector(&lhs,n_unknowns) == ALLOCATE_SUCCESS,"allocating system left hand side vector");
	handle(1,allocate_double_vector(&rhs,n_unknowns) == ALLOCATE_SUCCESS,"allocating system right hand side vector");

	CSR matrix = csr_new();

	{
		int i;
		for(i = 0; i < n_ids; i ++) if(id_to_unknown[i] >= 0) lhs[id_to_unknown[i]] = zone[ID_TO_ZONE(i)].value;
	}

	printf("assembling the system\n");

	assemble_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, lhs, rhs, n_faces, face, n_cells, cell, n_zones, zone, n_divergences, divergence);

	printf("solving the system\n");

	//handle(1,csr_solve_superlu(matrix, rhs) == CSR_SUCCESS,"solving the system");
	//handle(1,csr_solve_csparse(matrix, rhs) == CSR_SUCCESS,"solving the system");
	handle(1,csr_solve_umfpack(matrix, rhs) == CSR_SUCCESS,"solving the system");

	{
		printf("writing out zone data\n");

		int u, id, z, i, j;
		int n_polygon;
		double ***polygon;
		handle(1,allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2) == ALLOCATE_SUCCESS,"allocating polygon memory");

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

			n_polygon = (zone[z].location == 'f') ? 2 + face[i].n_borders : cell[i].n_faces;
			generate_control_volume_polygon(polygon, i, zone[z].location, face, cell);

			for(j = 0; j < n_polygon; j ++) fprintf(file[z],"%lf %lf %lf\n",polygon[j][0][0],polygon[j][0][1],rhs[u]);
			fprintf(file[z],"%lf %lf %lf\n\n\n",polygon[j-1][1][0],polygon[j-1][1][1],rhs[u]);
		}

		for(z = 0; z < n_zones; z ++) if(zone[z].condition[0] == 'u') fclose(file[z]);

		free_matrix((void **)polygon);
		free(file);
		free_vector(filename);
	}

	printf("cleaning up\n");

	//clean up
	free_vector(case_filename);
	free_vector(id_to_unknown);
	free_vector(unknown_to_id);
	free_vector(lhs);
	free_vector(rhs);

	nodes_destroy(node);
	faces_destroy(n_faces,face);
	cells_destroy(n_variables,n_cells,cell);
	zones_destroy(zone);
	divergences_destroy(n_divergences, divergence);
	csr_destroy(matrix);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
