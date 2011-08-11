////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

void read_instructions(char *filename, int *n_variables, char **geometry_filename, char **case_filename, int **maximum_order, double **weight_exponent, char ***connectivity)
{
	FILE *file = fopen(filename,"r");
	handle(file != NULL,"opening input file");

	handle(fetch_read(file, "number_of_variables", "i", 1, (void*)&n_variables) == 1, "reading \"number_of_variables\" from the input file");

	handle(allocate_instructions(*n_variables, geometry_filename, case_filename, maximum_order, weight_exponent, connectivity)
			== ALLOCATE_SUCCESS, "allocating instructions");

	handle(fetch_read(file, "geometry_filename", "s", 1, (void*)&geometry_filename) == 1,"reading \"geometry_filename\" from the input file");
	handle(fetch_read(file, "case_filename", "s", 1, (void*)&case_filename) == 1,"reading \"case_filename\" from the input file");

        char *format;
	handle(allocate_character_vector(&format,*n_variables + 1) == ALLOCATE_SUCCESS,"allocating format string");
        format[*n_variables] = '\0';

        memset(format,'i',*n_variables);
	handle(fetch_read(file, "maximum_order", format, 1, (void*)maximum_order) == 1 ,"reading \"maximum_order\" from the input file");

	int i;
	for(i = 0; i < *n_variables; i ++) (*maximum_order)[i] -= 1;

        memset(format,'d',*n_variables);
	handle(fetch_read(file, "weight_exponent", format, 1, (void*)weight_exponent) == 1, "reading \"weight_exponent\" from the input file");

        memset(format,'s',*n_variables);
	handle(fetch_read(file, "connectivity", format, 1, (void*)connectivity) == 1, "reading \"connectivity\" from the input file");

        fclose(file);
        free_vector(format);
}

////////////////////////////////////////////////////////////////////////////////

