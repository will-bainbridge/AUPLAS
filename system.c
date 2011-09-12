////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "quadrature.h"
#include "polynomial.h"

int generate_control_volume_interpolant(struct CELL ***interpolant, int *n_interpolant, int index, char location, struct FACE *face, struct CELL *cell);

void dgemv_(char *, int *, int *, double *, double *, int *, double *, int *, double *, double *, int *);

////////////////////////////////////////////////////////////////////////////////

int generate_control_volume_interpolant(struct CELL ***interpolant, int *n_interpolant, int index, char location, struct FACE *face, struct CELL *cell)
{
	int i, j;

	if(location == 'f')
	{
		for(i = 0; i < 2 + face[index].n_borders; i ++) n_interpolant[i] = 1;
		interpolant[0][0] = interpolant[1][0] = interpolant[2][0] = face[index].border[0];
		if(face[index].n_borders == 2) interpolant[2][0] = interpolant[3][0] = face[index].border[1];

		return 2 + face[index].n_borders;
	}
	else if(location == 'c')
	{
		for(i = 0; i < cell[index].n_faces; i ++)
		{
			n_interpolant[i] = cell[index].face[i]->n_borders;
			for(j = 0; j < n_interpolant[i]; j ++) interpolant[i][j] = cell[index].face[i]->border[j];
		}

		return cell[index].n_faces;
	}
	else exit_if_false(0,"recognising the location");
}

////////////////////////////////////////////////////////////////////////////////

void generate_lists_of_unknowns(int *n_ids, int **id_to_unknown, int *n_unknowns, int **unknown_to_id, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone)
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

void assemble_matrix(CSR matrix, int n_variables, int *id_to_unknown, int n_unknowns, int *unknown_to_id, struct FACE *face, struct CELL *cell, struct ZONE *zone)
{
	int i, j, k, l, p, u, v, z, id;

        int n_polygon, max_n_polygon = MAX(MAX_CELL_FACES,4);

	int *n_interpolant;
	exit_if_false(allocate_integer_vector(&n_interpolant,max_n_polygon),"allocating the number of interpolants");

	struct CELL ***interpolant;
	exit_if_false(allocate_cell_pointer_matrix(&interpolant,max_n_polygon,2),"allocating the interpolant pointers");

	for(u = 0; u < n_unknowns; u ++)
	{
		id = unknown_to_id[u];

		i = ID_TO_INDEX(id);
		z = ID_TO_ZONE(id);

		n_polygon = generate_control_volume_interpolant(interpolant, n_interpolant, i, zone[z].location, face, cell);

		for(p = 0; p < n_polygon; p ++)
		{
			for(j = 0; j < n_interpolant[p]; j ++)
			{
				for(k = 0; k < n_variables; k ++)
				{
					for(l = 0; l < interpolant[p][j]->n_stencil[k]; l ++)
					{
						v = id_to_unknown[interpolant[p][j]->stencil[k][l]];

						if(v >= 0) csr_insert_value(matrix, u, v, 0.0);
					}
				}
			}
		}
	}

	free_vector(n_interpolant);
	free_matrix((void **)interpolant);
}

////////////////////////////////////////////////////////////////////////////////

