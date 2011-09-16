////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "differential.h"
#include "fetch.h"

#define MAX_INDICES 100

#define ZONE_LABEL "zone"
#define ZONE_FORMAT "csisd"

#define DIVERGENCE_LABEL "divergence"
#define DIVERGENCE_FORMAT "icsssds"
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

void generate_timed_filename(char *filename, char *basename, double time);
void generate_timed_named_filename(char *filename, char *basename, double time, char *name);

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

void divergences_input(char *filename, char *constant, int *n_divergences, struct DIVERGENCE **divergence)
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
	int var_offset, dif_offset, pow_offset, dif[2];
	char *var_string = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *dif_string = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *pow_string = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	exit_if_false(var_string != NULL && dif_string != NULL && pow_string != NULL && temp != NULL,"allocating temporary strings");
	int *vars = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	int *difs = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	int *pows = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	exit_if_false(vars != NULL && difs != NULL && pows != NULL,"allocating temporary data");

	for(i = 0; i < n_fetch; i ++)
	{
		//equation
		fetch_get(fetch, i, 0, &d[n].equation);

		//direction
		fetch_get(fetch, i, 1, &direction);
		if(direction == 'x') {
			d[n].direction = 0;
		} else if(direction == 'y') {
			d[n].direction = 1;
		} else {
			warn_if_false(info = 0,"skipping divergence with unrecognised direction");
			continue;
		}

		//get the variable, differential and power strings
		fetch_get(fetch, i, 2, var_string);
		fetch_get(fetch, i, 3, dif_string);
		fetch_get(fetch, i, 4, pow_string);
		for(j = 0; j < strlen(var_string); j ++) if(var_string[j] == ',') var_string[j] = ' ';
		for(j = 0; j < strlen(dif_string); j ++) if(dif_string[j] == ',') dif_string[j] = ' ';
		for(j = 0; j < strlen(pow_string); j ++) if(pow_string[j] == ',') pow_string[j] = ' ';

		//read each variable in turn
		var_offset = dif_offset = pow_offset = d[n].n_variables = 0;
		while(var_offset < strlen(var_string))
		{
			info = 1;

			//read the variable index from the string
			info *= sscanf(&var_string[var_offset],"%s",temp) == 1;
			info *= sscanf(temp,"%i",&vars[d[n].n_variables]) == 1;
			var_offset += strlen(temp) + 1;

			//read the x and y differentials and convert to a differential index
			info *= sscanf(&dif_string[dif_offset],"%s",temp) == 1;
			j = dif[0] = dif[1] = 0;
			if(info)
			{
				while(temp[j] != '\0')
				{
					dif[0] += (temp[j] == 'x');
					dif[1] += (temp[j] == 'y');
					j ++;
				}
				difs[d[n].n_variables] = differential_index[dif[0]][dif[1]];
			}
			dif_offset += strlen(temp) + 1;

			//read the variable powers from the string
			info *= sscanf(&pow_string[pow_offset],"%s",temp) == 1;
			info *= sscanf(temp,"%i",&pows[d[n].n_variables]) == 1;
			pow_offset += strlen(temp) + 1;

			warn_if_false(info,"skipping divergence with unrecognised variable format");
			if(!info) continue;

			//next variable
			d[n].n_variables ++;
		}

		//allocate the variable and differential arrays
		d[n].variable = (int *)malloc(d[n].n_variables * sizeof(int)); //?? SHOULD BE MEMORY FUNCTIONS DOING THIS
		d[n].differential = (int *)malloc(d[n].n_variables * sizeof(int));
		d[n].power = (int *)malloc(d[n].n_variables * sizeof(int));
		exit_if_false(d[n].variable != NULL && d[n].differential != NULL && d[n].power != NULL,"allocating divergence variables and differentials");

		//copy over
		for(j = 0; j < d[n].n_variables; j ++)
		{
			d[n].variable[j] = vars[j];
			d[n].differential[j] = difs[j];
			d[n].power[j] = pows[j];
		}

		//implicit fraction
		fetch_get(fetch, i, 5, &d[n].implicit);

		//constant expression
		strcpy(temp, constant);
		j = strlen(temp);
		fetch_get(fetch, i, 6, &temp[j]);
		d[n].constant = expression_generate(temp);
		warn_if_false(d[n].constant != NULL,"skipping divergence for which the expression generation failed");
		if(d[n].constant == NULL) continue;

		//printf("divergence %i expression -> ",n); expression_print(d[n].constant); printf("\n");

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
	free(var_string);
	free(dif_string);
	free(pow_string);
	free(temp);
	free(vars);
	free(difs);
	free(pows);
}

