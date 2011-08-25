#ifndef ZONE_H
#define ZONE_H

#define MAX_ZONES 100
#define MAX_CONDITION_CHARACTERS 8

struct FACE;
struct CELL;

struct ZONE
{
	char location;
	int variable;
	char condition[MAX_CONDITION_CHARACTERS];
	double value;
};

struct ZONE * zones_new(int n_zones, struct ZONE *zone);
void zones_input(char *filename, int n_faces, struct FACE *face, int n_cells, struct CELL *cell, int *n_zones, struct ZONE **zone);
void zone_case_write(FILE *file, struct ZONE *zone);
void zone_case_get(FILE *file, struct ZONE *zone);
void zones_destroy(struct ZONE *zone);

#endif
