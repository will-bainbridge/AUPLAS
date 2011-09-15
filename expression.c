///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "expression.h"

#define PLUS '+'
#define MINUS '-'
#define MULTIPLY '*'
#define DIVIDE '/'
#define POWER '^'

#define LEFTBRACE '('
#define RIGHTBRACE ')'

#define EQUALITY '='
#define SEPERATOR ';'
#define SUBSTITUTE '$'

#define SUBSTITUTE_ZERO '`' //must have a larger ascii value than all the others
#define VALUE '#'
#define EMPTY ' '
#define END '\0'

#define IS_OPERATOR(x) ( \
		(x) == PLUS || \
		(x) == MINUS || \
		(x) == MULTIPLY || \
		(x) == DIVIDE || \
		(x) == POWER )

#define IS_CONTROL(x) ( \
		(x) == LEFTBRACE || \
		(x) == RIGHTBRACE )

#define IS_VALUE(x) ( \
		(x) == '0' || (x) == '1' || (x) == '2' || (x) == '3' || (x) == '4' || \
		(x) == '5' || (x) == '6' || (x) == '7' || (x) == '8' || (x) == '9' || \
		(x) == '.' )

#define IS_VARIABLE(x) ( \
		! IS_OPERATOR(x) && \
		! IS_CONTROL(x) && \
		! IS_VALUE(x) && \
		(x) != EQUALITY && \
		(x) != SEPERATOR && \
		(x) != SUBSTITUTE )

#define MAX_ELEMENTS 128
#define MAX_STRING 1024

static int precedence[128] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,2,1,0,1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

struct s_EXPRESSION
{
	int type;
	double value;
};

int expression_simplify(EXPRESSION expression, double *substitute);
void expression_remove_empty(EXPRESSION expression, int *index);

///////////////////////////////////////////////////////////////////////////////

/*int main()
{
	char *string = (char *)malloc(MAX_STRING * sizeof(char));
	strcpy(string,"cp=1005.0;cv=718.0;gamma=cp/cv;R=cp-cv;p0=1.0e5;(($1*2.0*gamma*R/(gamma-1.0))*((p0/$0)^(gamma/(gamma-1.0))-1.0))^0.5");
	//strcpy(string,"mu=0.001;rho=1.0;mu/rho");

	printf("     input > %s\n",string);
	
	EXPRESSION expression = expression_generate(string);

	printf("simplified > "); expression_print(expression); printf("\n");

	double value;
	double substitute[2] = { 9.5e4 , 300.0 };

	expression_evaluate(&value, expression, substitute);

	printf("     value > %lf\n", value);

	substitute[0] = 9.0e4;

	expression_evaluate(&value, expression, substitute);

	printf("     value > %lf\n", value);

	//-------------------------------------------------------------------//

	//int i, precedence[128];
	//for(i = 0; i < 128; i ++) precedence[i] = 0;
	//precedence[EMPTY] = -1;
	//precedence[PLUS] = 1;
	//precedence[MINUS] = 1;
	//precedence[MULTIPLY] = 2;
	//precedence[DIVIDE] = 2;
	//precedence[POWER] = 3;
	//printf("static int precedence[128] = {");
	//for(i = 0; i < 127; i ++) printf("%i,",precedence[i]);
	//printf("%i};\n",precedence[127]);

	//-------------------------------------------------------------------//

	free(string);
	expression_destroy(expression);

	return 0;
}*/

///////////////////////////////////////////////////////////////////////////////

