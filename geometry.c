////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

#include "quadrature.h"

////////////////////////////////////////////////////////////////////////////////

int generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int c, i, i1, j, k;
	struct NODE *v0, *v1, *vr;
	struct FACE *f0, *f1;
	double cross;

	for(c = 0; c < n_cells; c ++)
	{
		if(allocate_integer_vector(&(cell[c].oriented),cell[c].n_faces) != ALLOCATE_SUCCESS)
		{ printf("\nERROR - generate_orientations - allocating cell oriented memory"); return ERROR; }

		for(i = 0; i < cell[c].n_faces; i ++)
		{
			i1 = i + 1 - cell[c].n_faces*(i == cell[c].n_faces - 1);

			f0 = cell[c].face[i];
			f1 = cell[c].face[i1];

			v0 = f0->node[0];
			v1 = f0->node[1];

			if(f1->node[0] != v0 && f1->node[0] != v1)
				vr = f1->node[0];
			else
				vr = f1->node[1];

			//assumes the cell is convex
			cross = (v0->x[0] - vr->x[0])*(v1->x[1] - vr->x[1]) - (v1->x[0] - vr->x[0])*(v0->x[1] - vr->x[1]);

			cell[c].oriented[i] = (cross > 0);


		}
	}

	//copy the borders generated above into the face structures
	for(i = 0; i < n_faces; i ++)
	{
		if(allocate_integer_vector(&(face[i].oriented),face[i].n_borders) != ALLOCATE_SUCCESS)
		{ printf("\nERROR - generate_orientations - allocating face oriented memory"); return ERROR; }

		for(j = 0; j < face[i].n_borders; j ++)
		{
			for(k = 0; k < face[i].border[j]->n_faces; k ++)
			{
				if(face[i].border[j]->face[k] == &face[i])
				{
					face[i].oriented[j] = face[i].border[j]->oriented[k];
					break;
				}
			}

			if(k == face[i].border[j]->n_faces)
			{ printf("\nERROR - generate_orientations - finding corresponding face"); return ERROR; }
		}
	}

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int i;
	double **polygon;

	if(allocate_double_matrix(&polygon,MAX(MAX_CELL_FACES,4),2) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - calculate_control_volume_geometry - allocating polygon memory"); return ERROR; }

	for(i = 0; i < n_cells; i ++)
	{
		generate_control_volume_polygon(polygon, i, 'c', face, cell);
		calculate_polygon_centroid(cell[i].n_faces, polygon, cell[i].centroid);
	}
	for(i = 0; i < n_faces; i ++)
	{
		generate_control_volume_polygon(polygon, i, 'f', face, cell);
		calculate_polygon_centroid(2 + face[i].n_borders, polygon, face[i].centroid);
	}

	free_matrix((void**)polygon);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void generate_control_volume_polygon(double **polygon, int index, int location, struct FACE *face, struct CELL *cell)
{
	if(location == 'f')
	{
		double temp;

		polygon[0][0] = face[index].node[0]->x[0];
		polygon[0][1] = face[index].node[0]->x[1];

		polygon[1][0] = face[index].border[0]->centroid[0];
		polygon[1][1] = face[index].border[0]->centroid[1];

		polygon[2][0] = face[index].node[1]->x[0];
		polygon[2][1] = face[index].node[1]->x[1];

		if(face[index].n_borders == 2) {
			polygon[3][0] = face[index].border[1]->centroid[0];
			polygon[3][1] = face[index].border[1]->centroid[1];
		}

		if(face[index].oriented[0])
		{
			temp = polygon[0][0]; polygon[0][0] = polygon[2][0]; polygon[2][0] = temp;
			temp = polygon[0][1]; polygon[0][1] = polygon[2][1]; polygon[2][1] = temp;
		}
	}
	if(location == 'c')
	{
		int i, n;

		for(i = 0; i < cell[index].n_faces; i ++)
		{
			n = !cell[index].oriented[i];
			polygon[i][0] = cell[index].face[i]->node[n]->x[0];
			polygon[i][1] = cell[index].face[i]->node[n]->x[1];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void calculate_polygon_centroid(int n, double **polygon, double *centroid)
{
	double x[2], nx[2], area;
	int i, i1, j;

	area = 0;
	centroid[0] = 0;
	centroid[1] = 0;

	for(i = 0; i < n; i ++)
	{
		i1 = i + 1 - n*(i == n - 1);

		nx[0] = polygon[i1][1] - polygon[i][1];
		nx[1] = polygon[i][0] - polygon[i1][0];

		for(j = 0; j < 2; j ++)
		{
			x[0] = 0.5*polygon[i][0]*(1.0 - gauss_x[1][j]) + 0.5*polygon[i1][0]*(1.0 + gauss_x[1][j]);
			x[1] = 0.5*polygon[i][1]*(1.0 - gauss_x[1][j]) + 0.5*polygon[i1][1]*(1.0 + gauss_x[1][j]);

			area += x[0]*nx[0]*gauss_w[1][j]*0.5;

			centroid[0] += x[0]*x[1]*nx[1]*gauss_w[1][j]*0.5;
			centroid[1] += x[0]*x[1]*nx[0]*gauss_w[1][j]*0.5;
		}
	}

	centroid[0] /= area;
	centroid[1] /= area;
}

////////////////////////////////////////////////////////////////////////////////

/*int order_cells(int n_cells, struct CELL *cell)
{
	int c, f, n, f0, n0, f1, n1;
	struct FACE *temp;
	double area;

	for(c = 0; c < n_cells; c ++)
	{

		printf("(%lf %lf)",cell[c].face[0]->node[1]->x[0],cell[c].face[0]->node[1]->x[1]);

		//arbitrary starting node of first face
		f1 = 0;
		n1 = 0;
		area = cell[c].face[f1]->node[!n1]->x[0]*cell[c].face[f1]->node[n1]->x[1] - cell[c].face[f1]->node[n1]->x[0]*cell[c].face[f1]->node[!n1]->x[1];


		//loop over all the faces
		for(f0 = 0; f0 < cell[c].n_faces - 1; f0 ++)
		{
			//update matching node
			n0 = n1;
			n1 = -1;

			//next face
			f = f1 = f0 + 1;

			//loop over all remaining faces
			while(n1 < 0 && f < cell[c].n_faces)
			{
				//loop over each node of the remaining face
				n = 0;
				while(n1 < 0 && n < 2)
				{
					//if the nodes match
					if(cell[c].face[f0]->node[n0] == cell[c].face[f]->node[n])
					{
						//new node is the next one on the face
						n1 = !n;

						//put the matching face next in line
						temp = cell[c].face[f];
						cell[c].face[f] = cell[c].face[f1];
						cell[c].face[f1] = temp;
					}

					n ++;
				}

				f ++;
			}

			//check a match was found
			if(n1 < 0) { printf("\nERROR - order_cells - next face not found"); return ERROR; }

			printf(" -> (%lf %lf)",cell[c].face[f1]->node[n1]->x[0],cell[c].face[f1]->node[n1]->x[1]);
			


			area += cell[c].face[f1]->node[!n1]->x[0]*cell[c].face[f1]->node[n1]->x[1] - cell[c].face[f1]->node[n1]->x[0]*cell[c].face[f1]->node[!n1]->x[1];

		}


		printf("\n%lf",area);
		getchar();

	}

	return SUCCESS;
}*/

////////////////////////////////////////////////////////////////////////////////
