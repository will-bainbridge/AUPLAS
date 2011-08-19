#ifndef CSR_H
#define CSR_H

typedef struct _csr * csr;

csr csr_new();
int csr_append_row(csr A, int n, double *row);
void csr_empty(csr A);
void csr_print(csr A);
int csr_solve_superlu(csr A, double *b);
int csr_solve_csparse(csr A, double *b);
void csr_destroy(csr A);

#define CSR_SUCCESS 1
#define CSR_SOLVE_ERROR -1
#define CSR_MEMORY_ERROR -1

#endif
