////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 128

#define FETCH_FILE_ERROR -1
#define FETCH_MEMORY_ERROR -1

////////////////////////////////////////////////////////////////////////////////

void **fetch_allocate_values(char *type, int max_n_lines);
int fetch_read_values(char *filename, char *label, char *type, int max_n_lines, void **value);
void fetch_print_values(char *type, int n_lines, void **value);
void fetch_free_values(char *type, int max_n_lines, void **value);

////////////////////////////////////////////////////////////////////////////////

int main()
{
	void **value;
	char type[6] = "icsisd";
	int max_n_lines = 20, n_lines;

	value = fetch_allocate_values(type,max_n_lines);
	n_lines = fetch_read_values("cavity/cavity.input", "zone", type, max_n_lines, value);
	fetch_print_values(type, n_lines, value);
	fetch_free_values(type, max_n_lines, value);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void **fetch_allocate_values(char *type, int max_n_lines)
{
        //counters
        int i, j, n = strlen(type);

	//size is the sum of all the types
	int size = 0;
	for(i = 0; i < n; i ++)
	{
		switch(type[i])
		{
			case 'i':
				size += sizeof(int);
				break;
			case 'f':
				size += sizeof(float);
				break;
			case 'd':
				size += sizeof(double);
				break;
			case 'c':
				size += sizeof(char);
				break;
			case 's':
				size += sizeof(char*);
				break;
		}
	}

        //pointer to the values
        void **value, *v;
	value = (void **)malloc(max_n_lines * sizeof(void *));
	value[0] = (void *)malloc(max_n_lines * size);

	v = value[0];

	for(i = 0; i < max_n_lines; i ++)
	{
		value[i] = v;

		for(j = 0; j < n; j ++)
		{
			switch(type[j])
			{
				case 'i':
					v = (int*)v + 1;
					break;
				case 'f':
					v = (float*)v + 1;
					break;
				case 'd':
					v = (double*)v + 1;
					break;
				case 'c':
					v = (char*)v + 1;
					break;
				case 's':
					*((char**)v) = (char*)malloc(MAX_STRING_LENGTH * sizeof(char));
					v = (char**)v + 1;
					break;
			}
		}
	}

	//return array
	return value;
}

////////////////////////////////////////////////////////////////////////////////

int fetch_read_values(char *filename, char *label, char *type, int max_n_lines, void **value)
{
	//open the file
	FILE *file;
	file = fopen(filename,"r");
	if(file == NULL) { printf("\nERROR - read_labelled_values - opening the file\n\n"); return FETCH_FILE_ERROR; }

	//counters
	int i, offset, n = strlen(type), n_lines = 0;

	//pointer to the current value
	void *v = value[0];

	//allocate temporary storage
	char *line, *line_label, *line_data;
	line = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	line_label = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	line_data = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));

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

				//point to the first value
				v = value[n_lines];

				//loop over the desired bits of data
				for(i = 0; i < n; i ++)
				{
					//read the data as a string
					if(sscanf(&line[offset], "%s", line_data) == 1)
					{
						//convert the string data to the desired type and increment the value pointer
						if(type[i] == 'i') {
							if(sscanf(line_data, "%i", (int*)v) != 1) break;
							v = (int*)v + 1;
						} else if(type[i] == 'f') {
							if(sscanf(line_data, "%f", (float*)v) != 1) break;
							v = (float*)v + 1;
						} else if(type[i] == 'd') {
							if(sscanf(line_data, "%lf", (double*)v) != 1) break;
							v = (double*)v + 1;
						} else if(type[i] == 'c') {
							if(sscanf(line_data, "%c", (char*)v) != 1) break;
							v = (char*)v + 1;
						} else if(type[i] == 's') {
							if(sscanf(line_data, "%s", *((char**)v)) != 1) break;
							v = (char**)v + 1;
						}

						//offset to the start of the next piece of data
						offset += strlen(line_data) + 1;
						while(line[offset] == ' ') offset ++;
					}
				}

				//increment the number of lines if all values succesfully read
				if(i == n) n_lines ++;

				//quit if the maximum number of lines has been reached
				if(n_lines == max_n_lines) break;
			}
		}
	}

	//clean up and return the number of lines read
	fclose(file);
	free(line);
	free(line_label);
	free(line_data);
	return n_lines;
}

////////////////////////////////////////////////////////////////////////////////

void fetch_print_values(char *type, int n_lines, void **value)
{
	int i, j, n = strlen(type);
	void *v;

	for(i = 0; i < n_lines; i ++)
	{
		v = value[i];
		for(j = 0; j < n; j ++)
		{
			switch(type[j])
			{
				case 'i':
					printf("%i ",*((int*)v));
					v = (int*)v + 1; break;
				case 'f':
					printf("%f ",*((float*)v));
					v = (float*)v + 1; break;
				case 'd':
					printf("%lf ",*((double*)v));
					v = (double*)v + 1; break;
				case 'c':
					printf("%c ",*((char*)v));
					v = (char*)v + 1; break;
				case 's':
					printf("%s ",*((char**)v));
					v = (char**)v + 1; break;
			}
		}
		printf("\n");
	}
}

////////////////////////////////////////////////////////////////////////////////

void fetch_free_values(char *type, int max_n_lines, void **value)
{
	int i, j, n = strlen(type);
	void *v;

	for(i = 0; i < max_n_lines; i ++)
	{
		v = value[i];
		for(j = 0; j < n; j ++)
		{
			switch(type[j])
			{
				case 'i':
					v = (int*)v + 1;
					break;
				case 'f':
					v = (float*)v + 1;
					break;
				case 'd':
					v = (double*)v + 1;
					break;
				case 'c':
					v = (char*)v + 1;
					break;
				case 's':
					free(*((char**)v));
					v = (char**)v + 1;
					break;
			}
		}
	}

	free(value);
}

////////////////////////////////////////////////////////////////////////////////
