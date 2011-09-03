////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "csr.h"

#include "ilupack.h"
#include "umfpack.h"

////////////////////////////////////////////////////////////////////////////////

struct s_CSR
{
	int n, n_space; //number of rows
	int nnz, nnz_space; //number of non-zero elements
	int *row; //row pointers (length n+1)
	int *diagonal; //diagonal pointers (length n)
	int *index; //column indices (length nnz)
	double *value; //values (length nnz)
};

////////////////////////////////////////////////////////////////////////////////

CSR csr_new()
{
	CSR A;

	A = (CSR)malloc(sizeof(struct s_CSR));
	if(A == NULL) return NULL;

	A->n = A->n_space = 0;
	A->nnz = A->nnz_space = 0;

	A->row = (int *)malloc(sizeof(int));
	A->row[0] = 0;

	A->diagonal = NULL;
	A->index = NULL;
	A->value = NULL;

	return A;
}

////////////////////////////////////////////////////////////////////////////////

int csr_insert_value(CSR A, int row, int index, double value)
{
	int i;

	//allocate row space as necessary
	if(row + 1 > A->n_space)
	{
		A->n_space = 2*A->n_space + row + 1;

		A->row = (int *)realloc(A->row, (A->n_space + 1) * sizeof(int));
		if(A->row == NULL) return CSR_MEMORY_ERROR;

		A->diagonal = (int *)realloc(A->diagonal, A->n_space * sizeof(int));
		if(A->diagonal == NULL) return CSR_MEMORY_ERROR;
		for(i = A->n; i < A->n_space; i ++) A->diagonal[i] = -1;
	}

	//increase the number of rows as necessary
	if(row + 1 > A->n)
	{
		for(i = A->n + 1; i <= row + 1; i ++) A->row[i] = A->row[A->n];
		A->n = row + 1;
	}

	//move along the row to find the insert point
	for(i = A->row[row]; i < A->row[row + 1]; i ++)
	{
		if(A->index[i] == index) { A->value[i] += value; return CSR_SUCCESS; }
		if(A->index[i] > index) break;
	}

	//insert point
	int insert = i;

	//allocate index/value space as necessary
	if(A->nnz + 1 > A->nnz_space)
	{
		A->nnz_space = 2*A->nnz_space + 1;

		A->index = (int *)realloc(A->index, A->nnz_space * sizeof(int));
		if(A->index == NULL) return CSR_MEMORY_ERROR;

		A->value = (double *)realloc(A->value, A->nnz_space * sizeof(double));
		if(A->value == NULL) return CSR_MEMORY_ERROR;
	}

	//shift all values past the insert point back by one
	for(i = A->row[A->n]; i > insert; i --)
	{
		A->index[i] = A->index[i-1];
		A->value[i] = A->value[i-1];
	}

	//insert the index and a zero value
	A->index[insert] = index;
	A->value[insert] = value;

	//note diagonal
	if(index == row) A->diagonal[row] = insert;

	//increment the pointers to rows/diagonals past the value
	for(i = row + 1; i <= A->n; i ++) A->row[i] ++;
	for(i = row + (index >= row); i < A->n; i ++) A->diagonal[i] += A->diagonal[i] >= 0;

	//increment the total number of nonzeros
	A->nnz ++;

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void csr_set_row(CSR A, int row, double *value)
{
	int i;
	for(i = A->row[row]; i < A->row[row+1]; i ++)
	{
		A->value[i] = value[A->index[i]];
		value[A->index[i]] = 0.0;
	}
}

////////////////////////////////////////////////////////////////////////////////

void csr_add_to_diagonal(CSR A, double *value)
{
	int i;
	for(i = 0; i < A->n; i ++)
	{
		if(A->diagonal[i] >= 0) A->value[A->diagonal[i]] += value[i];
	}
}

////////////////////////////////////////////////////////////////////////////////

void csr_multiply_vector(CSR A, double *b, double *c)
{
	int i, j;
	for(i = 0; i < A->n; i ++)
	{
		for(j = A->row[i]; j < A->row[i+1]; j ++)
		{
			c[i] += A->value[j]*b[A->index[j]];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void csr_empty(CSR A)
{
	A->n = A->nnz = 0;
}

////////////////////////////////////////////////////////////////////////////////

void csr_print(CSR A)
{
	int i, j;
	for(i = 0; i < A->n; i ++)
	{
		for(j = A->row[i]; j < A->row[i+1]; j ++)
		{
			printf("%5i %5i %+15.10e\n",i,A->index[j],A->value[j]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void csr_print_diagonal(CSR A)
{
	int i;
	for(i = 0; i < A->n; i ++)
	{
		if(A->diagonal[i] >= 0) printf("%5i %15.10e\n",i,A->value[A->diagonal[i]]);
	}
}

////////////////////////////////////////////////////////////////////////////////

int csr_solve_umfpack(CSR A, double *x, double *b)
{
	void *Symbolic, *Numeric;
	
	umfpack_di_symbolic(A->n, A->n, A->row, A->index, A->value, &Symbolic, NULL, NULL);
	
	umfpack_di_numeric(A->row, A->index, A->value, Symbolic, &Numeric, NULL, NULL);
        umfpack_di_free_symbolic(&Symbolic);

	umfpack_di_solve(UMFPACK_At, A->row, A->index, A->value, x, b, Numeric, NULL, NULL);
        umfpack_di_free_numeric(&Numeric);

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int csr_solve_ilupack(CSR A, double *x, double *b)
{
	int i;

	//convert to numbering from 1
	for(i = 0; i <= A->n; i ++) A->row[i] ++;
	for(i = 0; i < A->nnz; i ++) A->index[i] ++;

	Dmat M;
	DAMGlevelmat P;
	DILUPACKparam param;

	M.nr = M.nc = A->n;
	M.nnz = A->nnz;
	M.ia = A->row;
	M.ja = A->index;
	M.a = A->value;

	DGNLAMGinit(&M, &param);

	param.matching = 0;
	param.ordering = "amd";
	param.droptol = 1.0;
	param.droptolS = 0.1;
	param.condest = 5;

	if(DGNLAMGfactor(&M, &P, &param) != 0) return CSR_SOLVE_ERROR;

	DGNLAMGsol(&P, &param, b, x);

	if(DGNLAMGsolver(&M, &P, &param, b, x) != 0) return CSR_SOLVE_ERROR;

	DGNLAMGdelete(&M,&P,&param);

	//convert back to numbering from 0
	for(i = 0; i <= A->n; i ++) A->row[i] --;
	for(i = 0; i < A->nnz; i ++) A->index[i] --;

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void csr_destroy(CSR A)
{
	free(A->row);
	free(A->index);
	free(A->value);
	free(A);
}

////////////////////////////////////////////////////////////////////////////////
