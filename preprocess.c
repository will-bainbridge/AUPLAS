////////////////////////////////////////////////////////////////////////////////

#include "auplas.h"
#include "allocate.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	//input arguments
	if(argc != 2) { printf("\nERROR - need exactly one argument, the input fileame\n\n"); return ERROR; }
	char *input_filename = argv[1];

	//--------------------------------------------------------------------//

	//get the number of variables
	int n_variables;
	if(read_labelled_values(input_filename,"number_of_variables","i",&n_variables) != SUCCESS)
	{ printf("\nERROR - number_of_variables not found in input file\n\n"); return ERROR; }

	//get the geometry filename
	char *geometry_filename;
	if(allocate_character_vector(&geometry_filename,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating geometry_filename memory\n\n"); return ERROR; }
	if(read_labelled_values(input_filename,"geometry_filename","s",&geometry_filename) != SUCCESS)
	{ printf("\nERROR - geometry_filename not found in input file\n\n"); return ERROR; }

        //get variable interpolation parameters
        char *format;
        if(allocate_character_vector(&format,n_variables+1) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating format memory\n\n"); return ERROR; }
        format[n_variables] = '\0';

        memset(format,'i',n_variables);
        int *maximum_order;
        if(allocate_integer_vector(&maximum_order,n_variables) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating maximum_order memory\n\n"); return ERROR; }
        if(read_labelled_values(input_filename,"maximum_order",format,maximum_order) != SUCCESS)
        { printf("\nERROR - maximum_order not found in input file\n\n"); return ERROR; }

        memset(format,'d',n_variables);
        double *weight_exponent;
        if(allocate_double_vector(&weight_exponent,n_variables) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating weight_exponent memory\n\n"); return ERROR; }
        if(read_labelled_values(input_filename,"weight_exponent",format,weight_exponent) != SUCCESS)
        { printf("\nERROR - weight_exponent not found in input file\n\n"); return ERROR; }

        memset(format,'s',n_variables);
        char **connectivity;
        if(allocate_character_matrix(&connectivity,n_variables,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating connectivity memory\n\n"); return ERROR; }
        if(read_labelled_values(input_filename,"connectivity",format,connectivity) != SUCCESS)
        { printf("\nERROR - connectivity not found in input file\n\n"); return ERROR; }

	free_vector(format);

	//--------------------------------------------------------------------//

	int n_nodes, n_faces, n_cells;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	if(read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell) != SUCCESS)
	{ printf("\nERROR - reading geometry file\n\n"); return ERROR; }

	//--------------------------------------------------------------------//

	//clean up
	free_vector(geometry_filename);
	free_vector(maximum_order);
	free_vector(weight_exponent);
	free_matrix((void*)connectivity);
	free_mesh_structures(n_nodes, node, n_faces, face, n_cells, cell);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void free_mesh_structures(int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell)
{
	int i;

	free(node);

	free(face);

	for(i = 0; i < n_cells; i ++)
	{
		free(cell[i].face);
	}
	free(cell);
}

////////////////////////////////////////////////////////////////////////////////
