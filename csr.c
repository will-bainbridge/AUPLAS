////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "csr.h"

#include "umfpack.h"

////////////////////////////////////////////////////////////////////////////////

struct s_CSR
{
	int n, n_space; //number of rows
	int nnz, nnz_space; //number of non-zero elements
	int *row; //row pointers (length n)
	int *index; //column indices (length nnz)
	double *value; //values (length nnz)
};

////////////////////////////////////////////////////////////////////////////////

CSR csr_new()
{
	CSR A;

	A = (CSR)malloc(sizeof(struct s_CSR));
	if(A == NULL) return NULL;

	A->n = 0;
	A->n_space = 1;
	A->nnz = A->nnz_space = 0;

	A->row = (int *)malloc(sizeof(int));
	A->row[0] = 0;

	A->index = NULL;
	A->value = NULL;

	return A;
}

////////////////////////////////////////////////////////////////////////////////

int csr_create_nonzero(CSR A, int row, int index)
{
	//allocate row space as necessary
	if(row + 2 > A->n_space)
	{
		A->n_space = 2*A->n_space + row + 2;

		A->row = (int *)realloc(A->row, A->n_space * sizeof(int));
		if(A->row == NULL) return CSR_MEMORY_ERROR;
	}

	int i;

	//increase the number of rows as necessary
	if(A->n < row + 1)
	{
		for(i = A->n + 1; i <= row + 1; i ++) A->row[i] = A->row[A->n];
		A->n = row + 1;
	}

	//move along the row to find the insert point
	for(i = A->row[row]; i < A->row[row + 1]; i ++)
	{
		if(A->index[i] == index) return CSR_SUCCESS;
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
	A->value[insert] = 0.0;

	//increment the pointers to rows past the value
	for(i = row + 1; i <= A->n; i ++) A->row[i] ++;

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

int csr_solve_umfpack(CSR A, double *b)
{
	void *Symbolic, *Numeric;
	
	umfpack_di_symbolic(A->n, A->n, A->row, A->index, A->value, &Symbolic, NULL, NULL);
	
	umfpack_di_numeric(A->row, A->index, A->value, Symbolic, &Numeric, NULL, NULL);
        umfpack_di_free_symbolic(&Symbolic);

	double *x = (double *)malloc(A->n * sizeof(double));
	if(x == NULL) return CSR_MEMORY_ERROR;

	umfpack_di_solve(UMFPACK_At, A->row, A->index, A->value, x, b, Numeric, NULL, NULL);
        umfpack_di_free_numeric(&Numeric);

	int i;
	for(i = 0; i < A->n; i ++) b[i] = x[i];

	free(x);

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
