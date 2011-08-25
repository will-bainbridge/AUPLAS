////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "fetch.h"

#define ZONE_LABEL "zone"
#define ZONE_FORMAT "csisd"

#define MAX_ZONES_PER_ELEMENT 10

////////////////////////////////////////////////////////////////////////////////

struct ZONE * zones_new(int n_zones, struct ZONE *zone)
{
	int new = zone == NULL;

	zone = (struct ZONE *)realloc(zone, n_zones * sizeof(struct ZONE));
	if(zone == NULL) return NULL;

	if(new)
	{
		int i;
		for(i = 0; i < n_zones; i ++)
		{
			zone[i].location = '\0';
			zone[i].variable = -1;
			memset(zone[i].condition,'\0',MAX_CONDITION_CHARACTERS);
			zone[i].value = 0.0;
		}
	}

	return zone;
}

////////////////////////////////////////////////////////////////////////////////

void zones_input(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
        int i, j, n = 0, info;

        //open the file
        FILE *file = fopen(filename,"r");
        handle(1,file != NULL,"opening input file");

        //fetch the data from the file
        FETCH fetch = fetch_new(ZONE_FORMAT, MAX_ZONES);
        handle(1,fetch != NULL,"allocating zone input");
        int n_fetch = fetch_read(file, ZONE_LABEL, fetch);
	handle(1,n_fetch > 1,"no zones found in input file");
        handle(0,n_fetch < MAX_ZONES,"maximum number of zones reached");

	//allocate zones
	struct ZONE *z = zones_new(n_fetch, NULL);
	handle(1,z != NULL,"allocating zones");

        //temporary storage
        int offset, index[2];
        char *range = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	handle(1,range != NULL && temp != NULL,"allocating temporary storage");

	//allocate face and cell zones
	for(i = 0; i < n_faces; i ++)
	{
		face[i].n_zones = 0;
		face[i].zone = (struct ZONE **)malloc(MAX_ZONES_PER_ELEMENT * sizeof(struct ZONE *));
		handle(1,face[i].zone != NULL,"allocating face zone");
	}
	for(i = 0; i < n_cells; i ++)
	{
		cell[i].n_zones = 0;
		cell[i].zone = (struct ZONE **)malloc(MAX_ZONES_PER_ELEMENT * sizeof(struct ZONE *));
		handle(1,cell[i].zone != NULL,"allocating cell zone");
	}

	//consider each feteched line
	for(i = 0; i < n_fetch; i ++)
	{
		//get zone parameters
		fetch_get(fetch, i, 0, &z[n].location);
                fetch_get(fetch, i, 2, &z[n].variable);
                fetch_get(fetch, i, 3, z[n].condition);
                fetch_get(fetch, i, 4, &z[n].value);

		//get the range string
                fetch_get(fetch, i, 1, range);

                //convert comma delimiters to whitespace
                for(j = 0; j < strlen(range); j ++) if(range[j] == ',') range[j] = ' ';

                //sequentially read ranges
                offset = info = 0;
                while(offset < strlen(range))
                {
                        //read the range from the string
			info = sscanf(&range[offset],"%s",temp) == 1;
			info *= sscanf(temp,"%i:%i",&index[0],&index[1]) == 2;
			handle(0,info,"skipping zone with unrecognised range");
			if(!info) break;

                        //store zone in the elements in the range
                        if(z[n].location == 'f') for(j = index[0]; j <= index[1]; j ++) face[j].zone[face[j].n_zones++] = &z[n];
                        if(z[n].location == 'c') for(j = index[0]; j <= index[1]; j ++) cell[j].zone[cell[j].n_zones++] = &z[n];

                        //move to the next range in the string
                        offset += strlen(temp) + 1;
                }
		
		//increment zone
		n += info;
	}

	//re-allocate face and cell zones
	for(i = 0; i < n_faces; i ++)
	{
		face[i].zone = (struct ZONE **)realloc(face[i].zone, face[i].n_zones * sizeof(struct ZONE *));
		handle(1,face[i].zone != NULL,"re-allocating face zone");
	}
	for(i = 0; i < n_cells; i ++)
	{
		cell[i].zone = (struct ZONE **)realloc(cell[i].zone, cell[i].n_zones * sizeof(struct ZONE *));
		handle(1,cell[i].zone != NULL,"re-allocating cell zone");
	}

	//resize zone list
	struct ZONE *z_new = zones_new(n, z);
        handle(1,zone != NULL,"re-allocating zones");
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) face[i].zone[j] += z_new - z;
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) cell[i].zone[j] += z_new - z;

	//check numbers
        fetch_destroy(fetch);
        fetch = fetch_new("",MAX_ZONES);
        handle(0,fetch_read(file,ZONE_LABEL,fetch) == n,"skipping zones with unrecognised formats");

	//copy over
        *n_zones = n;
        *zone = z_new;

	//clean up
	fclose(file);
	fetch_destroy(fetch);
	free(range);
	free(temp);
}

////////////////////////////////////////////////////////////////////////////////

void zone_case_write(FILE *file, struct ZONE *zone)
{
	handle(1,fwrite(&(zone->location), sizeof(char), 1, file) == 1,"writing the zone location");
	handle(1,fwrite(&(zone->variable), sizeof(int), 1, file) == 1,"writing the zone variable");
	handle(1,fwrite(zone->condition, sizeof(char), MAX_CONDITION_CHARACTERS, file) == MAX_CONDITION_CHARACTERS,"writing the zone condition");
	handle(1,fwrite(&(zone->value), sizeof(double), 1, file) == 1,"writing the zone value");
}
void zone_case_get(FILE *file, struct ZONE *zone)
{
	handle(1,fread(&(zone->location), sizeof(char), 1, file) == 1,"reading the zone location");
	handle(1,fread(&(zone->variable), sizeof(int), 1, file) == 1,"reading the zone variable");
	handle(1,fread(zone->condition, sizeof(char), MAX_CONDITION_CHARACTERS, file) == MAX_CONDITION_CHARACTERS,"reading the zone condition");
	handle(1,fread(&(zone->value), sizeof(double), 1, file) == 1,"reading the zone value");
}

////////////////////////////////////////////////////////////////////////////////

void zones_destroy(struct ZONE *zone)
{
	free(zone);
}

////////////////////////////////////////////////////////////////////////////////
