////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "differential.h"
#include "divergence.h"
#include "handle.h"

#define MAX_DIVERGENCES 100
#define MAX_VARIABLES 5
#define MAX_STRING_LENGTH 128

////////////////////////////////////////////////////////////////////////////////

DIVERGENCE divergence_new()
{
	DIVERGENCE divergence = (DIVERGENCE)malloc(sizeof(struct s_DIVERGENCE));
	if(divergence == NULL) return NULL;

	divergence->n_variables = 0;
	divergence->variable = NULL;
	divergence->differential = NULL;

	return divergence;
}

////////////////////////////////////////////////////////////////////////////////

DIVERGENCE * divergences_new(DIVERGENCE *divergence, int n_old, int n_new)
{
	int i;

	for(i = n_new; i < n_old; i ++) divergence_destroy(divergence[i]);

	divergence = (DIVERGENCE *)realloc(divergence, n_new * sizeof(DIVERGENCE));
	if(divergence == NULL) return NULL;

	for(i = n_old; i < n_new; i ++)
	{
		divergence[i] = divergence_new();
		if(divergence[i] == NULL) return NULL;
	}

	return divergence;
}

////////////////////////////////////////////////////////////////////////////////

void divergences_read(char *filename, int *n_divergences, DIVERGENCE **divergence)
{
	//open the file
	FILE *file = fopen(filename,"r");
	handle(1,file != NULL,"opening input file");

	//fetch the data
	FETCH fetch = fetch_new(DIVERGENCE_FORMAT,MAX_DIVERGENCES);
	handle(1,fetch != NULL,"allocating fetch");
	int n_fetch = fetch_read(file,DIVERGENCE_LABEL,fetch);
	handle(1,n_fetch > 1,"no divergences found in input file");
	handle(0,n_fetch < MAX_DIVERGENCES,"maximum number of divergences reached");

	//allocate pointers
	DIVERGENCE *div = divergences_new(NULL,0,n_fetch);
	handle(1,div != NULL,"allocating divergences");

	//counters
	int i, j, n = 0, info;

	//temporary storage
	char direction;
	int offset, n_diff, diff[2];
	char *piece = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING_LENGTH * sizeof(char));
	int *term = (int *)malloc(MAX_VARIABLES * sizeof(int));
	int *differential = (int *)malloc(MAX_VARIABLES * sizeof(int));
	handle(1,piece != NULL && temp != NULL && term != NULL && differential != NULL,"allocating temporary storage");

	for(i = 0; i < n_fetch; i ++)
	{
		//equation
		fetch_get(fetch, i, 0, &div[n]->equation);

		//constant
		fetch_get(fetch, i, 4, &div[n]->constant);

		//direction
		fetch_get(fetch, i, 2, &direction);
		if(direction == 'x') {
			div[n]->direction = 0;
		} else if(direction == 'y') {
			div[n]->direction = 1;
		} else {
			handle(1,info = 0,"skipping divergence with unrecognised direction");
			continue;
		}

		//variables
		fetch_get(fetch, i, 1, piece);
		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(piece); j ++) if(piece[j] == ',') piece[j] = ' ';
		//sequentially read variables
		offset = div[n]->n_variables = 0;
		while(offset < strlen(piece))
		{
			//read the variable from the string
			info = sscanf(&piece[offset],"%s",temp);
			info *= sscanf(temp,"%i",&term[div[n]->n_variables++]);
			handle(0,info,"skipping divergence with unrecognised variable format");
			if(!info) continue;

			//move to the next variable in the string
			offset += strlen(temp) + 1;
		}

		//differentials
		fetch_get(fetch, i, 3, piece);
		//convert comma delimiters to whitespace
		for(j = 0; j < strlen(piece); j ++) if(piece[j] == ',') piece[j] = ' ';
		//sequentially read differentials
		offset = n_diff = 0;
		while(offset < strlen(piece))
		{
			//read the variables' differential string
			info = sscanf(&piece[offset],"%s",temp);
			handle(0,info,"skipping divergence with unrecognised differentail format");
			if(!info) continue;

			//count the differentials in the different dimensions
			j = diff[0] = diff[1] = 0;
			while(temp[j] != '\0')
			{
				diff[0] += (temp[j] == 'x');
				diff[1] += (temp[j] == 'y');
				j ++;
			}
			//convert to a unique differential index
			differential[n_diff ++] = differential_index[diff[0]][diff[1]];
			//move to the next differential in the string
			offset += strlen(temp) + 1;
		}

		//check numbers
		info = div[n]->n_variables == n_diff;
		handle(0,info,"skipping divergence with different numbers of variables and differentials");
		if(!info) continue;

		//allocate the variable and differential arrays
		div[n]->variable = (int *)malloc(div[n]->n_variables * sizeof(int));
		div[n]->differential = (int *)malloc(div[n]->n_variables * sizeof(int));
		handle(1,div[n]->variable != NULL && div[n]->differential != NULL,"allocating divergence variables and differentials");

		//copy over
		for(j = 0; j < div[n]->n_variables; j ++)
		{
			div[n]->variable[j] = term[j];
			div[n]->differential[j] = differential[j];
		}

		//increment the number of divergences
		n ++;
	}

	//resize
	div = divergences_new(div,n_fetch,n);
	handle(1,div != NULL,"re-allocating divergences");

	//check numbers
	fetch_destroy(fetch);
	fetch = fetch_new("",MAX_DIVERGENCES);
	handle(0,fetch_read(file,DIVERGENCE_LABEL,fetch) == n,"skipping divergences with unrecognised formats");

	//copy over
	*n_divergences = n;
	*divergence = div;

	//clean up
	fclose(file);
	fetch_destroy(fetch);
	free(piece);
	free(temp);
	free(term);
	free(differential);
}

////////////////////////////////////////////////////////////////////////////////

void divergence_destroy(DIVERGENCE divergence)
{
	free(divergence->variable);
	free(divergence->differential);
	free(divergence);
}

////////////////////////////////////////////////////////////////////////////////

void divergences_destroy(int n_divergences, DIVERGENCE *divergence)
{
	int i;
	for(i = 0; i < n_divergences; i ++) divergence_destroy(divergence[i]);
	free(divergence);
}

////////////////////////////////////////////////////////////////////////////////
