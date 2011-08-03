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
	
	FILE *file = fopen(input_filename,"r");
	void *ptr;
	
	int n_variables;
	ptr = &n_variables;
	if(fetch_read(file, "number_of_variables", "i", 1, &ptr) != 1)
	{ printf("\nERROR - reading number_of_variables\n\n"); return ERROR; }

	char *geometry_filename;
	if(allocate_character_vector(&geometry_filename,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating geometry_filename memory\n\n"); return ERROR; }
	ptr = &geometry_filename;
	if(fetch_read(file, "geometry_filename", "s", 1, &ptr) != 1)
	{ printf("\nERROR - reading geometry_filename\n\n"); return ERROR; }

        char *format;
        if(allocate_character_vector(&format,n_variables+1) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating format memory\n\n"); return ERROR; }
        format[n_variables] = '\0';

        memset(format,'i',n_variables);
	
	int *maximum_order;
	if(allocate_integer_vector(&maximum_order,n_variables) != ALLOCATE_SUCCESS)
	{ printf("\nERROR - allocating maximum_order memory\n\n"); return ERROR; }
	if(fetch_read(file, "maximum_order", format, 1, &maximum_order) != 1)
	{ printf("\nERROR - reading maximum_order\n\n"); return ERROR; }

        memset(format,'d',n_variables);

        double *weight_exponent;
        if(allocate_double_vector(&weight_exponent,n_variables) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating weight_exponent memory\n\n"); return ERROR; }
	if(fetch_read(file, "weight_exponent", format, 1, &weight_exponent) != 1)
	{ printf("\nERROR - reading weight_exponent\n\n"); return ERROR; }

        memset(format,'s',n_variables);

        char **connectivity;
        if(allocate_character_matrix(&connectivity,n_variables,MAX_STRING_LENGTH) != ALLOCATE_SUCCESS)
        { printf("\nERROR - allocating connectivity memory\n\n"); return ERROR; }
	if(fetch_read(file, "connectivity", format, 1, &connectivity) != 1)
	{ printf("\nERROR - reading connectivity\n\n"); return ERROR; }

	fclose(file);
	free_vector(format);

	//--------------------------------------------------------------------//

	int n_nodes, n_faces, n_cells;
	struct NODE *node;
	struct FACE *face;
	struct CELL *cell;
	if(read_geometry(geometry_filename, &n_nodes, &node, &n_faces, &face, &n_cells, &cell) != SUCCESS)
	{ printf("\nERROR - reading geometry file\n\n"); return ERROR; }

	//--------------------------------------------------------------------//
	
	int n_zones;
	struct ZONE *zone;
	if(read_zones(input_filename, &n_zones, &zone) != SUCCESS)
	{ printf("\nERROR - reading zones\n\n"); return ERROR; }

	if(generate_connectivity(n_faces, face, n_cells, cell) != SUCCESS)
	{ printf("\nERROR - generating connectivity\n\n"); return ERROR; }

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

	for(i = 0; i < n_faces; i ++)
	{
		free(face[i].border);
	}
	free(face);

	for(i = 0; i < n_cells; i ++)
	{
		free(cell[i].face);
	}
	free(cell);
}

////////////////////////////////////////////////////////////////////////////////
