////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "csr.h"
#include "divergence.h"
#include "handle.h"
#include "mesh.h"
#include "zone.h"

////////////////////////////////////////////////////////////////////////////////

//error handlind return values
#define ALLOCATE_SUCCESS 1
#define ALLOCATE_ERROR 0

//cell order to number of powers
#define ORDER_TO_POWERS(order) ((order+1.0)*(order+2.0)*0.5)

//min and max macros
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a < b) ? a : b)

//id macros
#define ID_TO_ZONE(id) ((id) % MAX_ZONES)
#define ID_TO_INDEX(id) ((id) / MAX_ZONES)
#define INDEX_AND_ZONE_TO_ID(i,z) ((i)*MAX_ZONES + (z))

//string length
#define MAX_STRING_LENGTH 128

////////////////////////////////////////////////////////////////////////////////

//io.c
void read_geometry(char *filename, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell);
void write_case(char *filename, int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void read_case(char *filename, int *n_variables, int *n_nodes, struct NODE **node, int *n_faces, struct FACE **face, int *n_cells, struct CELL **cell, int *n_zones, struct ZONE **zone);

//connectivity.c
void generate_connectivity(int n_variables, char **connectivity, int *maximum_order, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);

//memory.c
int allocate_mesh(int n_variables, int n_nodes, struct NODE **node, int n_faces, struct FACE **face, int n_cells, struct CELL **cell, int n_zones, struct ZONE **zone);
void free_mesh(int n_variables, int n_nodes, struct NODE *node, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
int allocate_integer_vector(int **vector, int length);
int allocate_integer_zero_vector(int **vector, int length);
int allocate_double_vector(double **vector, int length);
int allocate_character_vector(char **vector, int length);
int allocate_integer_matrix(int ***matrix, int height, int width);
int allocate_integer_zero_matrix(int ***matrix, int height, int width);
int allocate_double_matrix(double ***matrix, int height, int width);
int allocate_double_pointer_matrix(double ****matrix, int height, int width);
int allocate_character_matrix(char ***matrix, int height, int width);
void free_vector(void *vector);
void free_matrix(void **matrix);

//geometry.c
void generate_face_orientations(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void calculate_control_volume_geometry(int n_faces, struct FACE *face, int n_cells, struct CELL *cell);
void generate_control_volume_polygon(double ***polygon, int index, int location, struct FACE *face, struct CELL *cell);
void calculate_polygon_centroid(int n, double ***polygon, double *centroid);

//numerics.c
void calculate_cell_reconstruction_matrices(int n_variables, double *weight_exponent, int *maximum_order, struct FACE *face, int n_cells, struct CELL *cell, struct ZONE *zone);
int least_squares(int m, int n, double **matrix);
int constrained_least_squares(int m, int n, double **matrix, int c, int *constrained);
double integer_power(double base, int exp);

//system.c
void generate_system_lists(int *n_ids, int **id_to_unknown, int *n_unknowns, int **unknown_to_id, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone);
void assemble_matrix(CSR matrix, int n_ids, int *id_to_unknown, int n_unknowns, int *unknown_to_id, double *lhs, double *rhs, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int n_zones, struct ZONE *zone, int n_divergences, DIVERGENCE *divergence);
void calculate_divergence(int n_polygon, double ***polygon, int *n_interpolant, struct CELL ***interpolant, int *id_to_unknown, double *lhs, double *rhs, double *row, struct ZONE *zone, DIVERGENCE divergence);

////////////////////////////////////////////////////////////////////////////////
