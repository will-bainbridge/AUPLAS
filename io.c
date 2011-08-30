////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "differential.h"
#include "fetch.h"

#define MAX_FACE_NODES 2
#define MAX_CELL_FACES 5

#define ZONE_LABEL "zone"
#define ZONE_FORMAT "csisd"

#define DIVERGENCE_LABEL "divergence"
#define DIVERGENCE_FORMAT "iscsd"

#define MAX_DIVERGENCES 100
#define MAX_DIVERGENCE_VARIABLES 5

void node_geometry_get(FILE *file, struct NODE *node);
void node_case_write(FILE *file, struct NODE *node);
void node_case_get(FILE *file, struct NODE *node);

void face_geometry_get(FILE *file, struct FACE *face, struct NODE *node);
void face_case_write(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone);
void face_case_get(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone);

void cell_geometry_get(FILE *file, struct CELL *cell, struct FACE *face);
void cell_case_write(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone);
void cell_case_get(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone);

void zone_case_write(FILE *file, struct ZONE *zone);
void zone_case_get(FILE *file, struct ZONE *zone);


////////////////////////////////////////////////////////////////////////////////

void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening geometry file");

	char *line;
	handle(1,allocate_character_vector(&line, MAX_STRING_LENGTH),"allocating line string");

	int i;

	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
	{
		if(strncmp(line,"NODES",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_nodes) == 1,"reading the number of nodes");
			*node = nodes_new(*n_nodes, NULL);
			handle(1,*node != NULL,"allocating the nodes");
			for(i = 0; i < *n_nodes; i ++) node_geometry_get(file, &(*node)[i]);
		}
		if(strncmp(line,"FACES",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_faces) == 1,"reading the number of faces");
			*face = faces_new(*n_faces, NULL);
			handle(1,*face != NULL,"allocating the faces");
			for(i = 0; i < *n_faces; i ++) face_geometry_get(file, &(*face)[i], *node);
		}
		if(strncmp(line,"CELLS",5) == 0)
		{
			handle(1,sscanf(&line[6],"%i",n_cells) == 1,"reading the number of cells");
			*cell = cells_new(*n_cells, NULL);
			handle(1,*cell != NULL,"allocating the cells");
			for(i = 0; i < *n_cells; i ++) cell_geometry_get(file, &(*cell)[i], *face);
		}
	}

	handle(1,*n_nodes > 0,"finding nodes in the geometry file");
	handle(1,*n_faces > 0,"finding faces in the geometry file");
	handle(1,*n_cells > 0,"finding cells in the geometry file");

	free(line);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void node_geometry_get(FILE *file, struct NODE *node)
{
	int info = fscanf(file,"%lf %lf\n",&(node->x[0]),&(node->x[1]));
	handle(1,info == 2, "reading a node's coordinates");
}

////////////////////////////////////////////////////////////////////////////////

