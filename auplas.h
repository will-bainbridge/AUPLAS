////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//error handling macro
#define handle(value,action) do { if( !(value) ){ printf("\n[ERROR %s:%i] %s\n\n",__FILE__,__LINE__,action); exit(EXIT_FAILURE); } } while(0)

//error handlind return values
#define ALLOCATE_SUCCESS 1
#define ALLOCATE_ERROR 0
#define FETCH_FILE_ERROR -1
#define FETCH_MEMORY_ERROR -2

//maximum extents for memory allocation
#define MAX_CELL_FACES 5
#define MAX_FACE_NODES 2
#define MAX_NEIGHBOURS 10
#define MAX_STENCIL 100
#define MAX_ZONES 100
#define MAX_INDICES 100
#define MAX_STRING_CHARACTERS 128

//cell order to number of powers
#define ORDER_TO_POWERS(order) ((order+1.0)*(order+2.0)*0.5)

//min and max macros
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

//id macros
#define ID_TO_ZONE(id) ((id) % MAX_ZONES)
#define ID_TO_INDEX(id) ((id) / MAX_ZONES)
#define INDEX_AND_ZONE_TO_ID(i,z) ((i)*MAX_ZONES + (z))

////////////////////////////////////////////////////////////////////////////////

struct NODE
{
	double x[2];
};

struct FACE
{
	int n_nodes;
	struct NODE **node;

	int n_borders;
	struct CELL **border;
	int *oriented;

	int n_zones;
	struct ZONE **zone;

	double centroid[2];
};

struct CELL
{
	int n_faces;
	struct FACE **face;
	int *oriented;

	int n_zones;
	struct ZONE **zone;

	double centroid[2];

	int *order;
	int *n_stencil;
	int **stencil;
	double ***matrix;
};

struct ZONE
{
	char location;
	int variable;
	char condition[MAX_STRING_CHARACTERS];
	double value;
};

////////////////////////////////////////////////////////////////////////////////

//io.c
void read_instructions(char *filename, int *n_variables, int **maximum_order, double **weight_exponent, char ***connectivity);
void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
void read_zones(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone);
void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

//connectivity.c
void generate_connectivity(int n_variables, char **connectivity, int *maximum_order, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

//memory.c
int allocate_mesh(int n_variables, int n_nodes, struct NODE **node, int n_faces, struct FACE **face, int n_cells, struct CELL **cell, int n_zones, struct ZONE **zone);
int allocate_instructions(int n_variables, int **maximum_order, double **weight_exponent, char ***connectivity);
void free_mesh(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void free_instructions(int n_variables, int *maximum_order, double *weight_exponent, char **connectivity);
int allocate_integer_vector(int **vector, int length);
int allocate_integer_zero_vector(int **vector, int length);
int allocate_double_vector(double **vector, int length);
int allocate_character_vector(char **vector, int length);
int allocate_integer_matrix(int ***matrix, int height, int width);
int allocate_integer_zero_matrix(int ***matrix, int height, int width);
int allocate_double_matrix(double ***matrix, int height, int width);
int allocate_character_matrix(char ***matrix, int height, int width);
void free_vector(void *vector);
void free_matrix(void **matrix);

//geometry.c
void generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void generate_control_volume_polygon(double **polygon, int index, int location, struct FACE *face, struct CELL *cell);
void calculate_polygon_centroid(int n, double **polygon, double *centroid);

//numerics.c
void calculate_cell_reconstruction_matrices(int n_variables, double *weight_exponent, int *maximum_order, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone);
int least_squares(int m, int n, double **matrix);
int constrained_least_squares(int m, int n, double **matrix, int c, int *constrained);
double integer_power(double base, int exp);

//fetch.c
void **fetch_allocate(char *format, int max_n_lines);
int fetch_read(FILE *file, char *label, char *format, int max_n_lines, void **data);
void fetch_get(char *format, void **data, int line_index, int value_index, void *value);
void fetch_print(char *format, int n_lines, void **data);
void fetch_free(char *format, int max_n_lines, void **data);

////////////////////////////////////////////////////////////////////////////////

