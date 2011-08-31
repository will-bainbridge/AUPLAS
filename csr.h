#ifndef CSR_H
#define CSR_H

typedef struct s_CSR * CSR;

CSR csr_new();
int csr_add_value_to_last_row(CSR A, int index, double value);
int csr_append_empty_row(CSR A);
int csr_append_sparse_row(CSR A, int n, int *index, double *value);
int csr_append_dense_row(CSR A, int n, double *value);
void csr_empty(CSR A);
void csr_print(CSR A);
int csr_solve_superlu(CSR A, double *b);
int csr_solve_csparse(CSR A, double *b);
int csr_solve_umfpack(CSR A, double *b);
void csr_destroy(CSR A);

#define CSR_SUCCESS 1
#define CSR_SOLVE_ERROR -1
#define CSR_MEMORY_ERROR -1

#endif
