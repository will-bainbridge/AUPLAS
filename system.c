////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "quadrature.h"
#include "polynomial.h"

void dgemv_(char *, int *, int *, double *, double *, int *, double *, int *, double *, double *, int *);

double ddot_(int *, double *, int *, double *, int *);

////////////////////////////////////////////////////////////////////////////////

void generate_system_lists(int *n_ids, int **id_to_unknown, int *n_unknowns, int **unknown_to_id, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
{
	int i, j;
	
	*n_ids = *n_unknowns = 0;

	for(i = 0; i < n_faces; i ++) for(j = 0; j < face[i].n_zones; j ++) *n_ids = MAX(*n_ids,INDEX_AND_ZONE_TO_ID(i,(int)(face[i].zone[j]-&zone[0])));
	for(i = 0; i < n_cells; i ++) for(j = 0; j < cell[i].n_zones; j ++) *n_ids = MAX(*n_ids,INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0])));
	(*n_ids) ++;

	handle(allocate_lists(*n_ids,id_to_unknown,0,NULL) == ALLOCATE_SUCCESS,"allocating id to system indices");

	for(i = 0; i < *n_ids; i ++) (*id_to_unknown)[i] = -1;

	for(i = 0; i < n_faces; i ++) {
		for(j = 0; j < face[i].n_zones; j ++) {
			if(face[i].zone[j]->condition[0] == 'u') {
				(*id_to_unknown)[INDEX_AND_ZONE_TO_ID(i,(int)(face[i].zone[j]-&zone[0]))] = (*n_unknowns) ++;
			} 
		}
	}
	for(i = 0; i < n_cells; i ++) {
		//for(j = 0; j < cell[i].n_zones; j ++) {
		//{
		{ j = 0;
			if(cell[i].zone[j]->condition[0] == 'u') {
				(*id_to_unknown)[INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0]))] = (*n_unknowns) ++;
			}
		}
	}
	for(i = 0; i < n_cells; i ++) {
		if(cell[i].n_zones > 1)
		{ j = 1;
			if(cell[i].zone[j]->condition[0] == 'u') {
				(*id_to_unknown)[INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0]))] = (*n_unknowns) ++;
			}
		}
	}

	handle(allocate_lists(0,NULL,*n_unknowns,unknown_to_id) == ALLOCATE_SUCCESS,"allocating system indices to id");

	for(i = 0; i < *n_ids; i ++) if((*id_to_unknown)[i] >= 0) (*unknown_to_id)[(*id_to_unknown)[i]] = i;
}

////////////////////////////////////////////////////////////////////////////////

void assemble_matrix(struct SPARSE *matrix, int n_ids, int *id_to_unknown, int n_unknowns, int *unknown_to_id, double *lhs, double *rhs, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone, int n_divergences, struct DIVERGENCE *divergence)
{
        int i, j, k, id, z, d, u;

        int n_polygon, max_n_polygon = MAX(MAX_CELL_FACES,4);

        double ***polygon;
        handle(allocate_double_pointer_matrix(&polygon,max_n_polygon,2) == ALLOCATE_SUCCESS,"allocating polygon memory");

        int *n_interpolant;
        handle(allocate_integer_vector(&n_interpolant,max_n_polygon) == ALLOCATE_SUCCESS,"allocating the number of interpolants");

        struct CELL ***interpolant;
        interpolant = (struct CELL ***)malloc(max_n_polygon * sizeof(struct CELL **));
        handle(interpolant != NULL,"allocating the interpolant pointers to pointers");
        interpolant[0] = (struct CELL **)malloc(max_n_polygon * MAX_INTERPOLANTS * sizeof(struct CELL **));
        handle(interpolant[0] != NULL,"allocating the interpolant pointers");
        for(i = 1; i < max_n_polygon; i ++) interpolant[i] = interpolant[i-1] + MAX_INTERPOLANTS;

        double *row;
        handle(allocate_double_vector(&row,n_unknowns) == ALLOCATE_SUCCESS,"allocating the row");

	//zero the right hand size
	for(i = 0; i < n_unknowns; i ++) rhs[i] = 0.0;

	for(u = 0; u < n_unknowns; u ++)
        {
		id = unknown_to_id[u];

                i = ID_TO_INDEX(id);
                z = ID_TO_ZONE(id);

		generate_control_volume_polygon(polygon, i, zone[z].location, face, cell);

		if(zone[z].location == 'f')
		{
			n_polygon = 2 + face[i].n_borders;
			for(j = 0; j < n_polygon; j ++) n_interpolant[j] = 1;
			interpolant[0][0] = interpolant[1][0] = interpolant[2][0] = face[i].border[0];
			if(face[i].n_borders == 2) interpolant[2][0] = interpolant[3][0] = face[i].border[1];
		}
		else if(zone[z].location == 'c')
		{
			n_polygon = cell[i].n_faces;
			for(j = 0; j < n_polygon; j ++)
			{
				n_interpolant[j] = cell[i].face[j]->n_borders;
				for(k = 0; k < n_interpolant[j]; k ++) interpolant[j][k] = cell[i].face[j]->border[k];
			}
		}
		else handle(0,"recognising the location");

		//------------------------------------------------------------//

		for(j = 0; j < n_unknowns; j ++) row[j] = 0.0;

		for(d = 0; d < n_divergences; d ++)
		{
			if(divergence[d].equation != zone[z].variable) continue;
			calculate_divergence(n_polygon, polygon, n_interpolant, interpolant, id_to_unknown, lhs, &rhs[u], row, zone, &divergence[d]);
		}

		//----------//
		matrix->row[u] = matrix->nnz;
		if(matrix->nnz + matrix->n > matrix->space)
		{
			matrix->space = MAX(2*matrix->space,matrix->space+matrix->n);
			allocate_sparse_matrix(matrix);
		}
		for(j = 0; j < n_unknowns; j ++)
		{
			if(fabs(row[j]) > 0.0)
			{
				matrix->index[matrix->nnz] = j;
				matrix->value[matrix->nnz++] = row[j];
			}
		}
	}

	matrix->row[matrix->n] = matrix->nnz;

	free_matrix((void **)polygon);
	free_vector(n_interpolant);
	free(interpolant[0]);
	free(interpolant);
	free_vector(row);
}

