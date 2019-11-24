//
// Created by nik on 11/23/19.
//

#ifndef AGAME_UTIL_H
#define AGAME_UTIL_H

#include <stdlib.h>
#include <math.h>

typedef struct coord {
	int x;
	int y;
} coord_t;

extern int bresenham(int x0, int y0, int x1, int y1, char const* level, int width, int boundary_block);

extern coord_t bresenham_next(int x0, int y0, int x1, int y1, char const* level, int width, int boundary_block);

float dist_to(int sx, int sy, int dx, int dy);

#endif //AGAME_UTIL_H
