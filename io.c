////////////////////////////////////////////////////////////////////////////////

#include "caspar.h"
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

int read_geometry_file(char *filename, int *n_nodes, struct NODE *node, int *n_faces, struct FACE *face, int *n_cells, struct CELL *cell)
{
	//open file
	FILE *file = fopen(filename,"r");
	if(file == NULL) return ERROR;

	//allocate temporary storage
	char *line;
	if(allocate_character_vector(&line, MAX_STRING_LENGTH) != ALLOCATE_SUCCESS) return ERROR;

	//read each line in turn
	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
	{
		if(strncmp(line,"NODES",5) == 0)
		{
			sscanf(&line[6],"%i",n_nodes);
		}
		if(strncmp(line,"FACES",5) == 0)
		{
			sscanf(&line[6],"%i",n_faces);
		}
		if(strncmp(line,"CELLS",5) == 0)
		{
			sscanf(&line[6],"%i",n_cells);
		}
	}

	//clean up
	free(line);
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