////////////////////////////////////////////////////////////////////////////////

void calculate_divergence(int n_polygon, double ***polygon, int *n_interpolant, struct CELL ***interpolant, int *id_to_unknown, double *lhs, double *rhs, double *row, struct ZONE *zone, struct DIVERGENCE *divergence)
{
	int i, j, k, p, q, s, u;

	int max_order = 0, max_stencil;
	for(p = 0; p < n_polygon; p ++) {
		for(i = 0; i < n_interpolant[p]; i ++) {
			for(j = 0; j < divergence->n_variables; j ++) {
				max_order = MAX(max_order,interpolant[p][i]->order[divergence->variable[j]]);
				max_stencil = MAX(max_stencil,interpolant[p][i]->n_stencil[divergence->variable[j]]);
			}
		}
	}

	double *interpolation_values, point_value, value;
	handle(allocate_double_vector(&interpolation_values,max_stencil) == ALLOCATE_SUCCESS,"allocating interpolation values");

	double *polynomial;
	handle(allocate_double_vector(&polynomial,ORDER_TO_POWERS(max_order)) == ALLOCATE_SUCCESS,"allocating polynomial");

	double x[2], normal;

	char trans = 'N';
	int m, n, increment = 1;
	double alpha = 1.0, beta = 0.0;

	for(p = 0; p < n_polygon; p ++)
	{
		if(divergence->direction == 0) normal = polygon[p][1][1] - polygon[p][0][1];
		else                           normal = polygon[p][0][0] - polygon[p][1][0];

		for(q = 0; q < max_order; q ++)
		{
			point_value = 1.0;

			for(i = 0; i < n_interpolant[p]; i ++)
			{
				x[0] =  0.5*polygon[p][0][0]*(1.0 - gauss_x[max_order-1][q]) +
					0.5*polygon[p][1][0]*(1.0 + gauss_x[max_order-1][q]) -
					interpolant[p][i]->centroid[0];
				x[1] =  0.5*polygon[p][0][1]*(1.0 - gauss_x[max_order-1][q]) +
					0.5*polygon[p][1][1]*(1.0 + gauss_x[max_order-1][q]) -
					interpolant[p][i]->centroid[1];

				for(j = divergence->n_variables - 1; j >= 0; j --)
				{
					u = divergence->variable[j];

					m = ORDER_TO_POWERS(interpolant[p][i]->order[u]);
					n = interpolant[p][i]->n_stencil[u];

					for(k = 0; k < m; k ++)
					{
						polynomial[k] = polynomial_coefficient[divergence->differential[j]][k] *
							integer_power(x[0],polynomial_power_x[divergence->differential[j]][k]) *
							integer_power(x[1],polynomial_power_y[divergence->differential[j]][k]);
					}

					//multiply polynomial and matrix
					dgemv_(&trans, &n, &m, &alpha, interpolant[p][i]->matrix[u][0], &n, polynomial, &increment, &beta, interpolation_values, &increment);

					// ??? combine these loops once tested ???
					if(j > 0)
					{
						value = 0;

						for(k = 0; k < n; k ++)
						{
							s = interpolant[p][i]->stencil[u][k];

							if(zone[ID_TO_ZONE(s)].condition[0] == 'u') {
								value += interpolation_values[k] * lhs[id_to_unknown[s]];
							} else {
								value += interpolation_values[k] * zone[ID_TO_ZONE(s)].value;
							}
						}

						point_value *= value;
					}
					else
					{
						for(k = 0; k < n; k ++)
						{
							s = interpolant[p][i]->stencil[u][k];

							if(zone[ID_TO_ZONE(s)].condition[0] == 'u') {
								row[id_to_unknown[s]] += divergence->constant * normal *
									gauss_w[max_order-1][q] * point_value *
									interpolation_values[k] / n_interpolant[p];
							} else {
								*rhs -= divergence->constant * normal *
									gauss_w[max_order-1][q] * point_value *
									interpolation_values[k] * zone[ID_TO_ZONE(s)].value /
									n_interpolant[p];
							}
						}
					}
				}
			}
		}
	}

	free_vector(interpolation_values);
	free_vector(polynomial);
}

////////////////////////////////////////////////////////////////////////////////
