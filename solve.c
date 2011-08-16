////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	handle(argc == 2, "checking the input arguments");
	char *input_filename = argv[1];

	char *case_filename;
	handle(allocate_character_vector(&case_filename,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating the case filename");
	FILE *file = fopen(input_filename,"r");
	handle(file != NULL, "opening the input file");
	void *ptr = &case_filename;
	handle(fetch_read(file, "case_filename", "s", 1, &ptr) == 1,"reading \"case_filename\" from the input file");
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

	//--------------------------------------------------------------------//
	
	int n_id, *id_to_unknown, *id_to_known;
	n_id = INDEX_AND_ZONE_TO_ID(MAX(n_faces,n_cells)-1,n_zones-1);
	handle(allocate_integer_vector(&id_to_unknown,n_id) == ALLOCATE_SUCCESS,"allocating id to unknown vector");
	handle(allocate_integer_vector(&id_to_known,n_id) == ALLOCATE_SUCCESS,"allocating id to known vector");

	int n_unknowns = 0, n_knowns = 0;

	int i, j;

	for(i = 0; i < n_id; i ++) id_to_unknown[i] = id_to_known[i] = -1;
	for(i = 0; i < n_faces; i ++) {
		for(j = 0; j < face[i].n_zones; j ++) {
			if(face[i].zone[j]->condition[0] == 'u') {
				id_to_unknown[INDEX_AND_ZONE_TO_ID(i,(int)(face[i].zone[j]-&zone[0]))] = n_unknowns ++;
			} else {
				id_to_known[INDEX_AND_ZONE_TO_ID(i,(int)(face[i].zone[j]-&zone[0]))] = n_knowns ++;
			}
		}
	}
	for(i = 0; i < n_cells; i ++) {
		for(j = 0; j < cell[i].n_zones; j ++) {
			if(cell[i].zone[j]->condition[0] == 'u') {
				id_to_unknown[INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0]))] = n_unknowns ++;
			} else {
				id_to_known[INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0]))] = n_knowns ++;
			}
		}
	}

	for(i = 0; i < n_id; i ++)
		if(id_to_unknown[i] != -1 || id_to_known[i] != -1)
			printf("[%5i] %5i %5i\n",i,id_to_unknown[i],id_to_known[i]);

	free_vector(id_to_unknown);
	free_vector(id_to_known);

	//--------------------------------------------------------------------//

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

	free(case_filename);
	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);
	free_equations(n_divergences, divergence);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

/*void assemble_matrices(int n_unknowns, int *id_to_unknown, int n_knowns, int *id_to_known, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_divergences, struct DIVERGENCE *divergence)
{
	for(c = 0; c < n_cells; c ++)
	{
		//generate control volume polygon

		for(i = 0; i < cell[c].n_zones; i ++)
		{
			row = id_to_unknown[INDEX_AND_ZONE_TO_ID(c,i)];
		}
	}
}*/

////////////////////////////////////////////////////////////////////////////////
