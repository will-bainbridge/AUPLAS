////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "differential.h"
#include "divergence.h"

////////////////////////////////////////////////////////////////////////////////

DIVERGENCE divergence_new()
{
	DIVERGENCE divergence;

	divergence = (DIVERGENCE)malloc(sizeof(struct s_DIVERGENCE));
	if(divergence == NULL) return NULL;

	divergence->n_variables = 0;
	divergence->variable = NULL;
	divergence->differential = NULL;

	return divergence;
}

////////////////////////////////////////////////////////////////////////////////

int divergence_get(FETCH fetch, int index, DIVERGENCE divergence)
{
	//counters
	int i;

	//temporary storage
	char direction;
	int offset, nd, d[2];
	char *piece = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	int *term = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	int *differential = (int *)malloc(MAX_DIVERGENCE_VARIABLES * sizeof(int));
	if(piece == NULL || temp == NULL || term == NULL || differential == NULL) return DIVERGENCE_MEMORY_ERROR;

	//equation
	fetch_get(fetch, index, 0, &divergence->equation);

	//constant
	fetch_get(fetch, index, 4, &divergence->constant);

	//direction
	fetch_get(fetch, index, 2, &direction);
	if(direction == 'x') {
		divergence->direction = 0;
	} else if(direction == 'y') {
		divergence->direction = 1;
	} else return DIVERGENCE_FORMAT_ERROR;

	//variables
	fetch_get(fetch, index, 1, piece);
	//convert comma delimiters to whitespace
	for(i = 0; i < strlen(piece); i ++) if(piece[i] == ',') piece[i] = ' ';
	//sequentially read variables
	offset = 0;
	while(offset < strlen(piece))
	{
		//read the variable from the string
		if(sscanf(&piece[offset],"%s",temp) != 1) return DIVERGENCE_FORMAT_ERROR;
		if(sscanf(temp,"%i",&term[divergence->n_variables++]) != 1) return DIVERGENCE_FORMAT_ERROR;
		//move to the next variable in the string
		offset += strlen(temp) + 1;
	}

	//differentials
	fetch_get(fetch, index, 3, piece);
	//convert comma delimiters to whitespace
	for(i = 0; i < strlen(piece); i ++) if(piece[i] == ',') piece[i] = ' ';
	//sequentially read differentials
	offset = nd = 0;
	while(offset < strlen(piece))
	{
		//read the variables' differential string
		if(sscanf(&piece[offset],"%s",temp) != 1) return DIVERGENCE_FORMAT_ERROR;
		//count the differentials in the different dimensions
		i = d[0] = d[1] = 0;
		while(temp[i] != '\0')
		{
			d[0] += (temp[i] == 'x');
			d[1] += (temp[i] == 'y');
			i ++;
		}
		//convert to a unique differential index
		differential[nd++] = differential_index[d[0]][d[1]];
		//move to the next differential in the string
		offset += strlen(temp) + 1;
	}

	//check numbers
	if(divergence->n_variables != nd) return DIVERGENCE_FORMAT_ERROR;

	//allocate the variable and differential arrays
	divergence->variable = (int *)malloc(divergence->n_variables * sizeof(int));
	divergence->differential = (int *)malloc(divergence->n_variables * sizeof(int));
	if(divergence->variable == NULL || divergence->differential == NULL) return DIVERGENCE_MEMORY_ERROR;

	//copy over
	for(i = 0; i < divergence->n_variables; i ++)
	{
		divergence->variable[i] = term[i];
		divergence->differential[i] = differential[i];
	}

	//clean up
	free(piece);
	free(temp);
	free(term);
	free(differential);

	return DIVERGENCE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

void divergence_destroy(DIVERGENCE divergence)
{
	free(divergence->variable);
	free(divergence->differential);
	free(divergence);
}

////////////////////////////////////////////////////////////////////////////////
