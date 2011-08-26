////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"

////////////////////////////////////////////////////////////////////////////////

struct FACE * faces_new(int n_faces, struct FACE *face)
{
	face = (struct FACE *)realloc(face, n_faces * sizeof(struct FACE));
	if(face == NULL) return NULL;

	int i;
	for(i = 0; i < n_faces; i ++)
	{
		face[i].n_nodes = 0;
		face[i].node = NULL;
		face[i].n_borders = 0;
		face[i].border = NULL;
		face[i].oriented = NULL;
		face[i].n_zones = 0;
		face[i].zone = NULL;
	}

	return face;
}

////////////////////////////////////////////////////////////////////////////////

int face_node_new(struct FACE *face)
{
	face->node = (struct NODE **)realloc(face->node, face->n_nodes * sizeof(struct NODE *));
	if(face->node == NULL) return 0;

	return 1;
}

int face_border_new(struct FACE *face)
{
	face->border = (struct CELL **)realloc(face->border, face->n_borders * sizeof(struct CELL *));
	if(face->border == NULL) return 0;

	return 1;
}

int face_oriented_new(struct FACE *face)
{
	face->oriented = (int *)realloc(face->oriented, face->n_borders * sizeof(int));
	if(face->oriented == NULL) return 0;

	return 1;
}

int face_zone_new(struct FACE *face)
{
	face->zone = (struct ZONE **)realloc(face->zone, face->n_zones * sizeof(struct ZONE *));
	if(face->zone == NULL) return 0;

	return 1;
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

void face_generate_border(struct FACE *face, struct CELL *cell)
{
	int i;
	for(i = 0; i < face->n_borders; i ++) if(face->border[i] == cell) return;

	face->n_borders ++;
	handle(1,face_border_new(face),"allocating face border");

	face->border[face->n_borders - 1] = cell;

	for(i = 0; i < face->n_nodes; i ++) node_generate_border(face->node[i],cell);
}

struct CELL ** face_add_node_borders_to_list(struct FACE *face, int *n_list, struct CELL **list)
{
	int i;
	for(i = 0; i < face->n_nodes; i ++) list = node_add_borders_to_list(face->node[i], n_list, list);
	return list;
}

struct CELL ** face_add_face_borders_to_list(struct FACE *face, int *n_list, struct CELL **list)
{
	int i, j, n_add = 0;

	int *add = (int *)malloc(face->n_borders * sizeof(int));
	handle(1,add != NULL,"allocating temporary storage");

	for(i = 0; i < face->n_borders; i ++)
	{
		add[i] = 1;

		for(j = 0; j < *n_list; j ++)
		{
			if(face->border[i] == list[j])
			{
				add[i] = 0;
				break;
			}
		}

		n_add += add[i];
	}

	if(n_add > 0)
	{
		list = (struct CELL **)realloc(list, (*n_list + n_add) * sizeof(struct CELL *));
		handle(1,list != NULL,"re-allocating list");

		for(i = 0; i < face->n_borders; i ++)
		{
			if(add[i])
			{
				list[(*n_list) ++] = face->border[i];
			}
		}
	}

	free(add);

	return list;
}

////////////////////////////////////////////////////////////////////////////////

void faces_destroy(int n_faces, struct FACE *face)
{
	int i;
	for(i = 0; i < n_faces; i ++) 
	{
		free(face[i].node);
		free(face[i].border);
		free(face[i].oriented);
		free(face[i].zone);
	}
	free(face);
}

////////////////////////////////////////////////////////////////////////////////
