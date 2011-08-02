////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

////////////////////////////////////////////////////////////////////////////////

int generate_connectivity(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	if(generate_face_borders(n_faces,face,n_cells,cell) != SUCCESS)
	{ printf("\nERROR - generate_connectivity - generating face borders"); return ERROR; }
}

////////////////////////////////////////////////////////////////////////////////

int generate_face_borders(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int i, j;

	//face borders
	for(i = 0; i < n_faces; i ++) face[i].n_borders = 0;
	for(i = 0; i < n_cells; i ++)
	{
		for(j = 0; j < cell[i].n_faces; j ++)
		{
			cell[i].face[j]->n_borders ++;
			cell[i].face[j]->border = (struct CELL **)realloc(cell[i].face[j]->border, cell[i].face[j]->n_borders * sizeof(struct CELL *));
			if(cell[i].face[j]->border == NULL) { printf("\nERROR - generate_face_borders - allocating border"); return ERROR; }
			cell[i].face[j]->border[cell[i].face[j]->n_borders-1] = &cell[i];
		}
	}

	/*for(i = 0; i < n_faces; i ++)
	{
		printf("[%i]",(int*)(&face[i] - &face[0]));
		for(j = 0; j < face[i].n_borders; j ++)
		{
			printf(" %i",(int*)(face[i].border[j] - &cell[0]));
		}
		printf("\n");
	}*/

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
