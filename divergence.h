#ifndef DIVERGENCE_H
#define DIVERGENCE_H

#define DIVERGENCE_SUCCESS 1
#define DIVERGENCE_FETCH_ERROR 0
#define DIVERGENCE_FORMAT_ERROR -1
#define DIVERGENCE_MEMORY_ERROR -2

typedef struct
{
	int equation;
	int n_variables;
	int *variable;
	int *differential;
	int direction;
	double constant;
}
DIVERGENCE;

DIVERGENCE * divergences_new(DIVERGENCE *divergence, int n_old, int n_new);
void divergences_input(char *filename, int *n_divergences, DIVERGENCE **divergence);
void divergences_destroy(int n_divergences, DIVERGENCE *divergence);

#endif
