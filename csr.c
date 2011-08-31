////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "csr.h"

#include "umfpack.h"
#include "slu_ddefs.h"
#include "cs.h"

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

int csr_add_value_to_last_row(CSR A, int index, double value)
{
	int i;

	for(i = A->row[A->n-1]; i < A->row[A->n]; i ++)
	{
		if(A->index[i] == index)
		{
			A->value[i] += value;
			return CSR_SUCCESS;
		}

		if(A->index[i] > index) break;
	}

	if(A->nnz + 1 > A->nnz_space)
	{
		A->nnz_space = 2*A->nnz_space + 1;

		A->index = (int *)realloc(A->index, A->nnz_space * sizeof(int));
		if(A->index == NULL) return CSR_MEMORY_ERROR;

		A->value = (double *)realloc(A->value, A->nnz_space * sizeof(double));
		if(A->value == NULL) return CSR_MEMORY_ERROR;
	}

	int insert = i;

	for(i = A->row[A->n]; i > insert; i --)
	{
		A->index[i] = A->index[i-1];
		A->value[i] = A->value[i-1];
	}

	A->index[insert] = index;
	A->value[insert] = value;

	A->row[A->n] ++;
	A->nnz ++;

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int csr_append_empty_row(CSR A)
{
	if(A->n + 2 > A->n_space)
	{
		A->n_space = 2*A->n_space + 2;

		A->row = (int *)realloc(A->row, A->n_space * sizeof(int));
		if(A->row == NULL) return CSR_MEMORY_ERROR;
	}

	A->n += 1;

	A->row[A->n] = A->row[A->n-1];

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int csr_append_sparse_row(CSR A, int n, int *index, double *value)
{
	if(A->n + 2 > A->n_space)
	{
		A->n_space = 2*A->n_space + 2;

		A->row = (int *)realloc(A->row, A->n_space * sizeof(int));
		if(A->row == NULL) return CSR_MEMORY_ERROR;
	}

	if(A->nnz + n > A->nnz_space)
	{
		A->nnz_space = 2*A->nnz_space + n;

		A->index = (int *)realloc(A->index, A->nnz_space*sizeof(int));
		if(A->index == NULL) return CSR_MEMORY_ERROR;

		A->value = (double *)realloc(A->value, A->nnz_space*sizeof(double));
		if(A->value == NULL) return CSR_MEMORY_ERROR;
	}

	A->row[A->n] = A->nnz;

	A->n += 1;

	int i;
	for(i = 0; i < n; i ++)
	{
		if(A->index[A->nnz-1] == index[i])
		{
			A->value[A->nnz-1] += value[i];
		}
		else
		{
			A->index[A->nnz] = index[i];
			A->value[A->nnz ++] = value[i];
		}
	}

	A->row[A->n] = A->nnz;

	return CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int csr_append_dense_row(CSR A, int n, double *value)
{
	if(A->n + 2 > A->n_space)
	{
		A->n_space = 2*A->n_space + 2;

		A->row = (int *)realloc(A->row, A->n_space * sizeof(int));
		if(A->row == NULL) return CSR_MEMORY_ERROR;
	}

	if(A->nnz + n > A->nnz_space)
	{
		A->nnz_space = 2*A->nnz_space + n;

		A->index = (int *)realloc(A->index, A->nnz_space*sizeof(int));
		if(A->index == NULL) return CSR_MEMORY_ERROR;

		A->value = (double *)realloc(A->value, A->nnz_space*sizeof(double));
		if(A->value == NULL) return CSR_MEMORY_ERROR;
	}

	A->row[A->n] = A->nnz;

	A->n += 1;

	int i;
	for(i = 0; i < n; i ++)
	{
		if(fabs(value[i]) > 0.0)
		{
			A->index[A->nnz] = i;
			A->value[A->nnz++] = value[i];
		}
	}

	A->row[A->n] = A->nnz;

	return CSR_SUCCESS;
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

int csr_solve_superlu(CSR A, double *b)
{
	SuperMatrix M, L, U, X;
	superlu_options_t options;
	SuperLUStat_t stat;
	int *perm_c, *perm_r;
	int info;

	perm_c = (int *)malloc(A->n * sizeof(int));
	if(perm_c == NULL) return CSR_MEMORY_ERROR;
	perm_r = (int *)malloc(A->n * sizeof(int));
	if(perm_r == NULL) return CSR_MEMORY_ERROR;

	dCreate_CompRow_Matrix(&M, A->n, A->n, A->nnz, A->value, A->index, A->row, SLU_NR, SLU_D, SLU_GE);

	dCreate_Dense_Matrix(&X, A->n, 1, b, A->n, SLU_DN, SLU_D, SLU_GE);

	set_default_options(&options);
	options.ColPerm = NATURAL;

	StatInit(&stat);

	dgssv(&options, &M, perm_c, perm_r, &L, &U, &X, &stat, &info);

	free(perm_c);
	free(perm_r);

	Destroy_SuperMatrix_Store(&M);
	Destroy_SuperMatrix_Store(&X);
	Destroy_SuperNode_Matrix(&L);
	Destroy_CompCol_Matrix(&U);

	StatFree(&stat);

	return info ? CSR_SOLVE_ERROR : CSR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int csr_solve_csparse(CSR A, double *b)
{
	cs *MT, *M;
	MT = cs_calloc(1,sizeof(cs));

	MT->nzmax = A->nnz;
	MT->m = A->n;
	MT->n = A->n;
	MT->p = A->row;
	MT->i = A->index;
	MT->x = A->value;
	MT->nz = -1;

	M = cs_transpose(MT,1);

	int info = cs_lusol(1, M, b, 1e-10);

	cs_spfree(M);
	cs_free(MT);

	return info ? CSR_SUCCESS : CSR_SOLVE_ERROR;
}

////////////////////////////////////////////////////////////////////////////////

int csr_solve_umfpack(CSR A, double *b)
{
	void *Symbolic, *Numeric;
	
	umfpack_di_symbolic(A->n, A->n, A->row, A->index, A->value, &Symbolic, NULL, NULL);
	
	umfpack_di_numeric(A->row, A->index, A->value, Symbolic, &Numeric, NULL, NULL);
        umfpack_di_free_symbolic(&Symbolic);

	double *x = (double *)malloc(A->n * sizeof(double));

	umfpack_di_solve(UMFPACK_At, A->row, A->index, A->value, x, b, Numeric, NULL, NULL);
        umfpack_di_free_numeric(&Numeric);

	int i;
	for(i = 0; i < A->n; i ++) b[i] = x[i];

	free(x);

	return 1;
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