////////////////////////////////////////////////////////////////////////////////

void generate_timed_filename(char *filename, char *basename, double time)
{
	char *sub = strchr(basename, '?');
	exit_if_false(sub != NULL,"substitute character \"?\" not found in data basename");

	union { int integer; float real; } number;
	number.real = time;

	*sub = '\0';
	sprintf(filename, "%s%i%s", basename, number.integer, sub + 1);
	*sub = '?';
}

////////////////////////////////////////////////////////////////////////////////

void generate_timed_named_filename(char *filename, char *basename, double time, char *name)
{
	char *sub[2];
	sub[0] = strchr(basename, '?');
	exit_if_false(sub[0] != NULL,"first substitute character \"?\" not found in data basename");
	sub[1] = strchr(sub[0] + 1, '?');
	exit_if_false(sub[1] != NULL,"second substitute character \"?\" not found in data basename");

	union { int integer; float real; } number;
	number.real = time;

	*sub[0] = *sub[1] = '\0';
	sprintf(filename, "%s%i%s%s%s", basename, number.integer, sub[0] + 1, name, sub[1] + 1);
	*sub[0] = *sub[1] = '?';
}

////////////////////////////////////////////////////////////////////////////////

void write_data(char *basename, double time, int n_data, double *data)
{
	char *filename;
	exit_if_false(allocate_character_vector(&filename, MAX_STRING_LENGTH),"allocating filename");
	generate_timed_filename(filename, basename, time);

	FILE *file = fopen(filename,"w");
	exit_if_false(file != NULL,"opening data file");

	exit_if_false(fwrite(&time, sizeof(double), 1, file) == 1,"writing the time");
	exit_if_false(fwrite(data, sizeof(double), n_data, file) == n_data,"writing the data");

	free_vector(filename);
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

void write_gnuplot(char *basename, double time, int n_variables, char **variable_name, int n_unknowns, int *unknown_to_id, double *x, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
        int i, j, u, v, z;

        int n_polygon;
        double ***polygon;
        exit_if_false(allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2),"allocating polygon memory");

	char *filename;
	exit_if_false(allocate_character_vector(&filename, MAX_STRING_LENGTH),"allocating filename");

	FILE **file = (FILE **)malloc(n_variables * sizeof(FILE *));
	exit_if_false(file != NULL,"allocating files");

	for(v = 0; v < n_variables; v ++)
	{
		generate_timed_named_filename(filename, basename, time, variable_name[v]);

		file[v] = fopen(filename,"w");
		exit_if_false(file[v] != NULL,"opening file");
	}

	for(u = 0; u < n_unknowns; u ++)
	{
		i = ID_TO_INDEX(unknown_to_id[u]);
		z = ID_TO_ZONE(unknown_to_id[u]);

		n_polygon = generate_control_volume_polygon(polygon, i, zone[z].location, face, cell);

		v = zone[z].variable;

		for(j = 1; j < n_polygon; j ++)
		{
			fprintf(file[v],"%+.10e %+.10e %+.10e\n",polygon[0][0][0],polygon[0][0][1],x[u]);
			fprintf(file[v],"%+.10e %+.10e %+.10e\n\n",polygon[j][0][0],polygon[j][0][1],x[u]);
			fprintf(file[v],"%+.10e %+.10e %+.10e\n",polygon[0][0][0],polygon[0][0][1],x[u]);
			fprintf(file[v],"%+.10e %+.10e %+.10e\n\n\n",polygon[j][1][0],polygon[j][1][1],x[u]);
		}
	}

        for(v = 0; v < n_variables; v ++) fclose(file[v]);

        free_matrix((void **)polygon);
        free_vector(filename);
	free(file);
}

////////////////////////////////////////////////////////////////////////////////

