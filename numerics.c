////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

#include "polynomial.h"
#include "quadrature.h"

#define LS_SUCCESS 1
#define LS_MEMORY_ERROR -1
#define LS_DIMENSION_ERROR -2

void dgeqrf_(int * n, int * m, double *, int *, double *, double *, int *, int *);
void dorgqr_(int *, int *, int *, double *, int *, double *, double *, int *, int *);
void dgemm_(char *, char *, int *, int *, int *, double *, double *, int *, double *, int *, double *, double *, int *);
void dgetrf_(int *, int *, double *, int *, int *, int *);
void dgetri_(int *, double *, int *, int *, double *, int *, int *);
void dgels_(char *, int *, int *, int *, double *, int *, double *, int *, double *, int *, int *);

////////////////////////////////////////////////////////////////////////////////

int calculate_cell_reconstruction_matrices(int n_variables, double *weight_exponent, int *maximum_order, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone)
{
	int c, u, i, j, k;

	int order, n_powers, n_stencil;

	//find the overall maximum order
	int maximum_maximum_order = 0;
	for(u = 0; u < n_variables; u ++) if(maximum_order[u] > maximum_maximum_order) maximum_maximum_order = maximum_order[u];

	//memory allocation
	if(allocate_mesh(n_variables, 0, NULL, 0, NULL, n_cells, &cell, 0, NULL) != SUCCESS)
	{ printf("\nERROR - calculate_cell_reconstruction_matrices - allocating cell matrices"); return ERROR; }
	double **matrix;
	if(allocate_double_matrix(&matrix,ORDER_TO_POWERS(maximum_maximum_order),MAX_STENCIL) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - calculate_cell_reconstruction_matrices - allocating matrix memory"); return ERROR; }
	int n_constraints, *constraint;
	if(allocate_integer_vector(&constraint,MAX_STENCIL) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - calculate_cell_reconstruction_matrices - allocating constraint memory"); return ERROR; }
	double *weight;
	if(allocate_double_vector(&weight,MAX_STENCIL) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - calculate_cell_reconstruction_matrices - allocating weight memory"); return ERROR; }

	//stencil element properties
	int s_id, s_index;
	struct ZONE *s_zone;
	char s_location, *s_condition;
	double *s_centroid, s_weight;

	//integration
	double x[2];
	int differential[2], d;

	for(c = 0; c < n_cells; c ++)
	{
		for(u = 0; u < n_variables; u ++)
		{
			//problem size
			order = cell[c].order[u];
			n_powers = ORDER_TO_POWERS(order);
			n_stencil = cell[c].n_stencil[u];
			n_constraints = 0;

			for(i = 0; i < n_stencil; i ++)
			{
				//stencil element properties
				s_id = cell[c].stencil[u][i];
				s_index = ID_TO_INDEX(s_id);
				s_zone = &zone[ID_TO_ZONE(s_id)];
				s_location = s_zone->location;
				s_condition = s_zone->condition;

				if(s_location == 'f') {
					s_centroid = face[s_index].centroid;
				} else if(s_location == 'c') {
					s_centroid = cell[s_index].centroid;
				} else { printf("\nERROR - calculate_cell_reconstruction_matrices - recognising zone location"); return ERROR; }

				s_weight  = (s_centroid[0] - cell[c].centroid[0])*(s_centroid[0] - cell[c].centroid[0]);
				s_weight += (s_centroid[1] - cell[c].centroid[1])*(s_centroid[1] - cell[c].centroid[1]);
				s_weight  = 1.0/pow(s_weight,0.5*weight_exponent[u]);
				if(s_location == 'c' && s_index == c) s_weight = 1.0;

				weight[i] = s_weight;

				//unknown and dirichlet conditions have zero differentiation
				if(s_condition[0] == 'u' || s_condition[0] == 'd')
				{
					differential[0] = differential[1] = 0;
				}
				//determine differentiation from the numbers of x and y characters in the condition string
				else
				{
					j = 0;
					while(s_condition[j] != '\0')
					{
						differential[0] += (s_condition[j] == 'x');
						differential[1] += (s_condition[j] == 'y');
					}
				}

				//index for the determined differential
				d = differential_index[differential[0]][differential[1]];

				//unknowns fit to centroid points
				//change to fit to CV average instead???
				if(s_condition[0] == 'u')
				{
					x[0] = s_centroid[0] - cell[c].centroid[0];
					x[1] = s_centroid[1] - cell[c].centroid[1];

					for(j = 0; j < n_powers; j ++)
					{
						matrix[j][i] = polynomial_coefficient[d][j]*
							integer_power(x[0],polynomial_power_x[d][j])*
							integer_power(x[1],polynomial_power_y[d][j])*
							s_weight;
					}
				}
				//known faces fit to face average
				//cells need implementing
				else
				{
					if(s_location == 'f')
					{
						for(j = 0; j < n_powers; j ++) matrix[j][i] = 0.0;

						for(j = 0; j < order; j ++)
						{
							x[0] =  0.5*face[s_index].node[0]->x[0]*(1.0 - gauss_x[order-1][j]) +
								0.5*face[s_index].node[1]->x[0]*(1.0 + gauss_x[order-1][j]) - 
								cell[c].centroid[0];
							x[1] =  0.5*face[s_index].node[0]->x[1]*(1.0 - gauss_x[order-1][j]) +
								0.5*face[s_index].node[1]->x[1]*(1.0 + gauss_x[order-1][j]) -
								cell[c].centroid[1];

							for(k = 0; k < n_powers; k ++)
							{
								matrix[k][i] += polynomial_coefficient[d][k]*
									integer_power(x[0],polynomial_power_x[d][k])*
									integer_power(x[1],polynomial_power_y[d][k])*
									s_weight*gauss_w[order-1][j]*0.5;
							}
						}
					}

					//if(s_location == 'c')
					//{
					//}
				}

				//constraints are the centre cell and any dirichlet boundaries
				if((s_location == 'c' && s_index == c) || s_condition[0] == 'd') constraint[n_constraints++] = i;
			}

			//solve
			if(n_constraints > 0) {
				if(constrained_least_squares(n_stencil,n_powers,matrix,n_constraints,constraint) != LS_SUCCESS)
				{ printf("\nERROR - calculate_cell_reconstruction_matrices - doing constrained least squares calculation"); return ERROR; }
			} else {
				if(least_squares(n_stencil,n_powers,matrix) != LS_SUCCESS)
				{ printf("\nERROR - calculate_cell_reconstruction_matrices - doing least squares calculation"); return ERROR; }
			}

			//multiply by the weights
			for(i = 0; i < n_powers; i ++) for(j = 0; j < n_stencil; j ++) matrix[i][j] *= weight[j];

			//store in the cell structure
			for(i = 0; i < n_powers; i ++) for(j = 0; j < n_stencil; j ++) cell[c].matrix[u][i][j] = matrix[i][j];
		}
	}

	//clean up
	free_matrix((void**)matrix);
	free_vector(constraint);
	free_vector(weight);

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int least_squares(int m, int n, double **matrix)
{
	if(m < 1 || n < 1 || n > m) return LS_DIMENSION_ERROR;

	int i, j;

	int info, lwork = m*m;
	double *work; if(allocate_double_vector(&work, lwork) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	char transa = 'N';

	double **a_matrix; if(allocate_double_matrix(&a_matrix, n, m) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	for(i = 0; i < m; i ++) for(j = 0; j < n; j ++) a_matrix[j][i] = matrix[j][i];

	double **b_matrix; if(allocate_double_matrix(&b_matrix, m, m) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	for(i = 0; i < m; i ++) for(j = 0; j < m; j ++) b_matrix[i][j] = (i == j);

	dgels_(&transa, &m, &n, &m, a_matrix[0], &m, b_matrix[0], &m, work, &lwork, &info);

	for(i = 0; i < n; i ++) for(j = 0; j < m; j ++) matrix[i][j] = b_matrix[j][i];

	free_matrix((void**)a_matrix);
	free_matrix((void**)b_matrix);
	free_vector(work);
	return LS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int constrained_least_squares(int m, int n, double **matrix, int c, int *constrained)
{
	//check problem dimensions
	if(m < 1 || n < 1 || n > m || c > n) return LS_DIMENSION_ERROR;

	//counters
	int i, j;

	//extra problem dimensions
	int f = m - c, u = n - c;

	//lapack and blas inputs
	char transa, transb;
	double alpha, beta;

	//lapack output
	int info;

	//lapack workspace
	int lwork = m*m;
	double *work; if(allocate_double_vector(&work, lwork) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }

	//lapack LU pivot indices
	int *ipiv; if(allocate_integer_vector(&ipiv,c) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }

	//lapack coefficients of QR elementary reflectors
	double *tau; if(allocate_double_vector(&tau,c) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }

	//matrices used
	double **t_matrix; if(allocate_double_matrix(&t_matrix, m, m) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	double **c_matrix; if(allocate_double_matrix(&c_matrix, n, n) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	double **r_matrix; if(allocate_double_matrix(&r_matrix, c, c) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	double **a_matrix; if(allocate_double_matrix(&a_matrix, n, f) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	double **d_matrix; if(allocate_double_matrix(&d_matrix, f, f) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }

	//indices of unconstrained equations
	int *temp, *unconstrained;
	if(allocate_integer_vector(&temp,m) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }
	if(allocate_integer_vector(&unconstrained,f) != ALLOCATE_SUCCESS) { return LS_MEMORY_ERROR; }

	//create vector of unconstrained indices
	for(i = 0; i < m; i ++) temp[i] = 0;
	for(i = 0; i < c; i ++) temp[constrained[i]] = 1;
	j = 0;
	for(i = 0; i < m; i ++) if(!temp[i]) unconstrained[j++] = i;

	//copy unconstrained equations from input matrix -> t_matrix
	for(i = 0; i < f; i ++) for(j = 0; j < n; j ++) t_matrix[i][j] = matrix[j][unconstrained[i]];

	//copy constrained equations from input matrix -> c_matrix
	for(i = 0; i < c; i ++) for(j = 0; j < n; j ++) c_matrix[i][j] = matrix[j][constrained[i]];

	//QR decomposition of the transposed constrained equations -> c_matrix
	dgeqrf_(&n, &c, c_matrix[0], &n, tau, work, &lwork, &info);

	//copy R out of the above QR decomposition -> r_matrix
	for(i = 0; i < c; i ++) for(j = 0; j < c; j ++) r_matrix[i][j] = ((j >= i) ? c_matrix[j][i] : 0);

	//form the square matrix Q from the above QR decomposition -> c_matrix'
	dorgqr_(&n, &n, &c, c_matrix[0], &n, tau, work, &lwork, &info);

	//multiply unconstrained eqations by Q -> a_matrix'
	transa = 'T'; transb = 'N'; alpha = 1.0; beta = 0.0;
	dgemm_(&transa, &transb, &f, &n, &n, &alpha, t_matrix[0], &m, c_matrix[0], &n, &beta, a_matrix[0], &f);

	//invert R' of the above QR decomposition -> r_matrix
	dgetrf_(&c, &c, r_matrix[0], &c, ipiv, &info);
	dgetri_(&c, r_matrix[0], &c, ipiv, work, &lwork, &info);

	//LS inversion of the non-square parts from unconstrained * Q -> d_matrix'
	for(i = 0; i < f; i ++) for(j = 0; j < u; j ++) t_matrix[j][i] = a_matrix[j+c][i];
	for(i = 0; i < f; i ++) for(j = 0; j < f; j ++) d_matrix[i][j] = (i == j);
	transa = 'N';
	dgels_(&transa, &f, &u, &f, t_matrix[0], &m, d_matrix[0], &f, work, &lwork, &info);

	//multiply matrices together to form the CLS solution -> t_matrix'
	transa = transb = 'N'; alpha = 1.0; beta = 0.0;
	dgemm_(&transa, &transb, &n, &f, &u, &alpha, c_matrix[c], &n, d_matrix[0], &f, &beta, t_matrix[0], &m);

	alpha = -1.0; beta = 1.0;
	dgemm_(&transa, &transb, &n, &c, &f, &alpha, t_matrix[0], &m, a_matrix[0], &f, &beta, c_matrix[0], &n);

	alpha = 1.0; beta = 0.0;
	dgemm_(&transa, &transb, &n, &c, &c, &alpha, c_matrix[0], &n, r_matrix[0], &c, &beta, t_matrix[f], &m);

	//copy the result out of the temporary matrix -> matrix
	for(i = 0; i < n; i ++) for(j = 0; j < f; j ++) matrix[i][unconstrained[j]] = t_matrix[j][i];
	for(i = 0; i < n; i ++) for(j = 0; j < c; j ++) matrix[i][constrained[j]] = t_matrix[j+f][i];

	//clean up and return successful
	free_vector(work);
	free_vector(ipiv);
	free_vector(tau);
	free_vector(temp);
	free_vector(unconstrained);
	free_matrix((void **)t_matrix);
	free_matrix((void **)c_matrix);
	free_matrix((void **)r_matrix);
	free_matrix((void **)a_matrix);
	free_matrix((void **)d_matrix);
	return LS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

double integer_power(double base, int exp)
{
	int i;
	double result = 1.0;
	for(i = 0; i < exp; i ++) result *= base;
	return result;
}

////////////////////////////////////////////////////////////////////////////////