////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

////////////////////////////////////////////////////////////////////////////////

int read_labelled_values_from_file(FILE *file, char *label, char type, int n, void *value)
{
        //check input file pointer
        if(file == NULL) return ERROR;

        //counters
        int i, offset;

        //allocate temporary storage
        char *line, *line_label, *line_data;
        if(allocate_character_vector(&line, MAX_STRING_LENGTH) != ALLOCATE_SUCCESS) return ERROR;
        if(allocate_character_vector(&line_label, MAX_STRING_LENGTH) != ALLOCATE_SUCCESS) return ERROR;
        if(allocate_character_vector(&line_data, MAX_STRING_LENGTH) != ALLOCATE_SUCCESS) return ERROR;

        //start at the beginning of the file
        rewind(file);

        //read each line in turn
        while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
        {
                // get the first string on the line
                if(sscanf(line, "%s", line_label) == 1)
                {
                        //check against the specified label
                        if(strcmp(line_label, label) == 0)
                        {
                                //offset to the start of the data
                                offset = strlen(label) + 1;

                                //loop over the desired bits of data
                                for(i = 0; i < n; i ++)
                                {
                                        //read the data as a string
                                        if(sscanf(&line[offset], "%s", line_data) == 1)
                                        {
                                                //convert the string data to the desired type
                                                if     (type == 'i') { if(sscanf(line_data,  "%i",    &((int*)value)[i]) != 1) break; }
                                                else if(type == 'f') { if(sscanf(line_data,  "%f",  &((float*)value)[i]) != 1) break; }
                                                else if(type == 'd') { if(sscanf(line_data, "%lf", &((double*)value)[i]) != 1) break; }
                                                else if(type == 'c') { if(sscanf(line_data,  "%c",   &((char*)value)[i]) != 1) break; }
                                                else if(type == 's') { if(sscanf(line_data,  "%s",   ((char**)value)[i]) != 1) break; }

                                                //offset to the start of the next piece of data
                                                offset += strlen(line_data) + 1;
                                        }
                                }

                                //clean up and return successful if all pieces of data have been successfully read
                                if(i == n)
                                {
                                        free_vector(line);free_vector(line_label);free_vector(line_data);
                                        return SUCCESS;
                                }
                        }
                }
        }

        //clean up and return error if the desired label and data were not found
        free_vector(line);free_vector(line_label);free_vector(line_data);
        return ERROR;
}

////////////////////////////////////////////////////////////////////////////////

int read_geometry_file(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
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
