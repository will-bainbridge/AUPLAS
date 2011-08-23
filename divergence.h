#ifndef DIVERGENCE_H
#define DIVERGENCE_H

#include "fetch.h"

#define DIVERGENCE_LABEL "divergence"
#define DIVERGENCE_FORMAT "iscsd"

#define MAX_DIVERGENCE_VARIABLES 5

#define DIVERGENCE_SUCCESS 1
#define DIVERGENCE_FETCH_ERROR 0
#define DIVERGENCE_FORMAT_ERROR -1
#define DIVERGENCE_MEMORY_ERROR -2

struct s_DIVERGENCE
{
	int equation;
	int n_variables;
	int *variable;
	int *differential;
	int direction;
	double constant;
};

typedef struct s_DIVERGENCE * DIVERGENCE;

DIVERGENCE divergence_new();
DIVERGENCE * divergences_new(DIVERGENCE *divergence, int n_old, int n_new);
void divergences_read(char *filename, int *n_divergences, DIVERGENCE **divergence);
void divergence_destroy(DIVERGENCE divergence);
void divergences_destroy(int n_divergences, DIVERGENCE *divergence);

#endif
