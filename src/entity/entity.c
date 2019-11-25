//
// Created by nik on 11/23/19.
//
#include "entity/entity.h"

int entity_move(entity_t* e, char const* lvl, int width, int bound, enum dir dir) {
	assert(e != (void*) 0);
	assert(lvl != (void*) 0);

	int dx, dy;
	switch (dir) {
		case DIR_RIGHT:
			dx = 1;
			dy = 0;
			break;
		case DIR_LEFT:
			dx = -1;
			dy = 0;
			break;
		case DIR_DOWN:
			dx = 0;
			dy = 1;
			break;
		case DIR_UP:
			dx = 0;
			dy = -1;
			break;
		case DIR_NONE:
			dx = 0;
			dy = 0;
			break;
		default:
			dx = 0;
			dy = 0;
	}

	if (lvl[(e->y + dy) * width + (e->x + dx)] != bound) {
		e->y += dy;
		e->x += dx;
		return 1;
	}
	return 0;
}