void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	//open file
	FILE *file = fopen(filename,"r");
	handle(file != NULL,"opening geometry file");

	//allocate temporary storage
	int i, j, *index, count, offset;
	char *line, *temp;
	handle(allocate_integer_vector(&index,MAX(MAX_FACE_NODES,MAX_CELL_FACES)) == ALLOCATE_SUCCESS, "allocating indices");
	handle(allocate_character_vector(&line, MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating line string");
	handle(allocate_character_vector(&temp, MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating temporary string");

	//initialise
	*n_nodes = *n_faces = *n_cells = 0;

	//read each line in turn
	while(fgets(line, MAX_STRING_CHARACTERS, file) != NULL)
	{
		//------------------------------------------------------------//

		if(strncmp(line,"NODES",5) == 0)
		{
			//number of nodes
			handle(sscanf(&line[6],"%i",n_nodes) == 1, "reading the number of nodes");

			//allocate the nodes
			handle(allocate_mesh(0, *n_nodes, node, 0, NULL, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating node structures");

			//read in the node locations
			for(i = 0; i < *n_nodes; i ++)
			{
				handle(fscanf(file,"%lf %lf\n",&((*node)[i].x[0]),&((*node)[i].x[1])) == 2, "reading a node's coordinates");
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"FACES",5) == 0)
		{
			//number of faces
			handle(sscanf(&line[6],"%i",n_faces) == 1, "reading the number of faces");

			//allocate the faces
			handle(allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face structures");

			//read in the face node indices and store pointers
			for(i = 0; i < *n_faces; i ++)
			{
				//read a line
				handle(fgets(line, MAX_STRING_CHARACTERS, file) != NULL, "reading a face line");

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
				handle(allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) == ALLOCATE_SUCCESS, "allocating face nodes");

				//node pointers
				for(j = 0; j < count; j ++) (*face)[i].node[j] = &((*node)[index[j]]);
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"CELLS",5) == 0)
		{
			//number of cells
			handle(sscanf(&line[6],"%i",n_cells) == 1, "reading the number of cells");

			//allocate the cells
			handle(allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell structures");

			//read in the cell faces and store pointers
			for(i = 0; i < *n_cells; i ++)
			{
				//same as above but for cells
				handle(fgets(line, MAX_STRING_CHARACTERS, file) != NULL, "reading a cell line");
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
				handle(allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL), "allocating cell faces");
				for(j = 0; j < count; j ++) (*cell)[i].face[j] = &((*face)[index[j]]);
			}
		}

		//------------------------------------------------------------//

	}

	//check
	handle(*node != NULL,"finding nodes in geometry file");
	handle(*face != NULL,"finding faces in geometry file");
	handle(*cell != NULL,"finding cells in geometry file");

	//clean up
	free(index);
	free(line);
	free(temp);
}

////////////////////////////////////////////////////////////////////////////////

void read_zones(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
	int i, j;

	//open the file
	FILE *file = fopen(filename,"r");
	handle(file != NULL,"opening input file");

	//allocate the zone data
	void **data;
	data = fetch_allocate("csisd", MAX_ZONES);
	handle(data != NULL, "allocating fetch data");

	//fetch the data from the file
	*n_zones = fetch_read(file, "zone", "csisd", MAX_ZONES, data);
	handle(*n_zones != FETCH_FILE_ERROR && *n_zones != FETCH_MEMORY_ERROR && *n_zones > 0, "reading zones")

	//allocate the zone structures
	handle(allocate_mesh(0, 0, NULL, 0, NULL, 0, NULL, *n_zones, zone) == ALLOCATE_SUCCESS, "allocating zone structures");

	//get the zone parameters
	char *condition;
	for(i = 0; i < *n_zones; i ++)
	{
		//fetch_get("csisd", data, i, 0, &location[i]);
		fetch_get("csisd", data, i, 0, &(*zone)[i].location);
		fetch_get("csisd", data, i, 2, &(*zone)[i].variable);
		fetch_get("csisd", data, i, 3, &condition);
		strcpy((*zone)[i].condition,condition);
		fetch_get("csisd", data, i, 4, &(*zone)[i].value);
	}

	//temporary storage for face and cell zones
	int **face_zone, **cell_zone;
	handle(allocate_integer_matrix(&face_zone,n_faces,*n_zones) == ALLOCATE_SUCCESS, "allocating list of face zones");
	handle(allocate_integer_matrix(&cell_zone,n_cells,*n_zones) == ALLOCATE_SUCCESS, "allocating list of cell zones");

	//decode the index ranges
	char *range, *temp;
	int offset, index[2];
	handle(allocate_character_vector(&temp,MAX_STRING_CHARACTERS) == ALLOCATE_SUCCESS, "allocating temporary string");

	for(i = 0; i < *n_zones; i ++)
	{
		//get the range string
		fetch_get("csisd", data, i, 1, &range);

		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(range); j ++) if(range[j] == ',') range[j] = ' ';

		//sequentially read ranges
		offset = 0;
		while(offset < strlen(range))
		{
			//read the range from the string
			handle(sscanf(&range[offset],"%s",temp) == 1, "reading range string");
			handle(sscanf(temp,"%i:%i",&index[0],&index[1]) == 2, "getting indices from range string");

			//note the elements in the range which are a part of this zone
			if((*zone)[i].location == 'f') for(j = index[0]; j <= index[1]; j ++) face_zone[j][face[j].n_zones++] = i;
			if((*zone)[i].location == 'c') for(j = index[0]; j <= index[1]; j ++) cell_zone[j][cell[j].n_zones++] = i;

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}
	}

	//copy face and cell zone data into the structures
	handle(allocate_mesh(0, 0, NULL, n_faces, &face, n_cells, &cell, 0, NULL) == ALLOCATE_SUCCESS, "allocating cell and face zones");
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) face[i].zone[j] = &(*zone)[face_zone[i][j]];
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) cell[i].zone[j] = &(*zone)[cell_zone[i][j]];

	//clean up
	fetch_free("csisd", MAX_ZONES, data);
	free_matrix((void **)face_zone);
	free_matrix((void **)cell_zone);
	free_vector(temp);
}

////////////////////////////////////////////////////////////////////////////////

void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i, j, n;

	//open the file
	FILE *file = fopen(filename,"w");
	handle(file != NULL, "opening the case file");

	//temporary storage for element pointers cast to indices
	int *index;
	handle(allocate_integer_vector(&index,MAX_INDICES) == ALLOCATE_SUCCESS, "allocating indices");

	//number of variables
	handle(fwrite(&n_variables, sizeof(int), 1, file) == 1, "writing the number of variables");

	//nodes
	handle(fwrite(&n_nodes, sizeof(int), 1, file) == 1, "writing the number of nodes");
	handle(fwrite(node, sizeof(struct NODE), n_nodes, file) == n_nodes, "writing the nodes");

	//face nodes
	for(i = 0; i < n_faces; i ++) handle(fwrite(&(face[i].n_nodes), sizeof(int), 1, file) == 1, "writing the number of face nodes");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_nodes; j ++) index[j] = (int)(face[i].node[j] - &node[0]);
		handle(fwrite(index, sizeof(int), face[i].n_nodes, file) == face[i].n_nodes, "writing the face nodes");
	}

	//face border indices
	for(i = 0; i < n_faces; i ++) handle(fwrite(&(face[i].n_borders), sizeof(int), 1, file) == 1, "writing the number of face borders");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_borders; j ++) index[j] = (int)(face[i].border[j] - &cell[0]);
		handle(fwrite(index, sizeof(int), face[i].n_borders, file) == face[i].n_borders, "writing the face borders");
		handle(fwrite(face[i].oriented, sizeof(int), face[i].n_borders, file) == face[i].n_borders, "writing the face orientations");
	}

	//face zone indices
	for(i = 0; i < n_faces; i ++) handle(fwrite(&(face[i].n_zones), sizeof(int), 1, file) == 1, "writing the number of face zones");
	for(i = 0; i < n_faces; i ++)
	{
		for(j = 0; j < face[i].n_zones; j ++) index[j] = (int)(face[i].zone[j] - &zone[0]);
		handle(fwrite(index, sizeof(int), face[i].n_zones, file) == face[i].n_zones, "writing the face zones");
	}

	//face centroids
	for(i = 0; i < n_faces; i ++) handle(fwrite(face[i].centroid, sizeof(double), 2, file) == 2, "writing the face centroid");

	//cell node indices
	for(i = 0; i < n_cells; i ++) handle(fwrite(&(cell[i].n_faces), sizeof(int), 1, file) == 1, "writing the number of cell faces");
	for(i = 0; i < n_cells; i ++)
	{
		for(j = 0; j < cell[i].n_faces; j ++) index[j] = (int)(cell[i].face[j] - &face[0]);
		handle(fwrite(index, sizeof(int), cell[i].n_faces, file) == cell[i].n_faces, "writing the cell faces");
		handle(fwrite(cell[i].oriented, sizeof(int), cell[i].n_faces, file) == cell[i].n_faces, "writing the cell orientations");
	}

	//cell zone indices
	for(i = 0; i < n_cells; i ++) handle(fwrite(&(cell[i].n_zones), sizeof(int), 1, file) == 1, "writing the number of cell zones");
	for(i = 0; i < n_cells; i ++)
	{
		for(j = 0; j < cell[i].n_zones; j ++) index[j] = (int)(cell[i].zone[j] - &zone[0]);
		handle(fwrite(index, sizeof(int), cell[i].n_zones, file) == cell[i].n_zones, "writing the cell zones");
	}

	//cell centroids
	for(i = 0; i < n_cells; i ++) handle(fwrite(cell[i].centroid, sizeof(double), 2, file) == 2, "writing the cell centroid");

	//cell orders and stencil numbers
	for(i = 0; i < n_cells; i ++)
	{
		handle(fwrite(cell[i].order, sizeof(int), n_variables, file) == n_variables, "writing the cell orders");
		handle(fwrite(cell[i].n_stencil, sizeof(int), n_variables, file) == n_variables, "writing the cell stencil sizes");
	}
	//cell stencils and matrices
	for(i = 0; i < n_cells; i ++) {
		for(j = 0; j < n_variables; j ++) {
			handle(fwrite(cell[i].stencil[j], sizeof(int), cell[i].n_stencil[j], file) == cell[i].n_stencil[j],"writing the cell stencil");
			n = ORDER_TO_POWERS(cell[i].order[j]) * cell[i].n_stencil[j];
			handle(fwrite(cell[i].matrix[j], sizeof(double), n, file) == n,"writing the cell matrix");
		}
	}

	//zones
	handle(fwrite(&n_zones, sizeof(int), 1, file) == 1, "writing the number of zones");
	handle(fwrite(zone, sizeof(struct ZONE), n_zones, file) == n_zones, "writing the zones");

	//clean up
	free_vector(index);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////
