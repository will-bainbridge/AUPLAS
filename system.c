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

	exit_if_false(allocate_integer_vector(id_to_unknown,*n_ids),"allocating id to system indices");

	for(i = 0; i < *n_ids; i ++) (*id_to_unknown)[i] = -1;

	for(i = 0; i < n_faces; i ++) {
		for(j = 0; j < face[i].n_zones; j ++) {
			if(face[i].zone[j]->condition[0] == 'u') {
				(*id_to_unknown)[INDEX_AND_ZONE_TO_ID(i,(int)(face[i].zone[j]-&zone[0]))] = (*n_unknowns) ++;
			} 
		}
	}
	for(i = 0; i < n_cells; i ++) {
		for(j = 0; j < cell[i].n_zones; j ++) {
			if(cell[i].zone[j]->condition[0] == 'u') {
				(*id_to_unknown)[INDEX_AND_ZONE_TO_ID(i,(int)(cell[i].zone[j]-&zone[0]))] = (*n_unknowns) ++;
			}
		}
	}

	exit_if_false(allocate_integer_vector(unknown_to_id,*n_unknowns),"allocating system indices to ids");

	for(i = 0; i < *n_ids; i ++) if((*id_to_unknown)[i] >= 0) (*unknown_to_id)[(*id_to_unknown)[i]] = i;
}

////////////////////////////////////////////////////////////////////////////////

void assemble_matrix(CSR matrix, int n_ids, int *id_to_unknown, int n_unknowns, int *unknown_to_id, double *lhs, double *rhs, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone, int n_divergences, struct DIVERGENCE *divergence)
{
        int i, j, k, id, z, d, u;

        int n_polygon, max_n_polygon = MAX(MAX_CELL_FACES,4);

        double ***polygon;
        exit_if_false(allocate_double_pointer_matrix(&polygon,max_n_polygon,2),"allocating polygon memory");

        int *n_interpolant;
        exit_if_false(allocate_integer_vector(&n_interpolant,max_n_polygon),"allocating the number of interpolants");

        struct CELL ***interpolant;
        interpolant = (struct CELL ***)malloc(max_n_polygon * sizeof(struct CELL **));
        exit_if_false(interpolant != NULL,"allocating the interpolant pointers to pointers");
        interpolant[0] = (struct CELL **)malloc(max_n_polygon * 2 * sizeof(struct CELL **));
        exit_if_false(interpolant[0] != NULL,"allocating the interpolant pointers");
        for(i = 1; i < max_n_polygon; i ++) interpolant[i] = interpolant[i-1] + 2;

        double *row;
        exit_if_false(allocate_double_vector(&row,n_unknowns),"allocating the row");

	//clear the matrix
	csr_empty(matrix);

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
		else exit_if_false(0,"recognising the location");

		for(j = 0; j < n_unknowns; j ++) row[j] = 0.0;

		for(d = 0; d < n_divergences; d ++)
		{
			if(divergence[d].equation != zone[z].variable) continue;
			calculate_divergence(n_polygon, polygon, n_interpolant, interpolant, id_to_unknown, lhs, &rhs[u], row, zone, divergence[d]);
		}

		exit_if_false(csr_append_row(matrix, n_unknowns, row) == CSR_SUCCESS,"appending row to system");
	}

	//clean up
	free_matrix((void **)polygon);
	free_vector(n_interpolant);
	free(interpolant[0]);
	free(interpolant);
	free_vector(row);
}

////////////////////////////////////////////////////////////////////////////////

void calculate_divergence(int n_polygon, double ***polygon, int *n_interpolant, struct CELL ***interpolant, int *id_to_unknown, double *lhs, double *rhs, double *row, struct ZONE *zone, struct DIVERGENCE divergence)
{
	int i, j, k, p, q, s, u;

	int max_order = 0, max_stencil = 0;
	for(p = 0; p < n_polygon; p ++) {
		for(i = 0; i < n_interpolant[p]; i ++) {
			for(j = 0; j < divergence.n_variables; j ++) {
				max_order = MAX(max_order,interpolant[p][i]->order[divergence.variable[j]]);
				max_stencil = MAX(max_stencil,interpolant[p][i]->n_stencil[divergence.variable[j]]);
			}
		}
	}

	double *interpolation_values, point_value, value;
	exit_if_false(allocate_double_vector(&interpolation_values,max_stencil),"allocating interpolation values");

	double *polynomial;
	exit_if_false(allocate_double_vector(&polynomial,ORDER_TO_POWERS(max_order)),"allocating polynomial");

	double x[2], normal;

	char trans = 'N';
	int m, n, increment = 1;
	double alpha = 1.0, beta = 0.0;

	for(p = 0; p < n_polygon; p ++)
	{
		if(divergence.direction == 0) normal = polygon[p][1][1] - polygon[p][0][1];
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

				for(j = divergence.n_variables - 1; j >= 0; j --)
				{
					u = divergence.variable[j];

					m = ORDER_TO_POWERS(interpolant[p][i]->order[u]);
					n = interpolant[p][i]->n_stencil[u];

					for(k = 0; k < m; k ++)
					{
						polynomial[k] = polynomial_coefficient[divergence.differential[j]][k] *
							integer_power(x[0],polynomial_power_x[divergence.differential[j]][k]) *
							integer_power(x[1],polynomial_power_y[divergence.differential[j]][k]);
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
								row[id_to_unknown[s]] += divergence.constant * normal *
									gauss_w[max_order-1][q] * point_value *
									interpolation_values[k] / n_interpolant[p];
							} else {
								*rhs -= divergence.constant * normal *
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

void initialise_unknowns(int n_ids, int *id_to_unknown, struct ZONE *zone, double *x)
{
	int i;
	for(i = 0; i < n_ids; i ++)
	{
		if(id_to_unknown[i] >= 0)
		{
			x[id_to_unknown[i]] = zone[ID_TO_ZONE(i)].value;
		}
	}

}

////////////////////////////////////////////////////////////////////////////////

void calculate_residuals(int n_variables, int n_unknowns, int *unknown_to_id, double *x, double *x1, double *residual, int n_zones, struct ZONE *zone)
{
	int i, u;
	double r;

	double *max = (double *)malloc(n_variables * sizeof(double));
	exit_if_false(max != NULL,"allocating variable maximums");
	double *min = (double *)malloc(n_variables * sizeof(double));
	exit_if_false(min != NULL,"allocating variable minimums");

	for(i = 0; i < n_variables; i ++) residual[i] = 0.0;

	for(i = 0; i < n_zones; i ++) if(zone[i].variable == i && zone[i].condition[0] == 'u') max[i] = min[i] = zone[i].value;

	for(i = 0; i < n_unknowns; i ++)
	{
		u = zone[ID_TO_ZONE(unknown_to_id[i])].variable;
		r = fabs(x1[i] - x[i]);
		residual[u] = MAX(r,residual[u]);

		max[u] = MAX(x1[i],max[u]);
		min[u] = MIN(x1[i],min[u]);
	}

	for(i = 0; i < n_variables; i ++) residual[i] /= max[i] - min[i];

	free(max);
	free(min);
}

////////////////////////////////////////////////////////////////////////////////