void calculate_divergence(double *f, CSR jacobian, double *x, int n_variables, int n_ids, int *id_to_unknown, int n_unknowns, int *unknown_to_id, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone, int n_divergences, struct DIVERGENCE *divergence)
{
	int c, d, e, i, j, p, q, s, t, u, v, z;
        int n_polygon, max_n_polygon = MAX(MAX_CELL_FACES,4);

        double ***polygon;
        int *n_interpolant;
        struct CELL ***interpolant;
        exit_if_false(allocate_double_pointer_matrix(&polygon,max_n_polygon,2),"allocating polygon memory");
        exit_if_false(allocate_integer_vector(&n_interpolant,max_n_polygon),"allocating the number of interpolants");
	exit_if_false(allocate_cell_pointer_matrix(&interpolant,max_n_polygon,2),"allocating the interpolant pointers");

	double *row;
	exit_if_false(allocate_double_zero_vector(&row,n_unknowns),"allocating the dense row");

	int max_order = 0, max_stencil = 0, max_divergence_variables = 0;
	for(c = 0; c < n_cells; c ++)
	{
		for(v = 0; v < n_variables; v ++)
		{
			max_order = MAX(max_order,cell[c].order[v]);
			max_stencil = MAX(max_stencil,cell[c].n_stencil[v]);
		}
	}
	for(d = 0; d < n_divergences; d ++) max_divergence_variables = MAX(max_divergence_variables,divergence[d].n_variables);

	double *polynomial, **interp_coef, *interp_value, point_value, point[2], location[2], normal[2];
	exit_if_false(allocate_double_vector(&polynomial,ORDER_TO_POWERS(max_order)),"allocating polynomial");
	exit_if_false(allocate_double_matrix(&interp_coef,max_divergence_variables,max_stencil),"allocating interpolation coefficients");
	exit_if_false(allocate_double_vector(&interp_value,max_divergence_variables),"allocating interpolation values");

	char trans = 'N';
	int m, n, increment = 1;
	double alpha = 1.0, beta = 0.0;

	for(u = 0; u < n_unknowns; u ++)
        {
		f[u] = 0.0;

                e = ID_TO_INDEX(unknown_to_id[u]);
                z = ID_TO_ZONE(unknown_to_id[u]);

		n_polygon = generate_control_volume_polygon(polygon, e, zone[z].location, face, cell);
		n_polygon = generate_control_volume_interpolant(interpolant, n_interpolant, e, zone[z].location, face, cell);

		for(p = 0; p < n_polygon; p ++)
		{
			//polygon face normal
			normal[0] = polygon[p][1][1] - polygon[p][0][1];
			normal[1] = polygon[p][0][0] - polygon[p][1][0];

			for(q = 0; q < max_order; q ++)
			{
				//integration point coordinates
				point[0] = 0.5*polygon[p][0][0]*(1.0 - gauss_x[max_order-1][q]) + 0.5*polygon[p][1][0]*(1.0 + gauss_x[max_order-1][q]);
				point[1] = 0.5*polygon[p][0][1]*(1.0 - gauss_x[max_order-1][q]) + 0.5*polygon[p][1][1]*(1.0 + gauss_x[max_order-1][q]);

				for(t = 0; t < n_interpolant[p]; t ++)
				{
					//integration location
					location[0] = point[0] - interpolant[p][t]->centroid[0];
					location[1] = point[1] - interpolant[p][t]->centroid[1];

					for(d = 0; d < n_divergences; d ++)
					{
						if(divergence[d].equation != zone[z].variable) continue;

						//calculate coefficients for interpolation to the point
						for(i = 0; i < divergence[d].n_variables; i ++)
						{
							v = divergence[d].variable[i];
							m = ORDER_TO_POWERS(interpolant[p][t]->order[v]);
							n = interpolant[p][t]->n_stencil[v];

							//evaluate the differentiated polynomial at the point
							for(j = 0; j < m; j ++)
							{
								polynomial[j] = polynomial_coefficient[divergence[d].differential[i]][j] *
									integer_power(location[0],polynomial_power_x[divergence[d].differential[i]][j]) *
									integer_power(location[1],polynomial_power_y[divergence[d].differential[i]][j]);
							}

							//multiply polynomial and matrix to get interpolation coefficients at the point
							dgemv_(&trans, &n, &m, &alpha, interpolant[p][t]->matrix[v][0], &n, polynomial, &increment,
									&beta, interp_coef[i], &increment);

							//sum the coefficients to calculate the value at the point
							interp_value[i] = 0;
							for(j = 0; j < n; j ++)
							{
								s = interpolant[p][t]->stencil[v][j];

								if(zone[ID_TO_ZONE(s)].condition[0] == 'u')
								{
									interp_value[i] += interp_coef[i][j] * x[id_to_unknown[s]];
								}
								else
								{
									interp_value[i] += interp_coef[i][j] * zone[ID_TO_ZONE(s)].value;
								}
							}
						}

						//calculate the flux and add to the function
						point_value = divergence[d].coefficient * normal[divergence[d].direction] * gauss_w[max_order-1][q] / n_interpolant[p];
						for(i = 0; i < divergence[d].n_variables; i ++) point_value *= integer_power(interp_value[i],divergence[d].power[i]);
						f[u] -= point_value;

						//calculate the jacobian
						for(i = 0; i < divergence[d].n_variables; i ++)
						{
							v = divergence[d].variable[i];
							n = interpolant[p][t]->n_stencil[v];

							point_value = divergence[d].coefficient * normal[divergence[d].direction] * gauss_w[max_order-1][q] / n_interpolant[p];
							point_value *= divergence[d].power[i] * integer_power(interp_value[i],divergence[d].power[i] - 1);
							for(j = 0; j < divergence[d].n_variables; j ++)
								if(j != i) point_value *= integer_power(interp_value[j],divergence[d].power[j]);

							for(j = 0; j < n; j ++)
							{
								s = interpolant[p][t]->stencil[v][j];

								if(zone[ID_TO_ZONE(s)].condition[0] == 'u')
								{
									row[id_to_unknown[s]] += interp_coef[i][j] * point_value;
								}
							}
						}
					}
				}
			}
		}

		//enter the calculated row into the matrix
		csr_set_row(jacobian, u, row);
	}

	//clean up
	free_matrix((void **)polygon);
	free_vector(n_interpolant);
	free_matrix((void **)interpolant);
	free_vector(row);
	free_vector(polynomial);
	free_matrix((void **)interp_coef);
	free_vector(interp_value);
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

void calculate_residuals(int n_variables, int n_unknowns, int *unknown_to_id, double *dx, double *x, double *residual, int n_zones, struct ZONE *zone)
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
		r = fabs(dx[i]);
		residual[u] = MAX(r,residual[u]);

		max[u] = MAX(x[i],max[u]);
		min[u] = MIN(x[i],min[u]);
	}

	for(i = 0; i < n_variables; i ++) residual[i] /= max[i] - min[i];

	free(max);
	free(min);
}

////////////////////////////////////////////////////////////////////////////////
