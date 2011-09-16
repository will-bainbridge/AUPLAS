////////////////////////////////////////////////////////////////////////////////

//cell order to number of powers
#define ORDER_TO_POWERS(order) ((order+1)*(order+2)/2)

//min and max macros
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

//id macros
#define ID_TO_ZONE(id) ((id) % MAX_ZONES)
#define ID_TO_INDEX(id) ((id) / MAX_ZONES)
#define INDEX_AND_ZONE_TO_ID(i,z) ((i)*MAX_ZONES + (z))

//maximums
#define MAX_STRING_LENGTH 128
#define MAX_INDICES 100

#define MAX_ZONES 100
#define MAX_CONDITION_LENGTH 8

#define MAX_FACE_NODES 2
#define MAX_CELL_FACES 5

#define MAX_STENCIL 100

////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "csr.h"
#include "expression.h"
#include "handle.h"

struct NODE
{
	double x[2];

	int n_borders;
	struct CELL **border;
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

	double area;
	double centroid[2];
};

struct CELL
{
	int n_faces;
	struct FACE **face;
	int *oriented;

	int n_zones;
	struct ZONE **zone;

	double area;
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
	char condition[MAX_CONDITION_LENGTH];
	double value;
};

struct DIVERGENCE
{
	int equation;
	int n_variables;
	int *variable;
	int *differential;
	int *power;
	int direction;
	double implicit;
	EXPRESSION constant;
};

#include "memory.h"

////////////////////////////////////////////////////////////////////////////////

//io.c
void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void read_case(char *filename, int *n_variables, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell, int *n_zones, struct ZONE **zone);
void zones_input(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone);
void divergences_input(char *filename, char *constant, int *n_divergences, struct DIVERGENCE **divergence);
void write_data(char *basename, double time, int n_data, double *data);
void read_data(char *filename, double *time, int n_data, double *data);
void write_gnuplot(char *basename, double time, int n_variables, char **variable_name, int n_unknowns, int *unknown_to_id, double *x, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void write_vtk(char *basename, double time, int n_variables, char **variable_name, int n_unknowns, int *unknown_to_id, double *x, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

//connectivity.c
void generate_borders(int n_cells, struct CELL *cell);
void generate_stencils(int n_variables, char **connectivity, int *maximum_order, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

//geometry.c
void generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
int generate_control_volume_polygon(double ***polygon, int index, int location, struct FACE *face, struct CELL *cell);
void calculate_polygon_area_and_centroid(int n, double ***polygon, double *area, double *centroid);

//numerics.c
void calculate_cell_reconstruction_matrices(int n_variables, double *weight_exponent, int *maximum_order, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone);
int least_squares(int m, int n, double **matrix);
int constrained_least_squares(int m, int n, double **matrix, int c, int *constrained);
double integer_power(double base, int exp);
void evaluate_polynomial(double *polynomial, int order, int differential, double *x);

//system.c
void generate_lists_of_unknowns(int *n_unknowns, int **unknown_to_id, int n_variables, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void assemble_matrix(CSR matrix, int n_variables, int n_unknowns, int *unknown_to_id, struct FACE *face, struct CELL *cell, struct ZONE *zone);
void calculate_divergence(double *f_explicit, double *f_implicit, CSR jacobian, double *x, double coefficient, int n_variables, int n_unknowns, int *unknown_to_id, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone, int n_divergences, struct DIVERGENCE *divergence);
void initialise_unknowns(int n_unknowns, int *unknown_to_id, struct ZONE *zone, double *x);
void calculate_residuals(int n_variables, int n_unknowns, int *unknown_to_id, double *dx, double *x, double *residual, int n_zones, struct ZONE *zone);

////////////////////////////////////////////////////////////////////////////////
