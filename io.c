////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "differential.h"
#include "fetch.h"

#define MAX_INDICES 100

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

char * generate_timed_filename(char *filename, double time);

////////////////////////////////////////////////////////////////////////////////

void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell)
{
	FILE *file = fopen(filename,"r");
	exit_if_false(file != NULL,"opening geometry file");

	char *line;
	exit_if_false(allocate_character_vector(&line, MAX_STRING_LENGTH),"allocating line string");

	int i;

	while(fgets(line, MAX_STRING_LENGTH, file) != NULL)
	{
		if(strncmp(line,"NODES",5) == 0)
		{
			exit_if_false(sscanf(&line[6],"%i",n_nodes) == 1,"reading the number of nodes");
			*node = nodes_new(*n_nodes, NULL);
			exit_if_false(*node != NULL,"allocating the nodes");
			for(i = 0; i < *n_nodes; i ++) node_geometry_get(file, &(*node)[i]);
		}
		if(strncmp(line,"FACES",5) == 0)
		{
			exit_if_false(sscanf(&line[6],"%i",n_faces) == 1,"reading the number of faces");
			*face = faces_new(*n_faces, NULL);
			exit_if_false(*face != NULL,"allocating the faces");
			for(i = 0; i < *n_faces; i ++) face_geometry_get(file, &(*face)[i], *node);
		}
		if(strncmp(line,"CELLS",5) == 0)
		{
			exit_if_false(sscanf(&line[6],"%i",n_cells) == 1,"reading the number of cells");
			*cell = cells_new(*n_cells, NULL);
			exit_if_false(*cell != NULL,"allocating the cells");
			for(i = 0; i < *n_cells; i ++) cell_geometry_get(file, &(*cell)[i], *face);
		}
	}

	exit_if_false(*n_nodes > 0,"finding nodes in the geometry file");
	exit_if_false(*n_faces > 0,"finding faces in the geometry file");
	exit_if_false(*n_cells > 0,"finding cells in the geometry file");

	free(line);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void node_geometry_get(FILE *file, struct NODE *node)
{
	int info = fscanf(file,"%lf %lf\n",&(node->x[0]),&(node->x[1]));
	exit_if_false(info == 2, "reading a node's coordinates");
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
        exit_if_false(index != NULL && line != NULL && temp != NULL ,"allocating temporary storage");

        //read a line
        exit_if_false(fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a face line");

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
        exit_if_false(face_node_new(face),"allocating face nodes");

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
        exit_if_false(index != NULL && line != NULL && temp != NULL ,"allocating temporary storage");

        //read the line
        exit_if_false(fgets(line, MAX_STRING_LENGTH, file) != NULL, "reading a cell line");

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
        exit_if_false(cell_face_new(cell),"allocating cell faces");

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
	exit_if_false(file != NULL, "opening the case file");

	//number of variables
	exit_if_false(fwrite(&n_variables, sizeof(int), 1, file) == 1, "writing the number of variables");

	//numbers of elements
	exit_if_false(fwrite(&n_nodes, sizeof(int), 1, file) == 1, "writing the number of nodes");
	exit_if_false(fwrite(&n_faces, sizeof(int), 1, file) == 1, "writing the number of faces");
	exit_if_false(fwrite(&n_cells, sizeof(int), 1, file) == 1, "writing the number of cells");
	exit_if_false(fwrite(&n_zones, sizeof(int), 1, file) == 1, "writing the number of zones");

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
	exit_if_false(file != NULL, "opening the case file");

	//number of variables
	exit_if_false(fread(n_variables, sizeof(int), 1, file) == 1, "reading the number of variables");

	//numbers of elements
	exit_if_false(fread(n_nodes, sizeof(int), 1, file) == 1, "reading the number of nodes");
	exit_if_false((*node = nodes_new(*n_nodes,NULL)) != NULL,"allocating nodes");
	exit_if_false(fread(n_faces, sizeof(int), 1, file) == 1, "reading the number of faces");
	exit_if_false((*face = faces_new(*n_faces,NULL)) != NULL,"allocating faces");
	exit_if_false(fread(n_cells, sizeof(int), 1, file) == 1, "reading the number of cells");
	exit_if_false((*cell = cells_new(*n_cells,NULL)) != NULL,"allocating cells");
	exit_if_false(fread(n_zones, sizeof(int), 1, file) == 1, "reading the number of zones");
	exit_if_false((*zone = zones_new(*n_zones,NULL)) != NULL,"allocating zones");

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
	        exit_if_false(fwrite(node->x, sizeof(double), 2, file) == 2, "writing the node location");
}

////////////////////////////////////////////////////////////////////////////////

void node_case_get(FILE *file, struct NODE *node)
{
	        exit_if_false(fread(node->x, sizeof(double), 2, file) == 2, "reading the node location");
}

////////////////////////////////////////////////////////////////////////////////

void face_case_write(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	exit_if_false(index != NULL,"allocating temporary storage");

	exit_if_false(fwrite(&(face->n_nodes), sizeof(int), 1, file) == 1, "writing the number of face nodes");
	for(i = 0; i < face->n_nodes; i ++) index[i] = (int)(face->node[i] - &node[0]);
	exit_if_false(fwrite(index, sizeof(int), face->n_nodes, file) == face->n_nodes, "writing the face nodes");
	exit_if_false(fwrite(&(face->area), sizeof(double), 1, file) == 1, "writing the face area");
	exit_if_false(fwrite(face->centroid, sizeof(double), 2, file) == 2, "writing the face centroid");
	exit_if_false(fwrite(&(face->n_borders), sizeof(int), 1, file) == 1, "writing the number of face borders");
	for(i = 0; i < face->n_borders; i ++) index[i] = (int)(face->border[i] - &cell[0]);
	exit_if_false(fwrite(index, sizeof(int), face->n_borders, file) == face->n_borders, "writing the face borders");
	exit_if_false(fwrite(face->oriented, sizeof(int), face->n_borders, file) == face->n_borders, "writing the face orientations");
	exit_if_false(fwrite(&(face->n_zones), sizeof(int), 1, file) == 1, "writing the number of face zones");
	for(i = 0; i < face->n_zones; i ++) index[i] = (int)(face->zone[i] - &zone[0]);
	exit_if_false(fwrite(index, sizeof(int), face->n_zones, file) == face->n_zones, "writing the face zones");

	free(index);
}

////////////////////////////////////////////////////////////////////////////////

void face_case_get(FILE *file, struct NODE *node, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, *index;

	index = (int *)malloc(MAX_INDICES * sizeof(int));
	exit_if_false(index != NULL,"allocating temporary storage");

	exit_if_false(fread(&(face->n_nodes), sizeof(int), 1, file) == 1, "reading the number of face nodes");
	exit_if_false(face_node_new(face),"allocating face nodes");
	exit_if_false(fread(index, sizeof(int), face->n_nodes, file) == face->n_nodes, "reading face nodes");
	for(i = 0; i < face->n_nodes; i ++) face->node[i] = &node[index[i]];
	exit_if_false(fread(&(face->area), sizeof(double), 1, file) == 1, "reading the face area");
	exit_if_false(fread(face->centroid, sizeof(double), 2, file) == 2, "reading the face centroid");
	exit_if_false(fread(&(face->n_borders), sizeof(int), 1, file) == 1, "reading the number of face borders");
	exit_if_false(face_border_new(face),"allocating face borders");
	exit_if_false(fread(index, sizeof(int), face->n_borders, file) == face->n_borders, "reading the face borders");
	for(i = 0; i < face->n_borders; i ++) face->border[i] = &cell[index[i]];
	exit_if_false(face_oriented_new(face),"allocating face orientations");
	exit_if_false(fread(face->oriented, sizeof(int), face->n_borders, file) == face->n_borders, "reading the face orientations");
	exit_if_false(fread(&(face->n_zones), sizeof(int), 1, file) == 1, "reading the number of face zones");
	exit_if_false(face_zone_new(face),"allocating face zones");
	exit_if_false(fread(index, sizeof(int), face->n_zones, file) == face->n_zones, "reading the face zones");
	for(i = 0; i < face->n_zones; i ++) face->zone[i] = &zone[index[i]];

	free(index);
}

////////////////////////////////////////////////////////////////////////////////

void cell_case_write(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
        int i, n, *index;

        index = (int *)malloc(MAX_INDICES * sizeof(int));
        exit_if_false(index != NULL,"allocating temporary storage");

        exit_if_false(fwrite(&(cell->n_faces), sizeof(int), 1, file) == 1, "writing the number of cell faces");
        for(i = 0; i < cell->n_faces; i ++) index[i] = (int)(cell->face[i] - &face[0]);
        exit_if_false(fwrite(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell faces");
        exit_if_false(fwrite(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "writing the cell orientations");
        exit_if_false(fwrite(&(cell->area), sizeof(double), 1, file) == 1, "writing the cell area");
        exit_if_false(fwrite(cell->centroid, sizeof(double), 2, file) == 2, "writing the cell centroid");
        exit_if_false(fwrite(&(cell->n_zones), sizeof(int), 1, file) == 1, "writing the number of cell zones");
        for(i = 0; i < cell->n_zones; i ++) index[i] = (int)(cell->zone[i] - &zone[0]);
        exit_if_false(fwrite(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "writing the cell zones");
        exit_if_false(fwrite(cell->order, sizeof(int), n_variables, file) == n_variables, "writing the cell orders");
        exit_if_false(fwrite(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "writing the cell stencil sizes");
        for(i = 0; i < n_variables; i ++)
        {
                exit_if_false(fwrite(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"writing the cell stencil");
                n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
                exit_if_false(fwrite(cell->matrix[i][0], sizeof(double), n, file) == n,"writing the cell matrix");
        }

        free(index);
}

////////////////////////////////////////////////////////////////////////////////

void cell_case_get(FILE *file, int n_variables, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
        int i, n, *index;

        index = (int *)malloc(MAX_INDICES * sizeof(int));
        exit_if_false(index != NULL,"allocating temporary storage");

        exit_if_false(fread(&(cell->n_faces), sizeof(int), 1, file) == 1, "reading the number of cell faces");
        exit_if_false(cell_face_new(cell),"allocating cell faces");
        exit_if_false(fread(index, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading the cell faces");
        for(i = 0; i < cell->n_faces; i ++) cell->face[i] = &face[index[i]];
        exit_if_false(cell_oriented_new(cell),"allocating cell orientations");
        exit_if_false(fread(cell->oriented, sizeof(int), cell->n_faces, file) == cell->n_faces, "reading cell orientations");
        exit_if_false(fread(&(cell->area), sizeof(double), 1, file) == 1, "reading the cell area");
        exit_if_false(fread(cell->centroid, sizeof(double), 2, file) == 2, "reading the cell centroid");
        exit_if_false(fread(&(cell->n_zones), sizeof(int), 1, file) == 1, "reading the number of cell zones");
        exit_if_false(cell_zone_new(cell),"allocating cell zones");
        exit_if_false(fread(index, sizeof(int), cell->n_zones, file) == cell->n_zones, "reading the cell zones");
        for(i = 0; i < cell->n_zones; i ++) cell->zone[i] = &zone[index[i]];
        exit_if_false(cell_order_new(n_variables,cell),"allocating cell orders");
        exit_if_false(fread(cell->order, sizeof(int), n_variables, file) == n_variables, "reading the cell orders");
        exit_if_false(cell_n_stencil_new(n_variables,cell),"allocating cell stencil sizes");
        exit_if_false(fread(cell->n_stencil, sizeof(int), n_variables, file) == n_variables, "reading the cell stencil sizes");
        exit_if_false(cell_stencil_new(n_variables,cell),"allocating cell stencils");
        exit_if_false(cell_matrix_new(n_variables,cell),"allocating cell matrices");
        for(i = 0; i < n_variables; i ++)
        {
                exit_if_false(fread(cell->stencil[i], sizeof(int), cell->n_stencil[i], file) == cell->n_stencil[i],"reading the cell stencil");
                n = ORDER_TO_POWERS(cell->order[i]) * cell->n_stencil[i];
                exit_if_false(fread(cell->matrix[i][0], sizeof(double), n, file) == n,"reading the cell matrix");
        }

        free(index);
}

////////////////////////////////////////////////////////////////////////////////

void zone_case_write(FILE *file, struct ZONE *zone)
{
	exit_if_false(fwrite(&(zone->location), sizeof(char), 1, file) == 1,"writing the zone location");
	exit_if_false(fwrite(&(zone->variable), sizeof(int), 1, file) == 1,"writing the zone variable");
	exit_if_false(fwrite(zone->condition, sizeof(char), MAX_CONDITION_LENGTH, file) == MAX_CONDITION_LENGTH,"writing the zone condition");
	exit_if_false(fwrite(&(zone->value), sizeof(double), 1, file) == 1,"writing the zone value");
}

////////////////////////////////////////////////////////////////////////////////

void zone_case_get(FILE *file, struct ZONE *zone)
{
	exit_if_false(fread(&(zone->location), sizeof(char), 1, file) == 1,"reading the zone location");
	exit_if_false(fread(&(zone->variable), sizeof(int), 1, file) == 1,"reading the zone variable");
	exit_if_false(fread(zone->condition, sizeof(char), MAX_CONDITION_LENGTH, file) == MAX_CONDITION_LENGTH,"reading the zone condition");
	exit_if_false(fread(&(zone->value), sizeof(double), 1, file) == 1,"reading the zone value");
}

////////////////////////////////////////////////////////////////////////////////

void zones_input(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone)
{
	//counters
	int i, j, n = 0, info;

	//open the file
	FILE *file = fopen(filename,"r");
	exit_if_false(file != NULL,"opening input file");

	//fetch the data from the file
	FETCH fetch = fetch_new(ZONE_FORMAT, MAX_ZONES);
	exit_if_false(fetch != NULL,"allocating zone input");
	int n_fetch = fetch_read(file, ZONE_LABEL, fetch);
	exit_if_false(n_fetch > 1,"no zones found in input file");
	warn_if_false(n_fetch < MAX_ZONES,"maximum number of zones reached");

	//allocate zones
	struct ZONE *z = zones_new(n_fetch, NULL);
	exit_if_false(z != NULL,"allocating zones");

	//temporary storage
	int offset, index[2];
	char *range = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	exit_if_false(range != NULL && temp != NULL,"allocating temporary storage");

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
			warn_if_false(info,"skipping zone with unrecognised range");
			if(!info) break;

			//store zone in the elements in the range
			if(z[n].location == 'f') for(j = index[0]; j <= index[1]; j ++) exit_if_false(face_zone_add(&face[j],&z[n]),"adding a face zone");
			if(z[n].location == 'c') for(j = index[0]; j <= index[1]; j ++) exit_if_false(cell_zone_add(&cell[j],&z[n]),"adding a cell zone");

			//move to the next range in the string
			offset += strlen(temp) + 1;
		}

		//increment zone
		n += info;
	}

	//check numbers
	fetch_destroy(fetch);
	fetch = fetch_new("",MAX_ZONES);
	warn_if_false(fetch_read(file,ZONE_LABEL,fetch) == n,"skipping zones with unrecognised formats");

	//resize zone list
	struct ZONE *z_new = zones_new(n, z);
	exit_if_false(zone != NULL,"re-allocating zones");
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
	exit_if_false(file != NULL,"opening input file");

	//fetch the data
	FETCH fetch = fetch_new(DIVERGENCE_FORMAT,MAX_DIVERGENCES);
	exit_if_false(fetch != NULL,"allocating fetch");
	int n_fetch = fetch_read(file,DIVERGENCE_LABEL,fetch);
	exit_if_false(n_fetch > 1,"no divergences found in input file");
	warn_if_false(n_fetch < MAX_DIVERGENCES,"maximum number of divergences reached");

	//allocate pointers
	struct DIVERGENCE *d = divergences_new(n_fetch,NULL);
	exit_if_false(d != NULL,"allocating divergences");

	//counters
	int i, j, n = 0, info;

	//temporary storage
	char direction;
	int offset, n_diff, diff[2];
	char *piece = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	int *term = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	int *differential = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	exit_if_false(piece != NULL && temp != NULL && term != NULL && differential != NULL,"allocating temporary storage");

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
			warn_if_false(info = 0,"skipping divergence with unrecognised direction");
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
			warn_if_false(info,"skipping divergence with unrecognised variable format");
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
			warn_if_false(info,"skipping divergence with unrecognised differentail format");
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
		warn_if_false(info,"skipping divergence with different numbers of variables and differentials");

		//allocate the variable and differential arrays
		d[n].variable = (int *)malloc(d[n].n_variables * sizeof(int)); //?? SHOULD BE MEMORY FUNCTIONS DOING THIS
		d[n].differential = (int *)malloc(d[n].n_variables * sizeof(int));
		exit_if_false(d[n].variable != NULL && d[n].differential != NULL,"allocating divergence variables and differentials");

		//copy over
		for(j = 0; j < d[n].n_variables; j ++)
		{
			d[n].variable[j] = term[j];
			d[n].differential[j] = differential[j];
		}

		//increment the number of divergences
		n ++;
	}

	//check numbers
	fetch_destroy(fetch);
	fetch = fetch_new("",MAX_DIVERGENCES);
	warn_if_false(fetch_read(file,DIVERGENCE_LABEL,fetch) == n,"skipping divergences with unrecognised formats");

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

char * generate_timed_filename(char *filename, double time)
{
	char *sub = strchr(filename, '?');
	exit_if_false(sub != NULL,"substitute character \"?\" not found in data filename");

	char *timed_filename;
	exit_if_false(allocate_character_vector(&timed_filename, MAX_STRING_LENGTH),"allocating timed filename");

	union { int integer; float real; } number;
	number.real = time;

	*sub = '\0';
	sprintf(timed_filename, "%s%i%s", filename, number.integer, sub + 1);
	*sub = '?';

	return timed_filename;
}

////////////////////////////////////////////////////////////////////////////////

void write_data(char *filename, double time, int n_data, double *data)
{
	char *timed_filename = generate_timed_filename(filename, time);

	FILE *file = fopen(timed_filename,"w");
	exit_if_false(file != NULL,"opening data file");

	exit_if_false(fwrite(&time, sizeof(double), 1, file) == 1,"writing the time");
	exit_if_false(fwrite(data, sizeof(double), n_data, file) == n_data,"writing the data");

	free_vector(timed_filename);
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void read_data(char *filename, double *time, int n_data, double *data)
{
	FILE *file = fopen(filename,"r");
	exit_if_false(file != NULL,"opening data file");

	exit_if_false(fread(time, sizeof(double), 1, file) == 1,"reading the time");
	exit_if_false(fread(data, sizeof(double), n_data, file) == n_data,"reading the data");

	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////

void write_gnuplot(char *filename, double time, int n_variables, int *id_to_unknown, double *x, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
        int n_polygon;
        double ***polygon;
        exit_if_false(allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2),"allocating polygon memory");

	char *timed_filename = generate_timed_filename(filename, time);

        char *temp;
        exit_if_false(allocate_character_vector(&temp, MAX_STRING_LENGTH),"allocating temporary string");

        double *data;
        exit_if_false(allocate_double_vector(&data,n_variables),"allocating data");
        int *is_data;
        exit_if_false(allocate_integer_vector(&is_data,n_variables),"allocating no data indicator");

        FILE *file = fopen(timed_filename,"w");
        exit_if_false(file != NULL,"opening file");

        int e, i, j, u, v;

        int nz;
        struct ZONE **z;

        for(e = 0; e < n_faces + n_cells; e ++)
        {
                i = e % n_faces;

                n_polygon = generate_control_volume_polygon(polygon, i, (e < n_faces ? 'f' : 'c'), face, cell);

                for(v = 0; v < n_variables; v ++) is_data[v] = 0;

                if(e < n_faces)
                {
                        nz = face[i].n_zones;
                        z = face[i].zone;
                }
                else
                {
                        nz = cell[i].n_zones;
                        z = cell[i].zone;
                }

                for(j = 0; j < nz; j ++)
                {
                        u = id_to_unknown[INDEX_AND_ZONE_TO_ID(i,z[j] - &zone[0])];
                        v = z[j]->variable;
                        if(u >= 0) { data[v] = x[u]; is_data[v] = 1; }
                }

                temp[0] = '\0';
                for(v = 0; v < n_variables; v ++)
                {
                        if(is_data[v]) sprintf(&temp[strlen(temp)]," %+.10e",data[v]);
                        else           sprintf(&temp[strlen(temp)]," NaN");
                }

                for(j = 1; j < n_polygon - 1; j ++)
                {
                        fprintf(file,"%+.10e %+.10e%s\n",polygon[0][0][0],polygon[0][0][1],temp);
                        fprintf(file,"%+.10e %+.10e%s\n\n",polygon[j][0][0],polygon[j][0][1],temp);
                        fprintf(file,"%+.10e %+.10e%s\n",polygon[0][0][0],polygon[0][0][1],temp);
                        fprintf(file,"%+.10e %+.10e%s\n\n\n",polygon[j][1][0],polygon[j][1][1],temp);
                }
        }

        fclose(file);

        free_matrix((void **)polygon);
        free_vector(timed_filename);
	free_vector(temp);
	free_vector(data);
	free_vector(is_data);
}

////////////////////////////////////////////////////////////////////////////////

/*void write_vtk(char *filename, double time, int n_variables, char **variable_name, int n_ids, int *id_to_unknown, double *x, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	char *timed_filename = generate_timed_filename(filename, time);

	FILE *file = fopen(timed_filename,"w");
	exit_if_false(file != NULL,"opening file");

	int *point_used, *point_index, *id_used;
	exit_if_false(allocate_integer_vector(&point_used,n_nodes + n_cells),"allocating point usage array");
	exit_if_false(allocate_integer_vector(&point_index,n_nodes + n_cells),"allocating point index array");
	exit_if_false(allocate_integer_vector(&id_used,n_ids),"allocating id usage array");

	int n_points, n_elements;

	fprintf(file,"<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n<UnstructuredGrid>\n");

	int i, j, k, v, z, index, offset;

	for(v = 0; v < n_variables; v ++)
	{
		for(i = 0; i < n_nodes + n_cells; i ++) point_used[i] = 0;
		for(i = 0; i < n_ids; i ++) id_used[i] = 0;

		for(z = 0; z < n_zones; z ++)
		{
			if(zone[z].variable != v || zone[z].condition[0] != 'u') continue;

			if(zone[z].location == 'f')
			{
				for(i = 0; i < n_faces; i ++)
				{
					for(j = 0; j < face[i].n_zones; j ++)
					{
						if(z == (int)(face[i].zone[j] - &zone[0]))
						{
							id_used[INDEX_AND_ZONE_TO_ID(i,z)] = 1;
							point_used[(int)(face[i].node[0] - &node[0])] = 1;
							point_used[(int)(face[i].node[face[i].n_nodes - 1] - &node[0])] = 1;
							for(k = 0; k < face[i].n_borders; k ++) point_used[n_nodes + (int)(face[i].border[k] - &cell[0])] = 1;
						}
					}
				}
			}
			else if(zone[z].location == 'c')
			{
				for(i = 0; i < n_cells; i ++)
				{
					for(j = 0; j < cell[i].n_zones; j ++)
					{
						if(z == (int)(cell[i].zone[j] - &zone[0]))
						{
							id_used[INDEX_AND_ZONE_TO_ID(i,z)] = 1;
							for(k = 0; k < cell[i].n_faces; k ++)
							{
								point_used[(int)(cell[i].face[k]->node[0] - &node[0])] = 1;
								point_used[(int)(face[i].node[cell[i].face[k]->n_nodes - 1] - &node[0])] = 1;
							}
						}
					}
				}
			}
			else exit_if_false(0,"recognising the location");
		}

		n_points = 0;
		for(i = 0; i < n_nodes + n_cells; i ++) if(point_used[i]) point_index[n_points ++] = i;
		n_elements = 0;
		for(i = 0; i < n_ids; i ++) if(id_used[i]) n_elements ++;

		fprintf(file,"<Piece NumberOfPoints=\"%i\" NumberOfCells=\"%i\">\n", n_points, n_elements);

		fprintf(file,"<CellData>\n");

		fprintf(file,"<DataArray Name=\"%s\" type=\"Float64\" format=\"ascii\">\n",variable_name[v]);
		for(i = 0; i < n_ids; i ++) if(id_used[i]) fprintf(file," %lf",x[id_to_unknown[i]]);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</CellData>\n");

		fprintf(file,"<Points>\n");

		fprintf(file,"<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n");
		for(i = 0; i < n_nodes; i ++) if(point_used[i]) fprintf(file," %lf %lf %lf",node[i].x[0],node[i].x[1],0.0);
		for(i = 0; i < n_cells; i ++) if(point_used[n_nodes + i]) fprintf(file," %lf %lf %lf",cell[i].centroid[0],cell[i].centroid[1],0.0);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</Points>\n");

		fprintf(file,"<Cells>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n");
		for(i = 0; i < n_ids; i ++)
		{
			if(id_used[i])
			{
				index = ID_TO_INDEX(i);
				z = ID_TO_ZONE(i);

				if(zone[z].location == 'f')
				{
					fprintf(file," %i",point_index[(int)((face[index].oriented[0] ? face[index].node[1] : face[index].node[0]) - &node[0])]);
					fprintf(file," %i",point_index[n_nodes + (int)(face[index].border[0] - &cell[0])]);
					fprintf(file," %i",point_index[(int)((face[index].oriented[0] ? face[index].node[0] : face[index].node[1]) - &node[0])]);
					if(face[index].n_borders == 2) fprintf(file," %i",point_index[n_nodes + (int)(face[index].border[1] - &cell[0])]);
				}
				else if(zone[z].location == 'c')
				{
					for(j = 0; j < cell[index].n_faces; j ++)
					{
						fprintf(file," %i",point_index[(int)(cell[index].face[j]->node[!cell[index].oriented[j]] - &node[0])]);
					}
				}
				else exit_if_false(0,"recognising the location");
			}
		}
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n");
		offset = 0;
		for(i = 0; i < n_ids; i ++)
		{
			if(id_used[i])
			{
				index = ID_TO_INDEX(i);
				z = ID_TO_ZONE(i);

				if(zone[z].location == 'f')
				{
					offset += 2 + face[index].n_borders;
				}
				else if(zone[z].location == 'c')
				{
					offset += cell[index].n_faces;
				}
				else exit_if_false(0,"recognising the location");

				fprintf(file," %i",offset);
			}
		}
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">\n");
		for(i = 0; i < n_elements; i ++) fprintf(file," %i",7);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</Cells>\n");

		fprintf(file,"</Piece>\n");
	}

	fprintf(file,"</UnstructuredGrid>\n</VTKFile>");

	fclose(file);

	free_vector(timed_filename);
	free_vector(point_used);
	free_vector(point_index);
	free_vector(id_used);
}*/

////////////////////////////////////////////////////////////////////////////////
