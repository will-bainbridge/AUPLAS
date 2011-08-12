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

	/*int n = 10, f = 10, c = 10, z = 5, i, j, k;
	printf("\n\n#### node %i ####",n);
	printf("\n    centroid -> %lf %lf",node[n].x[0],node[n].x[1]);
	printf("\n\n#### face %i ####",f);
	printf("\n       nodes ->"); for(i = 0; i < face[f].n_nodes; i ++) printf(" %i",(int)(face[f].node[i] - &node[0]));
	printf("\n     borders ->"); for(i = 0; i < face[f].n_borders; i ++) printf(" %i",(int)(face[f].border[i] - &cell[0]));
	printf("\norientations ->"); for(i = 0; i < face[f].n_borders; i ++) printf(" %i",face[f].oriented[i]);
	printf("\n       zones ->"); for(i = 0; i < face[f].n_zones; i ++) printf(" %i",(int)(face[f].zone[i] - &zone[0]));
	printf("\n    centroid -> %lf %lf",face[f].centroid[0],face[f].centroid[1]);
	printf("\n\n#### cell %i ####",c);
	printf("\n       faces ->"); for(i = 0; i < cell[c].n_faces; i ++) printf(" %i",(int)(cell[c].face[i] - &face[0]));
	printf("\norientations ->"); for(i = 0; i < cell[c].n_faces; i ++) printf(" %i",cell[c].oriented[i]);
	printf("\n       zones ->"); for(i = 0; i < cell[c].n_zones; i ++) printf(" %i",(int)(cell[c].zone[i] - &zone[0]));
	printf("\n    centroid -> %lf %lf",cell[c].centroid[0],cell[c].centroid[1]);
	for(i = 0; i < n_variables; i ++)
	{
		printf("\n   stencil %i ->",i);
		for(j = 0; j < cell[c].n_stencil[i]; j ++) printf(" %7i",cell[c].stencil[i][j]);
		printf("\n    matrix %i ->",i);
		for(j = 0; j < ORDER_TO_POWERS(cell[c].order[i]); j ++) {
			for(k = 0; k < cell[c].n_stencil[i]; k ++) {
				printf(" %+7.2lf",cell[c].matrix[i][j][k]);
			}
			if(j < ORDER_TO_POWERS(cell[c].order[i]) - 1) printf("\n               ");
		}
	}
	printf("\n\n#### zone %i ####",z);
	printf("\n    location -> %c",zone[z].location);
	printf("\n    variable -> %i",zone[z].variable);
	printf("\n   condition -> %s",zone[z].condition);
	printf("\n       value -> %lf",zone[z].value);
	printf("\n\n");*/

	free(case_filename);
	free_mesh(n_variables, n_nodes, node, n_faces, face, n_cells, cell, n_zones, zone);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
