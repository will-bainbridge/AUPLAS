///////////////////////////////////////////////////////////////////////////////

typedef struct s_EXPRESSION * EXPRESSION;

EXPRESSION expression_generate(char *string);
void expression_destroy(EXPRESSION expression);
void expression_print(EXPRESSION expression);
int expression_evaluate(double *value, EXPRESSION expression, double *substitute);

///////////////////////////////////////////////////////////////////////////////