EXPRESSION expression_generate(char *original)
{
	int i, length, offset;
	char *string = (char *)malloc(MAX_STRING * sizeof(char));
	strcpy(string,original);

	//remove whitespace

	offset = 0;
	length = strlen(string);

	for(i = 0; i < length; i ++)
	{
		if(string[i + offset] == ' ')
		{
			offset ++;
			length --;
		}
		string[i] = string[i + offset];
	}
	string[i] = '\0';

	//convert multiple expressions into just one

	length = strlen(string);

	int n_name, n_expr;
	char *name = (char *)malloc(MAX_STRING * sizeof(char));
	char *expr = (char *)malloc(MAX_STRING * sizeof(char));
	char *temp = (char *)malloc(MAX_STRING * sizeof(char));
	if(name == NULL || expr == NULL || temp == NULL) return NULL;

	while(IS_VARIABLE(string[0]))
	{
		i = 0;

		do name[i] = string[i]; while(string[++i] != EQUALITY);
		n_name = i;
		name[n_name] = '\0';

		i ++;

		do expr[i - n_name - 1] = string[i]; while(string[++i] != SEPERATOR);
		n_expr = i - n_name - 1;
		expr[n_expr] = '\0';

		i ++;

		while(string[i] == ';') i ++;

		sprintf(temp,"%s",&string[i]);
		strcpy(string,temp);

		i = 0;

		do
		{
			if(strncmp(&string[i],name,n_name) == 0)
			{
				string[i] = '\0';

				sprintf(temp,"%s%c%s%c%s",string,LEFTBRACE,expr,RIGHTBRACE,&string[i + n_name]);

				strcpy(string,temp);
			}

			i ++;

		} while(i < strlen(string));
	}

	free(name);
	free(expr);
	free(temp);

	//convert the string into lists of operations

	length = strlen(string);

	int n = 0, index;
	EXPRESSION expression = (EXPRESSION)malloc(length * sizeof(struct s_EXPRESSION));
	if(expression == NULL) return NULL;

	i = 0;
	do {
		if(IS_OPERATOR(string[i]) || IS_CONTROL(string[i]))
		{
			expression[n++].type = string[i++];
		}
		else if(string[i] == SUBSTITUTE)
		{
			sscanf(&string[++i],"%i",&index);
			expression[n++].type = SUBSTITUTE_ZERO + index;
			while(IS_VALUE(string[i])) i ++;
		}
		else if(IS_VALUE(string[i]))
		{
			sscanf(&string[i],"%lf",&expression[n].value);
			expression[n++].type = VALUE;

			//skip over a floating point number
			while((i < length) &&
					((!IS_OPERATOR(string[i]) && !IS_CONTROL(string[i])) ||
					 ((string[i] == MINUS || string[i] == PLUS) &&
					  (string[i-(i>0)] == 'e' || string[i-(i>0)] == 'E')))) i ++;
		}
		else return NULL;

	} while(i < length);

	expression[n].type = END;

	free(string);

	//simplify the expression

	int info = expression_simplify(expression, NULL);

	n = 0;
	while(expression[n++].type != END);
	expression = (EXPRESSION)realloc(expression, (n + 1) * sizeof(struct s_EXPRESSION));

	return info ? expression : NULL;
}

///////////////////////////////////////////////////////////////////////////////

void expression_destroy(EXPRESSION expression)
{
	free(expression);
}

///////////////////////////////////////////////////////////////////////////////

void expression_print(EXPRESSION expression)
{
	int i = 0;

	while(expression[i].type != END)
	{
		if(IS_OPERATOR(expression[i].type) || IS_CONTROL(expression[i].type))
		{
			printf("%c",expression[i].type);
		}
		else if(expression[i].type == VALUE)
		{
			printf("%g",expression[i].value);
		}
		else if(expression[i].type >= SUBSTITUTE_ZERO)
		{
			printf("$%i",expression[i].type - SUBSTITUTE_ZERO);
		}

		i ++;
	}
}

///////////////////////////////////////////////////////////////////////////////

int expression_evaluate(double *value, EXPRESSION expression, double *substitute)
{
	if(expression[0].type == VALUE && expression[1].type == END)
	{
		*value = expression[0].value;
		return 1;
	}

	int n = 0;
	while(expression[n++].type != END);

	EXPRESSION copy = (EXPRESSION)malloc((n + 1) * sizeof(struct s_EXPRESSION));
	if(copy == NULL) return 0;

	memcpy(copy, expression, (n + 1) * sizeof(struct s_EXPRESSION));

	int info = expression_simplify(copy, substitute);

	*value = copy[0].value;

	info = info && copy[0].type == VALUE && copy[1].type == END;

	free(copy);

	return info;
}

