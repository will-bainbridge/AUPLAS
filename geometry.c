////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "quadrature.h"

////////////////////////////////////////////////////////////////////////////////

void generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int c, i, i1, j, k;
	struct NODE *v0, *v1, *vr;
	struct FACE *f0, *f1;
	double cross;

	for(c = 0; c < n_cells; c ++)
	{
		handle(1,cell_oriented_new(&cell[c]),"allocating cell orientations");

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
		handle(1,face_oriented_new(&face[i]),"allocating face orientations");

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

			handle(1,k < face[i].border[j]->n_faces,"finding corresponding face");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int i;
	double ***polygon;

	handle(1,allocate_double_pointer_matrix(&polygon,MAX(MAX_CELL_FACES,4),2),"allocating polygon memory");

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
}

////////////////////////////////////////////////////////////////////////////////

void generate_control_volume_polygon(double ***polygon, int index, int location, struct FACE *face, struct CELL *cell)
{
	int i;

	if(location == 'f')
	{
		i = 1 + face[index].n_borders;

		polygon[0][0] = polygon[i][1] = face[index].oriented[0] ? face[index].node[1]->x : face[index].node[0]->x;

		polygon[0][1] = polygon[1][0] = face[index].border[0]->centroid;

		polygon[1][1] = polygon[2][0] = face[index].oriented[0] ? face[index].node[0]->x : face[index].node[1]->x;

		if(face[index].n_borders == 2) polygon[2][1] = polygon[3][0] = face[index].border[1]->centroid;
	}
	else if(location == 'c')
	{
		int o;

		for(i = 0; i < cell[index].n_faces; i ++)
		{
			o = cell[index].oriented[i];
			polygon[i][0] = cell[index].face[i]->node[!o]->x;
			polygon[i][1] = cell[index].face[i]->node[o]->x;
		}
	}
	else handle(1,0,"recognising the location");
}

////////////////////////////////////////////////////////////////////////////////

void calculate_polygon_centroid(int n, double ***polygon, double *centroid)
{
	double x[2], nx[2], area;
	int i, j;

	area = 0;
	centroid[0] = 0;
	centroid[1] = 0;

	for(i = 0; i < n; i ++)
	{
		nx[0] = polygon[i][1][1] - polygon[i][0][1];
		nx[1] = polygon[i][0][0] - polygon[i][1][0];

		for(j = 0; j < 2; j ++)
		{
			x[0] = 0.5*polygon[i][0][0]*(1.0 - gauss_x[1][j]) + 0.5*polygon[i][1][0]*(1.0 + gauss_x[1][j]);
			x[1] = 0.5*polygon[i][0][1]*(1.0 - gauss_x[1][j]) + 0.5*polygon[i][1][1]*(1.0 + gauss_x[1][j]);

			area += x[0]*nx[0]*gauss_w[1][j]*0.5;

			centroid[0] += x[0]*x[1]*nx[1]*gauss_w[1][j]*0.5;
			centroid[1] += x[0]*x[1]*nx[0]*gauss_w[1][j]*0.5;
		}
	}

	centroid[0] /= area;
	centroid[1] /= area;
}

////////////////////////////////////////////////////////////////////////////////
