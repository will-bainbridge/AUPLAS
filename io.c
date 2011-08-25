////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening geometry file");

	char *line;
	handle(1,allocate_character_vector(&line, MAX_STRING_LENGTH) == ALLOCATE_SUCCESS, "allocating line string");

	int i;

	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
	{
		if(strncmp(line,"NODES",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_nodes) == 1,"reading the number of nodes");
			*node = nodes_new(*n_nodes, NULL);
			handle(1,*node != NULL,"allocating the nodes");
			for(i = 0; i < *n_nodes; i ++) node_geometry_get(file, &(*node)[i]);
		}
		if(strncmp(line,"FACES",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_faces) == 1,"reading the number of faces");
			*face = faces_new(*n_faces, NULL);
			handle(1,*face != NULL,"allocating the faces");
			for(i = 0; i < *n_faces; i ++) face_geometry_get(file, &(*face)[i], *node);
		}
		if(strncmp(line,"CELLS",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_cells) == 1,"reading the number of cells");
			*cell = cells_new(*n_cells, NULL);
			handle(1,*cell != NULL,"allocating the cells");
			for(i = 0; i < *n_cells; i ++) cell_geometry_get(file, &(*cell)[i], *face);
		}
	}

	handle(1,*n_nodes > 0,"finding nodes in the geometry file");
	handle(1,*n_faces > 0,"finding faces in the geometry file");
	handle(1,*n_cells > 0,"finding cells in the geometry file");

	free(line);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i;

	//open the file
	FILE *file = fopen(filename,"w");
	handle(1,file != NULL, "opening the case file");

	//number of variables
	handle(1,fwrite(&n_variables, sizeof(int), 1, file) == 1, "writing the number of variables");

	//numbers of elements
	handle(1,fwrite(&n_nodes, sizeof(int), 1, file) == 1, "writing the number of nodes");
	handle(1,fwrite(&n_faces, sizeof(int), 1, file) == 1, "writing the number of faces");
	handle(1,fwrite(&n_cells, sizeof(int), 1, file) == 1, "writing the number of cells");
	handle(1,fwrite(&n_zones, sizeof(int), 1, file) == 1, "writing the number of zones");

	//elements
	for(i = 0; i < n_nodes; i ++) node_case_write(file, &node[i]);
	for(i = 0; i < n_faces; i ++) face_case_write(file, node, &face[i], cell, zone);
	for(i = 0; i < n_cells; i ++) cell_case_write(file, n_variables, face, &cell[i], zone);
	for(i = 0; i < n_zones; i ++) zone_case_write(file, &zone[i]);

	//clean up
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void read_case(char *filename, int *n_variables, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell, int *n_zones, struct ZONE **zone)
{
	int i;

	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL, "opening the case file");

	//number of variables
	handle(1,fread(n_variables, sizeof(int), 1, file) == 1, "reading the number of variables");

	//numbers of elements
	handle(1,fread(n_nodes, sizeof(int), 1, file) == 1, "reading the number of nodes");
	handle(1,(*node = nodes_new(*n_nodes,NULL)) != NULL,"allocating nodes");
	handle(1,fread(n_faces, sizeof(int), 1, file) == 1, "reading the number of faces");
	handle(1,(*face = faces_new(*n_faces,NULL)) != NULL,"allocating faces");
	handle(1,fread(n_cells, sizeof(int), 1, file) == 1, "reading the number of cells");
	handle(1,(*cell = cells_new(*n_cells,NULL)) != NULL,"allocating cells");
	handle(1,fread(n_zones, sizeof(int), 1, file) == 1, "reading the number of zones");
	handle(1,(*zone = zones_new(*n_zones,NULL)) != NULL,"allocating zones");

	//elements
	for(i = 0; i < *n_nodes; i ++) node_case_get(file, &(*node)[i]);
	for(i = 0; i < *n_faces; i ++) face_case_get(file, *node, &(*face)[i], *cell, *zone);
	for(i = 0; i < *n_cells; i ++) cell_case_get(file, *n_variables, *face, &(*cell)[i], *zone);
	for(i = 0; i < *n_zones; i ++) zone_case_get(file, &(*zone)[i]);

	//clean up
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////
