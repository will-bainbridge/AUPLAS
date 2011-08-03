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
	if(allocate_character_vector(&line, MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating line memory"); return ERROR; }
	if(allocate_character_vector(&temp,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - read_geometry_file - allocating temp memory"); return ERROR; }

	//read each line in turn
	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
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
				if(fgets(line, MAX_STRING_LENGTH, file) == NULL)
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

int read_zones(char *filename, int *n_zones, struct ZONE **zone)
{
	FILE *file = fopen(filename,"r");
	if(file == NULL) return ERROR;

	void **data;
	data = fetch_allocate("icsisd", MAX_N_ZONES);

	if(data == NULL) { printf("\nERROR - read_zones - allocating data memory"); return ERROR; }

	*n_zones = fetch_read(file, "zone", "icsisd", MAX_N_ZONES, data);

	if(*n_zones == FETCH_FILE_ERROR || *n_zones == FETCH_MEMORY_ERROR || *n_zones < 1)
	{ printf("\nERROR - read_zones - reading zones"); return ERROR; }

	//fetch_print("icsisd", *n_zones, data);

	fetch_free("icsisd", MAX_N_ZONES, data);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