void face_geometry_get(FILE *file, struct FACE *face, struct NODE *node)
{
        int i;

        //temporary storage
        int *index, count, offset;
        char *line, *temp;
        index = (int *)malloc(MAX_FACE_NODES * sizeof(int));
        line = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
        temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
        handle(1,index != NULL && line != NULL && temp != NULL ,"allocating temporary storage");

        //read a line
        handle(1,fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a face line");

        //strip newlines and whitespace off the end of the line
        for(i = strlen(line)-1; i >= 0; i --) if(line[i] != ' ' && line[i] != '\n') break;
        line[i+1] = '\0';

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
        face->n_nodes = count;

        //allocate the faces
        handle(1,face_node_new(face),"allocating face nodes");

        //node pointers
        for(i = 0; i < count; i ++) face->node[i] = &node[index[i]];

        //clean up
        free(index);
        free(line);
        free(temp);
}

////////////////////////////////////////////////////////////////////////////////

void cell_geometry_get(FILE *file, struct CELL *cell, struct FACE *face)
{
        int i;

        //temporary storage
        int *index, count, offset;
        char *line, *temp;
        index = (int *)malloc(MAX_CELL_FACES * sizeof(int));
        line = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
        temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
        handle(1,index != NULL && line != NULL && temp != NULL ,"allocating temporary storage");

        //read the line
        handle(1,fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a cell line");

        //eat up whitespace and newlines
        for(i = strlen(line)-1; i >= 0; i --) if(line[i] != ' ' && line[i] != '\n') break;
        line[i+1] = '\0';

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
        cell->n_faces = count;

        //allocate the faces
        handle(1,cell_face_new(cell),"allocating cell faces");

        //face pointers
        for(i = 0; i < count; i ++) cell->face[i] = &face[index[i]];

        //clean up
        free(index);
        free(line);
        free(temp);
}

////////////////////////////////////////////////////////////////////////////////

void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i;

	//open the file
	FILE *file = fopen(filename,"w");
	handle(1,file != NULL, "opening the case file");

	//number of variables
	handle(1,fwrite(&n_variables, sizeof(int), 1, file) == 1, "writing the number of variables");

	//numbers of elements
	handle(1,fwrite(&n_nodes, sizeof(int), 1, file) == 1, "writing the number of nodes");
	handle(1,fwrite(&n_faces, sizeof(int), 1, file) == 1, "writing the number of faces");
	handle(1,fwrite(&n_cells, sizeof(int), 1, file) == 1, "writing the number of cells");
	handle(1,fwrite(&n_zones, sizeof(int), 1, file) == 1, "writing the number of zones");

	//elements
	for(i = 0; i < n_nodes; i ++) node_case_write(file, &node[i]);
	for(i = 0; i < n_faces; i ++) face_case_write(file, node, &face[i], cell, zone);
	for(i = 0; i < n_cells; i ++) cell_case_write(file, n_variables, face, &cell[i], zone);
	for(i = 0; i < n_zones; i ++) zone_case_write(file, &zone[i]);

	//clean up
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void read_case(char *filename, int *n_variables, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell, int *n_zones, struct ZONE **zone)
{
	int i;

	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL, "opening the case file");

	//number of variables
	handle(1,fread(n_variables, sizeof(int), 1, file) == 1, "reading the number of variables");

	//numbers of elements
	handle(1,fread(n_nodes, sizeof(int), 1, file) == 1, "reading the number of nodes");
	handle(1,(*node = nodes_new(*n_nodes,NULL)) != NULL,"allocating nodes");
	handle(1,fread(n_faces, sizeof(int), 1, file) == 1, "reading the number of faces");
	handle(1,(*face = faces_new(*n_faces,NULL)) != NULL,"allocating faces");
	handle(1,fread(n_cells, sizeof(int), 1, file) == 1, "reading the number of cells");
	handle(1,(*cell = cells_new(*n_cells,NULL)) != NULL,"allocating cells");
	handle(1,fread(n_zones, sizeof(int), 1, file) == 1, "reading the number of zones");
	handle(1,(*zone = zones_new(*n_zones,NULL)) != NULL,"allocating zones");

	//elements
	for(i = 0; i < *n_nodes; i ++) node_case_get(file, &(*node)[i]);
	for(i = 0; i < *n_faces; i ++) face_case_get(file, *node, &(*face)[i], *cell, *zone);
	for(i = 0; i < *n_cells; i ++) cell_case_get(file, *n_variables, *face, &(*cell)[i], *zone);
	for(i = 0; i < *n_zones; i ++) zone_case_get(file, &(*zone)[i]);

	//clean up
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void node_case_write(FILE *file, struct NODE *node)
{
	        handle(1,fwrite(node->x, sizeof(double), 2, file) == 2, "writing the node location");
}

////////////////////////////////////////////////////////////////////////////////

void node_case_get(FILE *file, struct NODE *node)
{
	        handle(1,fread(node->x, sizeof(double), 2, file) == 2, "reading the node location");
}

////////////////////////////////////////////////////////////////////////////////

void face_case_write(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	handle(1,index != NULL,"allocating temporary storage");

	handle(1,fwrite(&(face->n_nodes), sizeof(int), 1, file) == 1, "writing the number of face nodes");
	for(i = 0; i < face->n_nodes; i ++) index[i] = (int)(face->node[i] - &node[0]);
	handle(1,fwrite(index, sizeof(int), face->n_nodes, file) == face->n_nodes, "writing the face nodes");
	handle(1,fwrite(face->centroid, sizeof(double), 2, file) == 2, "writing the face centroid");
	handle(1,fwrite(&(face->n_borders), sizeof(int), 1, file) == 1, "writing the number of face borders");
	for(i = 0; i < face->n_borders; i ++) index[i] = (int)(face->border[i] - &cell[0]);
	handle(1,fwrite(index, sizeof(int), face->n_borders, file) == face->n_borders, "writing the face borders");
	handle(1,fwrite(face->oriented, sizeof(int), face->n_borders, file) == face->n_borders, "writing the face orientations");
	handle(1,fwrite(&(face->n_zones), sizeof(int), 1, file) == 1, "writing the number of face zones");
	for(i = 0; i < face->n_zones; i ++) index[i] = (int)(face->zone[i] - &zone[0]);
	handle(1,fwrite(index, sizeof(int), face->n_zones, file) == face->n_zones, "writing the face zones");

	free(index);
}

////////////////////////////////////////////////////////////////////////////////

void face_case_get(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	handle(1,index != NULL,"allocating temporary storage");

	handle(1,fread(&(face->n_nodes), sizeof(int), 1, file) == 1, "reading the number of face nodes");
	handle(1,face_node_new(face),"allocating face nodes");
	handle(1,fread(index, sizeof(int), face->n_nodes, file) == face->n_nodes, "reading face nodes");
	for(i = 0; i < face->n_nodes; i ++) face->node[i] = &node[index[i]];
	handle(1,fread(face->centroid, sizeof(double), 2, file) == 2, "reading the face centroid");
	handle(1,fread(&(face->n_borders), sizeof(int), 1, file) == 1, "reading the number of face borders");
	handle(1,face_border_new(face),"allocating face borders");
	handle(1,fread(index, sizeof(int), face->n_borders, file) == face->n_borders, "reading the face borders");
	for(i = 0; i < face->n_borders; i ++) face->border[i] = &cell[index[i]];
	handle(1,face_oriented_new(face),"allocating face orientations");
	handle(1,fread(face->oriented, sizeof(int), face->n_borders, file) == face->n_borders, "reading the face orientations");
	handle(1,fread(&(face->n_zones), sizeof(int), 1, file) == 1, "reading the number of face zones");
	handle(1,face_zone_new(face),"allocating face zones");
	handle(1,fread(index, sizeof(int), face->n_zones, file) == face->n_zones, "reading the face zones");
	for(i = 0; i < face->n_zones; i ++) face->zone[i] = &zone[index[i]];

	free(index);
}

////////////////////////////////////////////////////////////////////////////////

void cell_case_write(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
        int i, n, *index;

        index = (int *)malloc(MAX_INDICES * sizeof(int));
        handle(1,index != NULL,"allocating temporary storage");

        handle(1,fwrite(&(cell->n_faces), sizeof(int), 1, file) == 1, "writing the number of cell faces");
        for(i = 0; i < cell->n_faces; i ++) index[i] = (int)(cell->face[i] - &face[0]);
        handle(1,fwrite(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell faces");
        handle(1,fwrite(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell orientations");
        handle(1,fwrite(cell->centroid, sizeof(double), 2, file) == 2, "writing the cell centroid");
        handle(1,fwrite(&(cell->n_zones), sizeof(int), 1, file) == 1, "writing the number of cell zones");
        for(i = 0; i < cell->n_zones; i ++) index[i] = (int)(cell->zone[i] - &zone[0]);
        handle(1,fwrite(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "writing the cell zones");
        handle(1,fwrite(cell->order, sizeof(int), n_variables, file) == n_variables, "writing the cell orders");
        handle(1,fwrite(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "writing the cell stencil sizes");
        for(i = 0; i < n_variables; i ++)
        {
                handle(1,fwrite(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"writing the cell stencil");
                n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
                handle(1,fwrite(cell->matrix[i][0], sizeof(double), n, file) == n,"writing the cell matrix");
        }

        free(index);
}

////////////////////////////////////////////////////////////////////////////////

void cell_case_get(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
        int i, n, *index;

        index = (int *)malloc(MAX_INDICES * sizeof(int));
        handle(1,index != NULL,"allocating temporary storage");

        handle(1,fread(&(cell->n_faces), sizeof(int), 1, file) == 1, "reading the number of cell faces");
        handle(1,cell_face_new(cell),"allocating cell faces");
        handle(1,fread(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading the cell faces");
        for(i = 0; i < cell->n_faces; i ++) cell->face[i] = &face[index[i]];
        handle(1,cell_oriented_new(cell),"allocating cell orientations");
        handle(1,fread(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading cell orientations");
        handle(1,fread(cell->centroid, sizeof(double), 2, file) == 2, "reading the cell centroid");
        handle(1,fread(&(cell->n_zones), sizeof(int), 1, file) == 1, "reading the number of cell zones");
        handle(1,cell_zone_new(cell),"allocating cell zones");
        handle(1,fread(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "reading the cell zones");
        for(i = 0; i < cell->n_zones; i ++) cell->zone[i] = &zone[index[i]];
        handle(1,cell_order_new(n_variables,cell),"allocating cell orders");
        handle(1,fread(cell->order, sizeof(int), n_variables, file) == n_variables, "reading the cell orders");
        handle(1,cell_n_stencil_new(n_variables,cell),"allocating cell stencil sizes");
        handle(1,fread(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "reading the cell stencil sizes");
        handle(1,cell_stencil_new(n_variables,cell),"allocating cell stencils");
        handle(1,cell_matrix_new(n_variables,cell),"allocating cell matrices");
        for(i = 0; i < n_variables; i ++)
        {
                handle(1,fread(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"reading the cell stencil");
                n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
                handle(1,fread(cell->matrix[i][0], sizeof(double), n, file) == n,"reading the cell matrix");
        }

        free(index);
}

////////////////////////////////////////////////////////////////////////////////

void zone_case_write(FILE *file, struct ZONE *zone)
{
	handle(1,fwrite(&(zone->location), sizeof(char), 1, file) == 1,"writing the zone location");
	handle(1,fwrite(&(zone->variable), sizeof(int), 1, file) == 1,"writing the zone variable");
	handle(1,fwrite(zone->condition, sizeof(char), MAX_CONDITION_LENGTH, file) == MAX_CONDITION_LENGTH,"writing the zone condition");
	handle(1,fwrite(&(zone->value), sizeof(double), 1, file) == 1,"writing the zone value");
}

////////////////////////////////////////////////////////////////////////////////

void zone_case_get(FILE *file, struct ZONE *zone)
{
	handle(1,fread(&(zone->location), sizeof(char), 1, file) == 1,"reading the zone location");
	handle(1,fread(&(zone->variable), sizeof(int), 1, file) == 1,"reading the zone variable");
	handle(1,fread(zone->condition, sizeof(char), MAX_CONDITION_LENGTH, file) == MAX_CONDITION_LENGTH,"reading the zone condition");
	handle(1,fread(&(zone->value), sizeof(double), 1, file) == 1,"reading the zone value");
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
			if(z[n].location == 'f') for(j = index[0]; j <= index[1]; j ++) handle(1,face_zone_add(&face[j],&z[n]),"adding a face zone");
			if(z[n].location == 'c') for(j = index[0]; j <= index[1]; j ++) handle(1,cell_zone_add(&cell[j],&z[n]),"adding a cell zone");

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}

		//increment zone
		n += info;
	}

	//check numbers
	fetch_destroy(fetch);
	fetch = fetch_new("",MAX_ZONES);
	handle(0,fetch_read(file,ZONE_LABEL,fetch) == n,"skipping zones with unrecognised formats");

	//resize zone list
	struct ZONE *z_new = zones_new(n, z);
	handle(1,zone != NULL,"re-allocating zones");
	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) face[i].zone[j] += z_new - z;
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) cell[i].zone[j] += z_new - z;

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

void divergences_input(char *filename, int *n_divergences, struct DIVERGENCE **divergence)
{
	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening input file");

	//fetch the data
	FETCH fetch = fetch_new(DIVERGENCE_FORMAT,MAX_DIVERGENCES);
	handle(1,fetch != NULL,"allocating fetch");
	int n_fetch = fetch_read(file,DIVERGENCE_LABEL,fetch);
	handle(1,n_fetch > 1,"no divergences found in input file");
	handle(0,n_fetch < MAX_DIVERGENCES,"maximum number of divergences reached");

	//allocate pointers
	struct DIVERGENCE *d = divergences_new(NULL,0,n_fetch);
	handle(1,d != NULL,"allocating divergences");

	//counters
	int i, j, n = 0, info;

	//temporary storage
	char direction;
	int offset, n_diff, diff[2];
	char *piece = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	int *term = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	int *differential = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	handle(1,piece != NULL && temp != NULL && term != NULL && differential != NULL,"allocating temporary storage");

	for(i = 0; i < n_fetch; i ++)
	{
		//equation
		fetch_get(fetch, i, 0, &d[n].equation);

		//constant
		fetch_get(fetch, i, 4, &d[n].constant);

		//direction
		fetch_get(fetch, i, 2, &direction);
		if(direction == 'x') {
			d[n].direction = 0;
		} else if(direction == 'y') {
			d[n].direction = 1;
		} else {
			handle(1,info = 0,"skipping divergence with unrecognised direction");
			continue;
		}

		//variables
		fetch_get(fetch, i, 1, piece);
		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(piece); j ++) if(piece[j] == ',') piece[j] = ' ';
		//sequentially read variables
		offset = d[n].n_variables = 0;
		while(offset < strlen(piece))
		{
			//read the variable from the string
			info = sscanf(&piece[offset],"%s",temp) == 1;
			info *= sscanf(temp,"%i",&term[d[n].n_variables++]) == 1;
			handle(0,info,"skipping divergence with unrecognised variable format");
			if(!info) continue;

			//move to the next variable in the string
			offset += strlen(temp) + 1;
		}

		//differentials
		fetch_get(fetch, i, 3, piece);
		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(piece); j ++) if(piece[j] == ',') piece[j] = ' ';
		//sequentially read differentials
		offset = n_diff = 0;
		while(offset < strlen(piece))
		{
			//read the variables' differential string
			info = sscanf(&piece[offset],"%s",temp) == 1;
			handle(0,info,"skipping divergence with unrecognised differentail format");
			if(!info) continue;

			//count the differentials in the different dimensions
			j = diff[0] = diff[1] = 0;
			while(temp[j] != '\0')
			{
				diff[0] += (temp[j] == 'x');
				diff[1] += (temp[j] == 'y');
				j ++;
			}
			//convert to a unique differential index
			differential[n_diff ++] = differential_index[diff[0]][diff[1]];
			//move to the next differential in the string
			offset += strlen(temp) + 1;
		}

		//check numbers
		info = d[n].n_variables == n_diff;
		handle(0,info,"skipping divergence with different numbers of variables and differentials");

		//allocate the variable and differential arrays
		d[n].variable = (int *)malloc(d[n].n_variables * sizeof(int));
		d[n].differential = (int *)malloc(d[n].n_variables * sizeof(int));
		handle(1,d[n].variable != NULL && d[n].differential != NULL,"allocating divergence variables and differentials");

		//copy over
		for(j = 0; j < d[n].n_variables; j ++)
		{
			d[n].variable[j] = term[j];
			d[n].differential[j] = differential[j];
		}

		//increment the number of divergences
		n ++;
	}

	//resize
	d = divergences_new(d,n_fetch,n);
	handle(1,d != NULL,"re-allocating divergences");

	//check numbers
	fetch_destroy(fetch);
	fetch = fetch_new("",MAX_DIVERGENCES);
	handle(0,fetch_read(file,DIVERGENCE_LABEL,fetch) == n,"skipping divergences with unrecognised formats");

	//copy over
	*n_divergences = n;
	*divergence = d;

	//clean up
	fclose(file);
	fetch_destroy(fetch);
	free(piece);
	free(temp);
	free(term);
	free(differential);
}

////////////////////////////////////////////////////////////////////////////////
