////////////////////////////////////////////////////////////////////////////////

#include "caspar.h"
#include "allocate.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	//check input arguments
	if(argc != 2) { printf("\nERROR - need exactly one argument, the input fileame\n\n"); return ERROR; }

	//--------------------------------------------------------------------//

	//open the input file
	FILE *input_file;
	input_file = fopen(argv[1],"r");
	if(input_file == NULL) { printf("\nERROR - opening the input file\n\n"); return ERROR; }

	//get the number of variables
	int n_variables;
	if(read_labelled_values_from_file(input_file,"number_of_variables",'i',1,&n_variables) != SUCCESS)
	{ printf("\nERROR - number_of_variables not found in input file\n\n"); return ERROR; }

	//get the geometry filename
	char *geometry_filename;
	if(allocate_character_vector(&geometry_filename,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating geometry_filename memory\n\n"); return ERROR; }
	if(read_labelled_values_from_file(input_file,"geometry_filename",'s',1,&geometry_filename) != SUCCESS)
	{ printf("\nERROR - geometry_filename not found in input file\n\n"); return ERROR; }

	//get variable interpolation parameters
	int *maximum_order;
	if(allocate_integer_vector(&maximum_order,n_variables) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating maximum_order memory\n\n"); return ERROR; }
	if(read_labelled_values_from_file(input_file,"maximum_order",'i',n_variables,maximum_order) != SUCCESS)
	{ printf("\nERROR - maximum_order not found in input file\n\n"); return ERROR; }
	double *weight_exponent;
	if(allocate_double_vector(&weight_exponent,n_variables) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating weight_exponent memory\n\n"); return ERROR; }
	if(read_labelled_values_from_file(input_file,"weight_exponent",'d',n_variables,weight_exponent) != SUCCESS)
	{ printf("\nERROR - weight_exponent not found in input file\n\n"); return ERROR; }
	char **connectivity;
	if(allocate_character_matrix(&connectivity,n_variables,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating connectivity memory\n\n"); return ERROR; }
	if(read_labelled_values_from_file(input_file,"connectivity",'s',n_variables,connectivity) != SUCCESS)
	{ printf("\nERROR - connectivity not found in input file\n\n"); return ERROR; }

	//--------------------------------------------------------------------//
	
	int n_nodes, n_faces, n_cells;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	if(read_geometry_file(geometry_filename, &n_nodes, node, &n_faces, face, &n_cells, cell) != SUCCESS)
	{ printf("\nERROR - reading geometry file\n\n"); return ERROR; }

	printf("%i %i %i\n", n_nodes, n_faces, n_cells);
	
	//--------------------------------------------------------------------//

	//clean up
	fclose(input_file);
	free_vector(geometry_filename);
	free_vector(maximum_order);
	free_vector(weight_exponent);
	free_matrix((void*)connectivity);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
