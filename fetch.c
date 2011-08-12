////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

/*int main()
{
	void **data;
	char format[6] = "csisd";
	int max_n_lines = 5, n_lines;

	FILE *file;
	file = fopen("cavity/cavity.input","r");

	data = fetch_allocate(format, max_n_lines);
	n_lines = fetch_read(file, "zone", format, max_n_lines, data);

	fetch_print(format, n_lines, data);

	printf("\n");

	char *value;
	value = (char *)malloc(128 * sizeof(char));
	fetch_get(format, data, 3, 1, &value);
	printf("* %s *\n", value);

	fetch_free(format, max_n_lines, data);

	fclose(file);

	return 0;
}*/

////////////////////////////////////////////////////////////////////////////////

void **fetch_allocate(char *format, int max_n_lines)
{
        //counters
        int i, j, n_values = strlen(format);

	//size is the sum of all the data types
	int size = 0;
	for(i = 0; i < n_values; i ++)
	{
		switch(format[i])
		{
			case 'i': size += sizeof(int); break;
			case 'f': size += sizeof(float); break;
			case 'd': size += sizeof(double); break;
			case 'c': size += sizeof(char); break;
			case 's': size += sizeof(char*); break;
		}
	}

        //pointer to the data
        void **data, *d;
	data = (void **)malloc(max_n_lines * sizeof(void *));
	if(data == NULL) return NULL;
	data[0] = (void *)malloc(max_n_lines * size);
	if(data[0] == NULL) return NULL;

	d = data[0];

	for(i = 0; i < max_n_lines; i ++)
	{
		data[i] = d;

		for(j = 0; j < n_values; j ++)
		{
			switch(format[j])
			{
				case 'i': d = (int*)d + 1; break;
				case 'f': d = (float*)d + 1; break;
				case 'd': d = (double*)d + 1; break;
				case 'c': d = (char*)d + 1; break;
				case 's': *((char**)d) = (char*)malloc(MAX_STRING_CHARACTERS * sizeof(char));
					  if(*((char**)d) == NULL) { free(data[0]); free(data); return NULL; }
					  d = (char**)d + 1; break;
			}
		}
	}

	//return array
	return data;
}

////////////////////////////////////////////////////////////////////////////////

int fetch_read(FILE *file, char *label, char *format, int max_n_lines, void **data)
{
	//check the file
	if(file == NULL) { return FETCH_FILE_ERROR; }
	rewind(file);

	//counters
	int i, offset, n_values = strlen(format), n_lines = 0;

	//pointer to the current value
	void *d;

	//allocate temporary storage
	char *line, *line_label, *line_data;
	line = (char *)malloc(MAX_STRING_CHARACTERS * sizeof(char));
	if(line == NULL) { return FETCH_MEMORY_ERROR; }
	line_label = (char *)malloc(MAX_STRING_CHARACTERS * sizeof(char));
	if(line_label == NULL) { free(line); return FETCH_MEMORY_ERROR; }
	line_data = (char *)malloc(MAX_STRING_CHARACTERS * sizeof(char));
	if(line_data == NULL) { free(line); free(line_label); return FETCH_MEMORY_ERROR; }

	//read each line in turn
	while(fgets(line, MAX_STRING_CHARACTERS, file) != NULL)
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
				d = data[n_lines];

				//loop over the desired bits of data
				for(i = 0; i < n_values; i ++)
				{
					//eat up whitespace
					while(line[offset] == ' ') offset ++;

					//read the data as a string
					if(sscanf(&line[offset], "%s", line_data) == 1)
					{
						//convert the string data to the desired type and increment the value pointer
						if(format[i] == 'i') {
							if(sscanf(line_data, "%i", (int*)d) != 1) break;
							d = (int*)d + 1;
						} else if(format[i] == 'f') {
							if(sscanf(line_data, "%f", (float*)d) != 1) break;
							d = (float*)d + 1;
						} else if(format[i] == 'd') {
							if(sscanf(line_data, "%lf", (double*)d) != 1) break;
							d = (double*)d + 1;
						} else if(format[i] == 'c') {
							if(sscanf(line_data, "%c", (char*)d) != 1) break;
							d = (char*)d + 1;
						} else if(format[i] == 's') {
							if(sscanf(line_data, "%s", *((char**)d)) != 1) break;
							d = (char**)d + 1;
						}

						//offset to the start of the next piece of data
						offset += strlen(line_data) + 1;
					}
				}

				//increment the number of lines if all values succesfully read
				if(i == n_values) n_lines ++;

				//quit if the maximum number of lines has been reached
				if(n_lines == max_n_lines) break;
			}
		}
	}

	//clean up and return the number of lines read
	free(line);
	free(line_label);
	free(line_data);
	return n_lines;
}

////////////////////////////////////////////////////////////////////////////////

void fetch_get(char *format, void **data, int line_index, int value_index, void *value)
{
	int i;
	void *d = data[line_index];

	for(i = 0; i < value_index; i ++)
	{
		switch(format[i])
		{
			case 'i': d = (int*)d + 1; break;
			case 'f': d = (float*)d + 1; break;
			case 'd': d = (double*)d + 1; break;
			case 'c': d = (char*)d + 1; break;
			case 's': d = (char**)d + 1; break;
		}
	}

	switch(format[value_index])
	{
		case 'i': *((int*)value) = *((int*)d); return;
		case 'f': *((float*)value) = *((float*)d); return;
		case 'd': *((double*)value) = *((double*)d); return;
		case 'c': *((char*)value) = *((char*)d); return;
		case 's': *((char**)value) = *((char**)d); return;
	}
}

////////////////////////////////////////////////////////////////////////////////

void fetch_print(char *format, int n_lines, void **data)
{
	int i, j, n_values = strlen(format);
	void *d;

	for(i = 0; i < n_lines; i ++)
	{
		d = data[i];
		for(j = 0; j < n_values; j ++)
		{
			switch(format[j])
			{
				case 'i': printf("%i ",*((int*)d)); d = (int*)d + 1; break;
				case 'f': printf("%f ",*((float*)d)); d = (float*)d + 1; break;
				case 'd': printf("%lf ",*((double*)d)); d = (double*)d + 1; break;
				case 'c': printf("%c ",*((char*)d)); d = (char*)d + 1; break;
				case 's': printf("%s ",*((char**)d)); d = (char**)d + 1; break;
			}
		}
		printf("\n");
	}
}

////////////////////////////////////////////////////////////////////////////////

void fetch_free(char *format, int max_n_lines, void **data)
{
	int i, j, n_values = strlen(format);
	void *d;

	for(i = 0; i < max_n_lines; i ++)
	{
		d = data[i];
		for(j = 0; j < n_values; j ++)
		{
			switch(format[j])
			{
				case 'i': d = (int*)d + 1; break;
				case 'f': d = (float*)d + 1; break;
				case 'd': d = (double*)d + 1; break;
				case 'c': d = (char*)d + 1; break;
				case 's': free(*((char**)d)); 
					  d = (char**)d + 1; break;
			}
		}
	}

	free(data);
}

////////////////////////////////////////////////////////////////////////////////
