////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"
#include "fetch.h"

////////////////////////////////////////////////////////////////////////////////

int read_instructions(char *filename, int *n_variables, char **geometry_filename, int **maximum_order, double **weight_exponent, char ***connectivity)
{
	FILE *file = fopen(filename,"r");

        if(fetch_read(file, "number_of_variables", "i", 1, (void*)&n_variables) != 1)
        { printf("\nERROR - read_instructions - reading number_of_variables"); return ERROR; }

	if(allocate_instructions(*n_variables, geometry_filename, maximum_order, weight_exponent, connectivity) != SUCCESS)
	{ printf("\nERROR - read_instructions - allocating instructions"); return ERROR; }

        if(fetch_read(file, "geometry_filename", "s", 1, (void*)&geometry_filename) != 1)
        { printf("\nERROR - read_instructions - reading geometry_filename"); return ERROR; }

        char *format;
        if(allocate_character_vector(&format,*n_variables+1) != ALLOCATE_SUCCESS)
        { printf("\nERROR - read_instructions - allocating format"); return ERROR; }
        format[*n_variables] = '\0';

        memset(format,'i',*n_variables);

        if(fetch_read(file, "maximum_order", format, 1, (void*)maximum_order) != 1)
        { printf("\nERROR - read_instructions - reading maximum_order"); return ERROR; }

	int i;
	for(i = 0; i < *n_variables; i ++) (*maximum_order)[i] -= 1;

        memset(format,'d',*n_variables);

        if(fetch_read(file, "weight_exponent", format, 1, (void*)weight_exponent) != 1)
        { printf("\nERROR - read_instructions - reading weight_exponent"); return ERROR; }

        memset(format,'s',*n_variables);

        if(fetch_read(file, "connectivity", format, 1, (void*)connectivity) != 1)
        { printf("\nERROR - read_instructions - reading connectivity"); return ERROR; }

        fclose(file);
        free_vector(format);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	//open file
	FILE *file = fopen(filename,"r");
	if(file == NULL) return ERROR;

	//allocate temporary storage
	int i, j, *index, count, offset;
	char *line, *temp;
	if(allocate_integer_vector(&index,MAX_CELL_FACES) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating index"); return ERROR; }
	if(allocate_character_vector(&line, MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating line"); return ERROR; }
	if(allocate_character_vector(&temp,MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating temp"); return ERROR; }

	//read each line in turn
	while(fgets(line, MAX_STRING_CHARACTERS, file) != NULL)
	{
		//------------------------------------------------------------//

		if(strncmp(line,"NODES",5) == 0)
		{
			//number of nodes
			sscanf(&line[6],"%i",n_nodes);

			//allocate the nodes
			if(allocate_mesh(0, *n_nodes, node, 0, NULL, 0, NULL, 0, NULL) != SUCCESS)
			{ printf("\nERROR - read_geometry - allocating node structures"); return ERROR; }

			//read in the node locations
			for(i = 0; i < *n_nodes; i ++)
			{
				if(fscanf(file,"%lf %lf\n",&((*node)[i].x[0]),&((*node)[i].x[1])) != 2)
				{ printf("\nERROR - read_geometry_file - reading a node"); return ERROR; }
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"FACES",5) == 0)
		{
			//number of faces
			sscanf(&line[6],"%i",n_faces);

			//allocate the faces
			if(allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) != SUCCESS)
			{ printf("\nERROR - read_geometry - allocating face structures"); return ERROR; }

			//read in the face node indices and store pointers
			for(i = 0; i < *n_faces; i ++)
			{
				//read a line
				if(fgets(line, MAX_STRING_CHARACTERS, file) == NULL)
				{ printf("\nERROR - read_geometry_file - reading a face line"); return ERROR; }

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
				if(allocate_mesh(0, 0, NULL, *n_faces, face, 0, NULL, 0, NULL) != SUCCESS)
				{ printf("\nERROR - read_geometry - allocating face nodes"); return ERROR; }

				//node pointers
				for(j = 0; j < count; j ++) (*face)[i].node[j] = &((*node)[index[j]]);
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"CELLS",5) == 0)
		{
			//number of cells
			sscanf(&line[6],"%i",n_cells);

			//allocate the cells
			if(allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) != SUCCESS)
			{ printf("\nERROR - read_geometry - allocating cell structures"); return ERROR; }

			//read in the cell faces and store pointers
			for(i = 0; i < *n_cells; i ++)
			{
				//same as above but for cells
				if(fgets(line, MAX_STRING_CHARACTERS, file) == NULL)
				{ printf("\nERROR - read_geometry_file - reading a cell line"); return ERROR; }
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
				if(allocate_mesh(0, 0, NULL, 0, NULL, *n_cells, cell, 0, NULL) != SUCCESS)
				{ printf("\nERROR - read_geometry - allocating cell faces"); return ERROR; }
				for(j = 0; j < count; j ++) (*cell)[i].face[j] = &((*face)[index[j]]);
			}
		}

		//------------------------------------------------------------//

	}

	//check
	if(*node == NULL) { printf("\nERROR - read_geometry_file - nodes not found"); return ERROR; }
	if(*face == NULL) { printf("\nERROR - read_geometry_file - faces not found"); return ERROR; }
	if(*cell == NULL) { printf("\nERROR - read_geometry_file - cells not found"); return ERROR; }

	//clean up
	free(index);
	free(line);
	free(temp);
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int read_zones(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
	int i, j;

	//open the file
	FILE *file = fopen(filename,"r");
	if(file == NULL) return ERROR;

	//allocate the zone data
	void **data;
	data = fetch_allocate("csisd", MAX_ZONES);
	if(data == NULL) { printf("\nERROR - read_zones - allocating data"); return ERROR; }

	//fetch the data from the file
	*n_zones = fetch_read(file, "zone", "csisd", MAX_ZONES, data);
	if(*n_zones == FETCH_FILE_ERROR || *n_zones == FETCH_MEMORY_ERROR || *n_zones < 1)
	{ printf("\nERROR - read_zones - reading zones"); return ERROR; }

	//allocate the zone structures
	if(allocate_mesh(0, 0, NULL, 0, NULL, 0, NULL, *n_zones, zone) != SUCCESS)
	{ printf("\nERROR - read_geometry - allocating zone structures"); return ERROR; }

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
	if(allocate_integer_matrix(&face_zone,n_faces,*n_zones) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_zones - allocating face zone"); return ERROR; }
	if(allocate_integer_matrix(&cell_zone,n_cells,*n_zones) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_zones - allocating cell zone"); return ERROR; }

	//decode the index ranges
	char *range, *temp;
	int offset, index[2];
	if(allocate_character_vector(&temp,MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_zones - allocating temp"); return ERROR; }

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
			if(sscanf(&range[offset],"%s",temp) != 1)
			{ printf("\nERROR - read_zones - reading range"); return ERROR; }
			if(sscanf(temp,"%i:%i",&index[0],&index[1]) != 2)
			{ printf("\nERROR - read_zones - unrecognised range"); return ERROR; }

			//note the elements in the range which are a part of this zone
			if((*zone)[i].location == 'f') for(j = index[0]; j <= index[1]; j ++) face_zone[j][face[j].n_zones++] = i;
			if((*zone)[i].location == 'c') for(j = index[0]; j <= index[1]; j ++) cell_zone[j][cell[j].n_zones++] = i;

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}
	}

	//copy face and cell zone data into the structures
	if(allocate_mesh(0, 0, NULL, n_faces, &face, n_cells, &cell, 0, NULL) != SUCCESS)
	{ printf("\nERROR - read_geometry - allocating cell and face zones"); return ERROR; }
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) face[i].zone[j] = &(*zone)[face_zone[i][j]];
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) cell[i].zone[j] = &(*zone)[cell_zone[i][j]];

	//clean up
	fetch_free("csisd", MAX_ZONES, data);
	free_matrix((void **)face_zone);
	free_matrix((void **)cell_zone);
	free_vector(temp);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
