//
// Created by nik on 11/23/19.
//

#ifndef AGAME_UTIL_H
#define AGAME_UTIL_H

#include <stdlib.h>
#include <math.h>

extern int bresenham(int x0, int y0, int x1, int y1, char const* lvl, int lvl_w, int b_wall);

float dist_to(int sx, int sy, int dx, int dy);

#endif //AGAME_UTIL_H
