////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"
#include "fetch.h"

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
	{ printf("\nERROR - read_geometry_file - allocating index memory"); return ERROR; }
	if(allocate_character_vector(&line, MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating line memory"); return ERROR; }
	if(allocate_character_vector(&temp,MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating temp memory"); return ERROR; }

	//read each line in turn
	while(fgets(line, MAX_STRING_CHARACTERS, file) != NULL)
	{

		//------------------------------------------------------------//

		if(strncmp(line,"NODES",5) == 0)
		{
			//number of nodes
			sscanf(&line[6],"%i",n_nodes);

			//allocate the nodes
			*node = (struct NODE *)malloc(*n_nodes * sizeof(struct NODE));
			if(*node == NULL) { printf("\nERROR - read_geometry_file - allocating node memory"); return ERROR; }

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
			*face = (struct FACE *)malloc(*n_faces * sizeof(struct FACE));
			if(*face == NULL) { printf("\nERROR - read_geometry_file - allocating face memory"); return ERROR; }

			//read in the face node indices and store pointers
			for(i = 0; i < *n_faces; i ++)
			{
				if(fscanf(file,"%i %i\n",&index[0],&index[1]) != 2)
				{ printf("\nERROR - read_geometry_file - reading a face"); return ERROR; }

				for(j = 0; j < 2; j ++) (*face)[i].node[j] = &((*node)[index[j]]);
			}
		}

		//------------------------------------------------------------//

		else if(strncmp(line,"CELLS",5) == 0)
		{
			//number of cells
			sscanf(&line[6],"%i",n_cells);

			//allocate the cells
			*cell = (struct CELL *)malloc(*n_cells * sizeof(struct CELL));
			if(*cell == NULL) { printf("\nERROR - read_geometry_file - allocating cell memory"); return ERROR; }

			//read in the cell faces and store pointers
			for(i = 0; i < *n_cells; i ++)
			{
				//read a line
				if(fgets(line, MAX_STRING_CHARACTERS, file) == NULL)
				{ printf("\nERROR - read_geometry_file - reading a cell line"); return ERROR; }

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
				(*cell)[i].n_faces = count;

				//allocate the faces
				(*cell)[i].face = (struct FACE **)malloc(count * sizeof(struct FACE *));
				if((*cell)[i].face == NULL) { printf("\nERROR - read_geometry_file - allocating cell face memory"); return ERROR; }

				//face pointers
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

int read_zones(char *filename, struct FACE *face, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
	int i, j;

	//open the file
	FILE *file = fopen(filename,"r");
	if(file == NULL) return ERROR;

	//allocate the zone data
	void **data;
	data = fetch_allocate("csisd", MAX_ZONES);
	if(data == NULL) { printf("\nERROR - read_zones - allocating data memory"); return ERROR; }

	//fetch the data from the file
	*n_zones = fetch_read(file, "zone", "csisd", MAX_ZONES, data);
	if(*n_zones == FETCH_FILE_ERROR || *n_zones == FETCH_MEMORY_ERROR || *n_zones < 1)
	{ printf("\nERROR - read_zones - reading zones"); return ERROR; }

	//fetch_print("csisd",*n_zones,data);

	//allocate the zone structures
	*zone = (struct ZONE *)malloc(*n_zones * sizeof(struct ZONE));
	if(data == NULL) { printf("\nERROR - read_zones - allocating data memory"); return ERROR; }

	//get the zone parameters
	char *condition, *location;
	if(allocate_character_vector(&location,*n_zones) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_zones - allocating location memory"); return ERROR; }
	for(i = 0; i < *n_zones; i ++)
	{
		fetch_get("csisd", data, i, 0, &location[i]);
		fetch_get("csisd", data, i, 2, &(*zone)[i].variable);
		fetch_get("csisd", data, i, 3, &condition);
		strcpy((*zone)[i].condition,condition);
		fetch_get("csisd", data, i, 4, &(*zone)[i].value);
	}

	//decode the index ranges
	char *range, *temp;
	int offset, index[2];
	if(allocate_character_vector(&temp,MAX_STRING_CHARACTERS) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_zones - allocating temp memory"); return ERROR; }

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

			//put pointer to the zone in the relevent elements
			if(location[i] == 'f')
			{
				for(j = index[0]; j <= index[1]; j ++)
				{
					face[j].zone = realloc(face[j].zone,(face[j].n_zones+1)*sizeof(struct ZONE *));
					face[j].zone[face[j].n_zones++] = &(*zone)[i];
				}
			}
			if(location[i] == 'c')
			{
				for(j = index[0]; j <= index[1]; j ++)
				{
					cell[j].zone = realloc(cell[j].zone,(cell[j].n_zones+1)*sizeof(struct ZONE *));
					cell[j].zone[cell[j].n_zones++] = &(*zone)[i];
				}
			}

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}
	}

	//clean up
	fetch_free("csisd", MAX_ZONES, data);
	free_vector(temp);
	free_vector(location);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
