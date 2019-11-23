//
// Created by nik on 11/23/19.
//

#include "util.h"

int bresenham(int x0, int y0, int x1, int y1, char const* level, int width, int boundary_block) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */

	for (;;) {
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
		if (x0 == x1 && y0 == y1) return 1;
		if (level[y0 * width + x0] == boundary_block) return 0;
	}
}


float dist_to(int sx, int sy, int dx, int dy) {
	return sqrtf(powf((float)sx - (float)dx, 2) + powf((float)sy - (float)dy, 2));
}