///////////////////////////////////////////////////////////////////////////////

int expression_simplify(EXPRESSION expression, double *substitute)
{
	int i, operations, p, p_l, p_r;

	//substitute variables into expression
	i = 0;
	while(substitute != NULL && expression[i].type != END)
	{
		if(expression[i].type >= SUBSTITUTE_ZERO)
		{
			expression[i].value = substitute[expression[i].type - SUBSTITUTE_ZERO];
		}

		i ++;
	}

	//evaluate as many terms as possible
	do {
		operations = 0;

		//remove brackets around single values
		i = 1;
		while(expression[i+1].type != END)
		{
			if(expression[i].type == VALUE || expression[i].type >= SUBSTITUTE_ZERO)
			{
				if(expression[i-1].type == LEFTBRACE && expression[i+1].type == RIGHTBRACE)
				{
					expression[i-1].type = expression[i+1].type = EMPTY;
					expression_remove_empty(expression, &i);
				}
			}

			i ++;
		}

		/*//absorb negation into values
		i = 0;
		while(expression[i+1].type != END)
		{
			if(		expression[i].type == MINUS &&
					(expression[i+1].type == VALUE || expression[i+1].type >= SUBSTITUTE_ZERO) &&
					(IS_OPERATOR(expression[i-(i>0)].type) || IS_CONTROL(expression[i-(i>0)].type)))
			{
				expression[i].type = EMPTY;
				expression[i+1].value = - expression[i+1].value;
				expression_remove_empty(expression, &i);
			}

			i ++;
		}*/

		//perform operations
		i = 1;
		while(expression[i+1].type != END)
		{
			if(IS_OPERATOR(expression[i].type))
			{
				if(IS_OPERATOR(expression[i-1].type) || IS_OPERATOR(expression[i+1].type)) return 0;
				if(IS_CONTROL(expression[i-1].type) || IS_CONTROL(expression[i+1].type)) { i++; continue; }
				if((expression[i-1].type >= SUBSTITUTE_ZERO || expression[i+1].type >= SUBSTITUTE_ZERO) && substitute == NULL) { i ++; continue; }

				p = precedence[expression[i].type];
				p_l = p_r = precedence[EMPTY];
				if(i > 1) if(IS_OPERATOR(expression[i-2].type)) p_l = precedence[expression[i-2].type];
				if(IS_OPERATOR(expression[i+2].type)) p_r = precedence[expression[i+2].type];

				if(p >= p_l && p >= p_r)
				{
					switch(expression[i].type)
					{
						case POWER:
							expression[i].value = pow(expression[i-1].value, expression[i+1].value);
							break;
						case MULTIPLY:
							expression[i].value = expression[i-1].value * expression[i+1].value;
							break;
						case DIVIDE:
							expression[i].value = expression[i-1].value / expression[i+1].value;
							break;
						case PLUS:
							expression[i].value = expression[i-1].value + expression[i+1].value;
							break;
						case MINUS:
							expression[i].value = expression[i-1].value - expression[i+1].value;
							break;
					}

					expression[i].type = VALUE;
					expression[i-1].type = expression[i+1].type = EMPTY;
					expression_remove_empty(expression, &i);

					operations ++;
				}
			}
			i ++;
		}
	} while(operations > 0);

	//return succesful
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void expression_remove_empty(EXPRESSION expression, int *index)
{
	int i = 0, offset = 0;

	do {
		if(expression[i + offset].type == EMPTY)
		{
			offset ++;
		}
		expression[i].type = expression[i + offset].type;
		expression[i].value = expression[i + offset].value;

		if(*index == i + offset) *index = i;

	} while(expression[i++].type != END);
}

///////////////////////////////////////////////////////////////////////////////