void write_vtk(char *basename, double time, int n_variables, char **variable_name, int n_unknowns, int *unknown_to_id, double *x, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	char *filename;
	exit_if_false(allocate_character_vector(&filename, MAX_STRING_LENGTH),"allocating filename");

	FILE *file;

	int *point_used, *point_index;
	exit_if_false(allocate_integer_vector(&point_used,n_nodes + n_cells),"allocating point usage array");
	exit_if_false(allocate_integer_vector(&point_index,n_nodes + n_cells),"allocating point index array");

	int n_points, n_elements;

	int i, j, u, v, z, offset;

	for(v = 0; v < n_variables; v ++)
	{
		generate_timed_named_filename(filename, basename, time, variable_name[v]);

		file = fopen(filename,"w");
		exit_if_false(file != NULL,"opening file");

		fprintf(file,"<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n<UnstructuredGrid>\n");

		for(i = 0; i < n_nodes + n_cells; i ++) point_used[i] = 0;

		n_elements = 0;

		for(u = 0; u < n_unknowns; u ++)
		{
			i = ID_TO_INDEX(unknown_to_id[u]);
			z = ID_TO_ZONE(unknown_to_id[u]);

			if(zone[z].variable == v)
			{
				n_elements ++;

				if(zone[z].location == 'f')
				{
					point_used[(int)(face[i].node[0] - &node[0])] = 1;
					point_used[(int)(face[i].node[face[i].n_nodes - 1] - &node[0])] = 1;
					for(j = 0; j < face[i].n_borders; j ++) point_used[n_nodes + (int)(face[i].border[j] - &cell[0])] = 1;
				}
				else if(zone[z].location == 'c')
				{
					for(j = 0; j < cell[i].n_faces; j ++)
					{
						point_used[(int)(cell[i].face[j]->node[0] - &node[0])] = 1;
						point_used[(int)(face[i].node[cell[i].face[j]->n_nodes - 1] - &node[0])] = 1;
					}
				}
				else exit_if_false(0,"recognising the location");
			}
		}

		n_points = 0;
		for(i = 0; i < n_nodes + n_cells; i ++) if(point_used[i]) point_index[n_points ++] = i;

		fprintf(file,"<Piece NumberOfPoints=\"%i\" NumberOfCells=\"%i\">\n", n_points, n_elements);

		fprintf(file,"<CellData>\n");

		fprintf(file,"<DataArray Name=\"%s\" type=\"Float64\" format=\"ascii\">\n",variable_name[v]);
		for(u = 0; u < n_unknowns; u ++) if(zone[ID_TO_ZONE(unknown_to_id[u])].variable == v) fprintf(file," %+.10e",x[u]);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</CellData>\n");

		fprintf(file,"<Points>\n");

		fprintf(file,"<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n");
		for(i = 0; i < n_nodes; i ++) if(point_used[i]) fprintf(file," %+.10e %+.10e %+.10e",node[i].x[0],node[i].x[1],0.0);
		for(i = 0; i < n_cells; i ++) if(point_used[n_nodes + i]) fprintf(file," %+.10e %+.10e %+.10e",cell[i].centroid[0],cell[i].centroid[1],0.0);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</Points>\n");

		fprintf(file,"<Cells>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n");
		for(u = 0; u < n_unknowns; u ++)
		{
			i = ID_TO_INDEX(unknown_to_id[u]);
			z = ID_TO_ZONE(unknown_to_id[u]);

			if(zone[z].variable == v)
			{
				if(zone[z].location == 'f')
				{
					fprintf(file," %i",point_index[(int)((face[i].oriented[0] ? face[i].node[1] : face[i].node[0]) - &node[0])]);
					fprintf(file," %i",point_index[n_nodes + (int)(face[i].border[0] - &cell[0])]);
					fprintf(file," %i",point_index[(int)((face[i].oriented[0] ? face[i].node[0] : face[i].node[1]) - &node[0])]);
					if(face[i].n_borders == 2) fprintf(file," %i",point_index[n_nodes + (int)(face[i].border[1] - &cell[0])]);
				}
				else if(zone[z].location == 'c')
				{
					for(j = 0; j < cell[i].n_faces; j ++)
					{
						fprintf(file," %i",point_index[(int)(cell[i].face[j]->node[!cell[i].oriented[j]] - &node[0])]);
					}
				}
				else exit_if_false(0,"recognising the location");
			}
		}
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n");
		offset = 0;
		for(u = 0; u < n_unknowns; u ++)
		{
			i = ID_TO_INDEX(unknown_to_id[u]);
			z = ID_TO_ZONE(unknown_to_id[u]);

			if(zone[z].variable == v)
			{
				if(zone[z].location == 'f')
				{
					offset += 2 + face[i].n_borders;
				}
				else if(zone[z].location == 'c')
				{
					offset += cell[i].n_faces;
				}
				else exit_if_false(0,"recognising the location");

				fprintf(file," %i",offset);
			}
		}
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"<DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">\n");
		for(i = 0; i < n_elements; i ++) fprintf(file," %i",7);
		fprintf(file,"\n</DataArray>\n");

		fprintf(file,"</Cells>");

		fprintf(file,"\n</Piece>\n</UnstructuredGrid>\n</VTKFile>");

		fclose(file);
	}

	free_vector(filename);
	free_vector(point_used);
	free_vector(point_index);
}

////////////////////////////////////////////////////////////////////////////////
