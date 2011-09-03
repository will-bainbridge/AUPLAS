#ifndef CSR_H
#define CSR_H

typedef struct s_CSR * CSR;

CSR csr_new();
int csr_insert_value(CSR A, int row, int index, double value);
void csr_set_row(CSR A, int row, double *value);
void csr_add_to_diagonal(CSR A, double *value);
void csr_empty(CSR A);
void csr_print(CSR A);
void csr_print_diagonal(CSR A);
int csr_solve_umfpack(CSR A, double *x, double *b);
int csr_solve_ilupack(CSR A, double *x, double *b);
void csr_destroy(CSR A);

#define CSR_SUCCESS 1
#define CSR_SOLVE_ERROR -1
#define CSR_MEMORY_ERROR -1

#endif
