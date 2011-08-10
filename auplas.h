////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//return values
#define SUCCESS 1
#define ERROR -1

//maximum extents for memory allocation
#define MAX_CELL_FACES 5
#define MAX_NEIGHBOURS 10
#define MAX_STENCIL 100
#define MAX_ZONES 100
#define MAX_STRING_CHARACTERS 128

//cell order to number of powers
#define ORDER_TO_POWERS(order) ((order+1.0)*(order+2.0)*0.5)

//min and max macros
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

//accuracy
#define EPS 1e-15

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
	struct NODE *node[2];

	struct CELL **border;
	int *oriented;
	int n_borders;

	struct ZONE **zone;
	int n_zones;

	double centroid[2];
};

struct CELL
{
	struct FACE **face;
	int *oriented;
	int n_faces;

	struct ZONE **zone;
	int n_zones;

	double centroid[2];

	int **stencil;
	int *n_stencil;

	int *order;

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

int read_instructions(char *filename, int *n_variables, char **geometry_filename, int **maximum_order, double **weight_exponent, char ***connectivity);
int read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
int read_zones(char *filename, struct FACE *face, struct CELL *cell, int *n_zones, struct ZONE **zone);

int generate_connectivity(int n_variables, char **connectivity, int *maximum_order, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

void free_mesh_structures(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

int generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
int calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void generate_control_volume_polygon(double **polygon, int index, int location, struct FACE *face, struct CELL *cell);
void calculate_polygon_centroid(int n, double **polygon, double *centroid);

int calculate_cell_reconstruction_matrices(int n_variables, double *weight_exponent, int *maximum_order, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone);
int least_squares(int m, int n, double **matrix);
int constrained_least_squares(int m, int n, double **matrix, int c, int *constrained);
double ipow(double base, int exp);

////////////////////////////////////////////////////////////////////////////////
