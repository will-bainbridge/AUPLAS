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
