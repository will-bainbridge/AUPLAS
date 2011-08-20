////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	char *case_filename;
	handle(allocate_character_vector(&case_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the case filename");

	FILE *file = fopen(input_filename,"r");
	handle(file != NULL,"opening the input file");
	handle(fetch_single_value(file, "case_filename", 's', case_filename) == FETCH_SUCCESS,"reading \"case_filename\" from the input file");
	fclose(file);

	int n_variables = 0;
	int n_nodes = 0, n_faces = 0, n_cells = 0, n_zones = 0;
	struct NODE *node = NULL;
	struct FACE *face = NULL;
	struct CELL *cell = NULL;
	struct ZONE *zone = NULL;
	read_case(case_filename, &n_variables, &n_nodes, &node, &n_faces, &face, &n_cells, &cell, &n_zones, &zone);

	int n_divergences = 0;
	struct DIVERGENCE *divergence = NULL;
	read_divergences(input_filename, n_variables, &n_divergences, &divergence);

	int n_ids = 0, *id_to_unknown = NULL;
	int n_unknowns = 0, *unknown_to_id = NULL;
	generate_system_lists(&n_ids, &id_to_unknown, &n_unknowns, &unknown_to_id, n_faces, face, n_cells, cell, n_zones, zone);

	double *lhs, *rhs;
	handle(allocate_double_vector(&lhs,n_unknowns) == ALLOCATE_SUCCESS,"allocating system left hand side vector");
	handle(allocate_double_vector(&rhs,n_unknowns) == ALLOCATE_SUCCESS,"allocating system right hand side vector");

	CSR matrix = csr_new();

	//initialise
	{
		int i;
		for(i = 0; i < n_ids; i ++)
		{
			if(id_to_unknown[i] >= 0)
			{
				lhs[id_to_unknown[i]] = zone[ID_TO_ZONE(i)].value;
			}
		}
	}

	assemble_matrix(matrix, n_ids, id_to_unknown, n_unknowns, unknown_to_id, lhs, rhs, n_faces, face, n_cells, cell, n_zones, zone, n_divergences, divergence);

	handle(csr_solve_superlu(matrix, rhs) == CSR_SUCCESS,"solving the system");

	{
		int u, id, i, z, j;
		int n_polygon;
		double ***polygon;
		handle(allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2) == ALLOCATE_SUCCESS,"allocating polygon memory");

		FILE **file;
		file = (FILE **)malloc(n_zones * sizeof(FILE *));
		char *filename;
		allocate_character_vector(&filename,MAX_STRING_CHARACTERS);

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


	/*int n = 10, f = 10, c = 10, z = 5, d = 4, i, j, k;
	printf("\n\n#### node %i ####",n);
	printf("\n     centroid -> %lf %lf",node[n].x[0],node[n].x[1]);
	printf("\n\n#### face %i ####",f);
	printf("\n        nodes ->"); for(i = 0; i < face[f].n_nodes; i ++) printf(" %i",(int)(face[f].node[i] - &node[0]));
	printf("\n      borders ->"); for(i = 0; i < face[f].n_borders; i ++) printf(" %i",(int)(face[f].border[i] - &cell[0]));
	printf("\n orientations ->"); for(i = 0; i < face[f].n_borders; i ++) printf(" %i",face[f].oriented[i]);
	printf("\n        zones ->"); for(i = 0; i < face[f].n_zones; i ++) printf(" %i",(int)(face[f].zone[i] - &zone[0]));
	printf("\n     centroid -> %lf %lf",face[f].centroid[0],face[f].centroid[1]);
	printf("\n\n#### cell %i ####",c);
	printf("\n        faces ->"); for(i = 0; i < cell[c].n_faces; i ++) printf(" %i",(int)(cell[c].face[i] - &face[0]));
	printf("\n orientations ->"); for(i = 0; i < cell[c].n_faces; i ++) printf(" %i",cell[c].oriented[i]);
	printf("\n        zones ->"); for(i = 0; i < cell[c].n_zones; i ++) printf(" %i",(int)(cell[c].zone[i] - &zone[0]));
	printf("\n     centroid -> %lf %lf",cell[c].centroid[0],cell[c].centroid[1]);
	for(i = 0; i < n_variables; i ++)
	{
		printf("\n    stencil %i ->",i);
		for(j = 0; j < cell[c].n_stencil[i]; j ++) printf(" %7i",cell[c].stencil[i][j]);
		printf("\n     matrix %i ->",i);
		for(j = 0; j < ORDER_TO_POWERS(cell[c].order[i]); j ++) {
			for(k = 0; k < cell[c].n_stencil[i]; k ++) {
				printf(" %+7.2lf",cell[c].matrix[i][j][k]);
			}
			if(j < ORDER_TO_POWERS(cell[c].order[i]) - 1) printf("\n                ");
		}
	}
	printf("\n\n#### zone %i ####",z);
	printf("\n     location -> %c",zone[z].location);
	printf("\n     variable -> %i",zone[z].variable);
	printf("\n    condition -> %s",zone[z].condition);
	printf("\n        value -> %lf",zone[z].value);
	printf("\n\n#### divergence %i ####",d);
	printf("\n     equation -> %i",divergence[d].equation);
	printf("\n    direction -> %i",divergence[d].direction);
	printf("\n    variables ->"); for(i = 0; i < divergence[d].n_variables; i ++) printf(" %i",divergence[d].variable[i]);
	printf("\ndifferentials ->"); for(i = 0; i < divergence[d].n_variables; i ++) printf(" %i",divergence[d].differential[i]);
	printf("\n     constant -> %lf",divergence[d].constant);
	printf("\n\n");*/

	//clean up
	free(case_filename);

	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);
	free_equations(n_divergences, divergence);
	free_lists(n_ids, id_to_unknown, n_unknowns, unknown_to_id);

	free_vector(lhs);
	free_vector(rhs);

	csr_destroy(matrix);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
