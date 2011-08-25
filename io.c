////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

#define NODE_ID 0
#define FACE_ID 1
#define CELL_ID 2
#define ZONE_ID 3

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
	int i, j, n;

	//open the file
	FILE *file = fopen(filename,"w");
	handle(1,file != NULL, "opening the case file");

	//temporary storage for element pointers cast to indices
	int *index;
	handle(1,allocate_integer_vector(&index,MAX_INDICES) == ALLOCATE_SUCCESS, "allocating indices");

	//number of variables
	handle(1,fwrite(&n_variables, sizeof(int), 1, file) == 1, "writing the number of variables");

	//nodes
	handle(1,fwrite(&n_nodes, sizeof(int), 1, file) == 1, "writing the number of nodes");
	handle(1,fwrite(node, sizeof(struct NODE), n_nodes, file) == n_nodes, "writing the nodes");

	//faces
	handle(1,fwrite(&n_faces, sizeof(int), 1, file) == 1, "writing the number of faces");
	for(i = 0; i < n_faces; i ++) handle(1,fwrite(&(face[i].n_nodes), sizeof(int), 1, file) == 1, "writing the number of face nodes");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_nodes; j ++) index[j] = (int)(face[i].node[j] - &node[0]);
		handle(1,fwrite(index, sizeof(int), face[i].n_nodes, file) == face[i].n_nodes, "writing the face nodes");
	}
	for(i = 0; i < n_faces; i ++) handle(1,fwrite(face[i].centroid, sizeof(double), 2, file) == 2, "writing the face centroid");

	//cells
	handle(1,fwrite(&n_cells, sizeof(int), 1, file) == 1, "writing the number of cells");
	for(i = 0; i < n_cells; i ++) handle(1,fwrite(&(cell[i].n_faces), sizeof(int), 1, file) == 1, "writing the number of cell faces");
	for(i = 0; i < n_cells; i ++)
	{
		for(j = 0; j < cell[i].n_faces; j ++) index[j] = (int)(cell[i].face[j] - &face[0]);
		handle(1,fwrite(index, sizeof(int), cell[i].n_faces, file) == cell[i].n_faces, "writing the cell faces");
		handle(1,fwrite(cell[i].oriented, sizeof(int), cell[i].n_faces, file) == cell[i].n_faces, "writing the cell orientations");
	}
	for(i = 0; i < n_cells; i ++) handle(1,fwrite(cell[i].centroid, sizeof(double), 2, file) == 2, "writing the cell centroid");

	//face borders
	for(i = 0; i < n_faces; i ++) handle(1,fwrite(&(face[i].n_borders), sizeof(int), 1, file) == 1, "writing the number of face borders");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_borders; j ++) index[j] = (int)(face[i].border[j] - &cell[0]);
		handle(1,fwrite(index, sizeof(int), face[i].n_borders, file) == face[i].n_borders, "writing the face borders");
		handle(1,fwrite(face[i].oriented, sizeof(int), face[i].n_borders, file) == face[i].n_borders, "writing the face orientations");
	}

	//zones
	handle(1,fwrite(&n_zones, sizeof(int), 1, file) == 1, "writing the number of zones");
	handle(1,fwrite(zone, sizeof(struct ZONE), n_zones, file) == n_zones, "writing the zones");

	//face zones
	for(i = 0; i < n_faces; i ++) handle(1,fwrite(&(face[i].n_zones), sizeof(int), 1, file) == 1, "writing the number of face zones");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_zones; j ++) index[j] = (int)(face[i].zone[j] - &zone[0]);
		handle(1,fwrite(index, sizeof(int), face[i].n_zones, file) == face[i].n_zones, "writing the face zones");
	}

	//cell zones
	for(i = 0; i < n_cells; i ++) handle(1,fwrite(&(cell[i].n_zones), sizeof(int), 1, file) == 1, "writing the number of cell zones");
	for(i = 0; i < n_cells; i ++)
	{
		for(j = 0; j < cell[i].n_zones; j ++) index[j] = (int)(cell[i].zone[j] - &zone[0]);
		handle(1,fwrite(index, sizeof(int), cell[i].n_zones, file) == cell[i].n_zones, "writing the cell zones");
	}

	//cell stencils
	for(i = 0; i < n_cells; i ++)
	{
		handle(1,fwrite(cell[i].order, sizeof(int), n_variables, file) == n_variables, "writing the cell orders");
		handle(1,fwrite(cell[i].n_stencil, sizeof(int), n_variables, file) == n_variables, "writing the cell stencil sizes");
	}
	for(i = 0; i < n_cells; i ++) {
		for(j = 0; j < n_variables; j ++) {
			handle(1,fwrite(cell[i].stencil[j], sizeof(int), cell[i].n_stencil[j], file) == cell[i].n_stencil[j],"writing the cell stencil");
			n = ORDER_TO_POWERS(cell[i].order[j]) * cell[i].n_stencil[j];
			handle(1,fwrite(cell[i].matrix[j][0], sizeof(double), n, file) == n,"writing the cell matrix");
		}
	}

	//clean up
	free_vector(index);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void read_case(char *filename, int *n_variables, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell, int *n_zones, struct ZONE **zone)
{
	int i, j, n;

	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL, "opening the case file");

	//temporary storage for element pointers cast to indices
	int *index;
	handle(1,allocate_integer_vector(&index,MAX_INDICES) == ALLOCATE_SUCCESS, "allocating indices");

	//number of variables
	handle(1,fread(n_variables, sizeof(int), 1, file) == 1, "reading the number of variables");

	//nodes
	handle(1,fread(n_nodes, sizeof(int), 1, file) == 1, "reading the number of nodes");
	handle(1,allocate_mesh(0, *n_nodes, node, 0, NULL, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating node structures");
	handle(1,fread(*node, sizeof(struct NODE), *n_nodes, file) == *n_nodes, "reading the nodes");

	//faces
	handle(1,fread(n_faces, sizeof(int), 1, file) == 1, "reading the number of faces");
	handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face structures");
	for(i = 0; i < *n_faces; i ++) handle(1,fread(&((*face)[i].n_nodes), sizeof(int), 1, file) == 1, "reading the number of face nodes");
	handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face nodes");
	for(i = 0; i < *n_faces; i ++)
	{
		handle(1,fread(index, sizeof(int), (*face)[i].n_nodes, file) == (*face)[i].n_nodes, "reading the face nodes");
		for(j = 0; j < (*face)[i].n_nodes; j ++) (*face)[i].node[j] = &(*node)[index[j]];
	}
	for(i = 0; i < *n_faces; i ++) handle(1,fread((*face)[i].centroid, sizeof(double), 2, file) == 2, "reading the face centroid");

	//cells
	handle(1,fread(n_cells, sizeof(int), 1, file) == 1, "reading the number of cells");
	handle(1,allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell structures");
	for(i = 0; i < *n_cells; i ++) handle(1,fread(&((*cell)[i].n_faces), sizeof(int), 1, file) == 1, "reading the number of cell faces");
	handle(1,allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell faces");
	for(i = 0; i < *n_cells; i ++)
	{
		handle(1,fread(index, sizeof(int), (*cell)[i].n_faces, file) == (*cell)[i].n_faces, "reading the cell faces");
		for(j = 0; j < (*cell)[i].n_faces; j ++) (*cell)[i].face[j] = &(*face)[index[j]];
		handle(1,fread((*cell)[i].oriented, sizeof(int), (*cell)[i].n_faces, file) == (*cell)[i].n_faces, "reading the cell orientations");
	}
	for(i = 0; i < *n_cells; i ++) handle(1,fread((*cell)[i].centroid, sizeof(double), 2, file) == 2, "reading the cell centroid");

	//face borders
	for(i = 0; i < *n_faces; i ++) handle(1,fread(&((*face)[i].n_borders), sizeof(int), 1, file) == 1, "reading the number of face borders");
	handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face borders");
	for(i = 0; i < *n_faces; i ++)
	{
		handle(1,fread(index, sizeof(int), (*face)[i].n_borders, file) == (*face)[i].n_borders, "reading the face borders");
		for(j = 0; j < (*face)[i].n_borders; j ++) (*face)[i].border[j] = &(*cell)[index[j]];
		handle(1,fread((*face)[i].oriented, sizeof(int), (*face)[i].n_borders, file) == (*face)[i].n_borders, "reading the face orientations");
	}

	//zones
	handle(1,fread(n_zones, sizeof(int), 1, file) == 1, "reading the number of zones");
	handle(1,allocate_mesh(0, 0, NULL, 0, NULL, 0, NULL, *n_zones, zone) == ALLOCATE_SUCCESS, "allocating zone structures");
	handle(1,fread(*zone, sizeof(struct ZONE), *n_zones, file) == *n_zones, "reading the zones");

	//face zones
	for(i = 0; i < *n_faces; i ++) handle(1,fread(&((*face)[i].n_zones), sizeof(int), 1, file) == 1, "reading the number of face zones");
	handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face zones");
	for(i = 0; i < *n_faces; i ++)
	{
		handle(1,fread(index, sizeof(int), (*face)[i].n_zones, file) == (*face)[i].n_zones, "reading the face zones");
		for(j = 0; j < (*face)[i].n_zones; j ++) (*face)[i].zone[j] = &(*zone)[index[j]];
	}

	//cell zones
	for(i = 0; i < *n_cells; i ++) handle(1,fread(&((*cell)[i].n_zones), sizeof(int), 1, file) == 1, "reading the number of cell zones");
	handle(1,allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell zones");
	for(i = 0; i < *n_cells; i ++)
	{
		handle(1,fread(index, sizeof(int), (*cell)[i].n_zones, file) == (*cell)[i].n_zones, "reading the cell zones");
		for(j = 0; j < (*cell)[i].n_zones; j ++) (*cell)[i].zone[j] = &(*zone)[index[j]];
	}

	//cell stencils
	handle(1,allocate_mesh(*n_variables, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell stencil numbers and orders");
	for(i = 0; i < *n_cells; i ++)
	{
		handle(1,fread((*cell)[i].order, sizeof(int), *n_variables, file) == *n_variables, "reading the cell orders");
		handle(1,fread((*cell)[i].n_stencil, sizeof(int), *n_variables, file) == *n_variables, "reading the cell stencil sizes");
	}
	handle(1,allocate_mesh(*n_variables, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell stencils and matrices");
	for(i = 0; i < *n_cells; i ++) {
		for(j = 0; j < *n_variables; j ++) {
			handle(1,fread((*cell)[i].stencil[j], sizeof(int), (*cell)[i].n_stencil[j], file) == (*cell)[i].n_stencil[j],"reading the cell stencil");
			n = ORDER_TO_POWERS((*cell)[i].order[j]) * (*cell)[i].n_stencil[j];
			handle(1,fread((*cell)[i].matrix[j][0], sizeof(double), n, file) == n,"reading the cell matrix");
		}
	}

	//clean up
	free_vector(index);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////
