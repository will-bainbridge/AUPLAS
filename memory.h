#ifndef MEMORY_H
#define MEMORY_H

struct NODE * nodes_new(int n_nodes, struct NODE *node);
void nodes_destroy(int n_nodes, struct NODE *node);

struct FACE * faces_new(int n_faces, struct FACE *face);
int face_node_new(struct FACE *face);
int face_border_new(struct FACE *face);
int face_oriented_new(struct FACE *face);
int face_zone_new(struct FACE *face);
int face_zone_add(struct FACE *face, struct ZONE *zone);
void faces_destroy(int n_faces, struct FACE *face);

struct CELL * cells_new(int n_cells, struct CELL *cell);
int cell_face_new(struct CELL *cell);
int cell_oriented_new(struct CELL *cell);
int cell_zone_new(struct CELL *cell);
int cell_zone_add(struct CELL *cell, struct ZONE *zone);
int cell_order_new(int n_variables, struct CELL *cell);
int cell_n_stencil_new(int n_variables, struct CELL *cell);
int cell_stencil_new(int n_variables, struct CELL *cell);
int cell_matrix_new(int n_variables, struct CELL *cell);
void cells_destroy(int n_variables, int n_cells, struct CELL *cell);

struct ZONE * zones_new(int n_zones, struct ZONE *zone);
void zones_destroy(struct ZONE *zone);

struct DIVERGENCE * divergences_new(int n_divergences, struct DIVERGENCE *divergence);
void divergences_destroy(int n_divergences, struct DIVERGENCE *divergence);

struct ACCUMULATION * accumulations_new(int n_accumulations, struct ACCUMULATION *accumulation);
void accumulations_destroy(int n_accumulations, struct ACCUMULATION *accumulation);

int allocate_integer_vector(int **vector, int length);
int allocate_integer_zero_vector(int **vector, int length);
int allocate_double_vector(double **vector, int length);
int allocate_character_vector(char **vector, int length);
int allocate_integer_matrix(int ***matrix, int height, int width);
int allocate_integer_zero_matrix(int ***matrix, int height, int width);
int allocate_double_matrix(double ***matrix, int height, int width);
int allocate_double_pointer_matrix(double ****matrix, int height, int width);
int allocate_character_matrix(char ***matrix, int height, int width);
int allocate_cell_pointer_matrix(struct CELL ****matrix, int height, int width);
void free_vector(void *vector);
void free_matrix(void **matrix);

#endif
