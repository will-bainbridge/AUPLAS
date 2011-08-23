////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	//open file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening geometry file");

	//allocate temporary storage
	int i, j, *index, count, offset;
	char *line, *temp;
	handle(1,allocate_integer_vector(&index,MAX(MAX_FACE_NODES,MAX_CELL_FACES)) == ALLOCATE_SUCCESS, "allocating indices");
	handle(1,allocate_character_vector(&line, MAX_STRING_LENGTH) == ALLOCATE_SUCCESS, "allocating line string");
	handle(1,allocate_character_vector(&temp, MAX_STRING_LENGTH) == ALLOCATE_SUCCESS, "allocating temporary string");

	//initialise
	*n_nodes = *n_faces = *n_cells = 0;

	//read each line in turn
	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
	{
		//------------------------------------------------------------//

		if(strncmp(line,"NODES",5) == 0)
		{
			//number of nodes
			handle(1,sscanf(&line[6],"%i",n_nodes) == 1, "reading the number of nodes");

			//allocate the nodes
			handle(1,allocate_mesh(0, *n_nodes, node, 0, NULL, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating node structures");

			//read in the node locations
			for(i = 0; i < *n_nodes; i ++) handle(1,fscanf(file,"%lf %lf\n",&((*node)[i].x[0]),&((*node)[i].x[1])) == 2, "reading a node's coordinates");
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"FACES",5) == 0)
		{
			//number of faces
			handle(1,sscanf(&line[6],"%i",n_faces) == 1, "reading the number of faces");

			//allocate the faces
			handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face structures");

			//read in the face node indices and store pointers
			for(i = 0; i < *n_faces; i ++)
			{
				//read a line
				handle(1,fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a face line");

				//strip newlines and whitespace off the end of the line
				for(j = strlen(line)-1; j >= 0; j --) if(line[j] != ' ' && line[j] != '\n') break;
				line[j+1] = '\0';

				//sequentially read the integers on the line
				count = offset = 0;
				while(offset < strlen(line))
				{
					sscanf(&line[offset],"%s",temp);
					sscanf(temp,"%i",&index[count]);
					count ++;
					offset += strlen(temp) + 1;
					while(line[offset] == ' ') offset ++;
				}

				//number of faces
				(*face)[i].n_nodes = count;

				//allocate the faces
				handle(1,allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face nodes");

				//node pointers
				for(j = 0; j < count; j ++) (*face)[i].node[j] = &((*node)[index[j]]);
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"CELLS",5) == 0)
		{
			//number of cells
			handle(1,sscanf(&line[6],"%i",n_cells) == 1, "reading the number of cells");

			//allocate the cells
			handle(1,allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell structures");

			//read in the cell faces and store pointers
			for(i = 0; i < *n_cells; i ++)
			{
				//same as above but for cells
				handle(1,fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a cell line");
				for(j = strlen(line)-1; j >= 0; j --) if(line[j] != ' ' && line[j] != '\n') break;
				line[j+1] = '\0';
				count = offset = 0;
				while(offset < strlen(line))
				{
					sscanf(&line[offset],"%s",temp);
					sscanf(temp,"%i",&index[count]);
					count ++;
					offset += strlen(temp) + 1;
					while(line[offset] == ' ') offset ++;
				}
				(*cell)[i].n_faces = count;
				handle(1,allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL), "allocating cell faces");
				for(j = 0; j < count; j ++) (*cell)[i].face[j] = &((*face)[index[j]]);
			}
		}

		//------------------------------------------------------------//

	}

	//check
	handle(1,*node != NULL,"finding nodes in geometry file");
	handle(1,*face != NULL,"finding faces in geometry file");
	handle(1,*cell != NULL,"finding cells in geometry file");

	//clean up
	free(index);
	free(line);
	free(temp);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void read_zones(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
	int i, j;

	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening input file");

	//allocate the zone data
	FETCH fetch;
	fetch = fetch_new("csisd", MAX_ZONES);
	handle(1,fetch != NULL,"allocating zone input");

	//fetch the data from the file
	*n_zones = fetch_read(file, "zone", fetch);
	handle(1,*n_zones != FETCH_FILE_ERROR && *n_zones != FETCH_MEMORY_ERROR && *n_zones > 0, "reading zones");

	//allocate the zone structures
	handle(1,allocate_mesh(0, 0, NULL, 0, NULL, 0, NULL, *n_zones, zone) == ALLOCATE_SUCCESS, "allocating zone structures");

	//get the zone parameters
	for(i = 0; i < *n_zones; i ++)
	{
		fetch_get(fetch, i, 0, &(*zone)[i].location);
		fetch_get(fetch, i, 2, &(*zone)[i].variable);
		fetch_get(fetch, i, 3, (*zone)[i].condition);
		fetch_get(fetch, i, 4, &(*zone)[i].value);
	}

	//temporary storage for face and cell zones
	int **face_zone, **cell_zone;
	handle(1,allocate_integer_matrix(&face_zone,n_faces,*n_zones) == ALLOCATE_SUCCESS, "allocating list of face zones");
	handle(1,allocate_integer_matrix(&cell_zone,n_cells,*n_zones) == ALLOCATE_SUCCESS, "allocating list of cell zones");

	//decode the index ranges
	char *range, *temp;
	int offset, index[2];
	handle(1,allocate_character_vector(&range,MAX_STRING_LENGTH) == ALLOCATE_SUCCESS, "allocating range string");
	handle(1,allocate_character_vector(&temp,MAX_STRING_LENGTH) == ALLOCATE_SUCCESS, "allocating temporary string");

	for(i = 0; i < *n_zones; i ++)
	{
		//get the range string
		fetch_get(fetch, i, 1, range);

		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(range); j ++) if(range[j] == ',') range[j] = ' ';

		//sequentially read ranges
		offset = 0;
		while(offset < strlen(range))
		{
			//read the range from the string
			handle(1,sscanf(&range[offset],"%s",temp) == 1, "reading range string");
			handle(1,sscanf(temp,"%i:%i",&index[0],&index[1]) == 2, "getting indices from range string");

			//note the elements in the range which are a part of this zone
			if((*zone)[i].location == 'f') for(j = index[0]; j <= index[1]; j ++) face_zone[j][face[j].n_zones++] = i;
			if((*zone)[i].location == 'c') for(j = index[0]; j <= index[1]; j ++) cell_zone[j][cell[j].n_zones++] = i;

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}
	}

	//copy face and cell zone data into the structures
	handle(1,allocate_mesh(0, 0, NULL, n_faces, &face, n_cells, &cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell and face zones");
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) face[i].zone[j] = &(*zone)[face_zone[i][j]];
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) cell[i].zone[j] = &(*zone)[cell_zone[i][j]];

	//clean up
	fetch_destroy(fetch);
	free_matrix((void **)face_zone);
	free_matrix((void **)cell_zone);
	free_vector(range);
	free_vector(temp);
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
