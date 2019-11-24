//
// Created by nik on 11/23/19.
//
#include "entity.h"


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
		default:
			dx = 0;
			dy = 0;
			break;
	}

	if (lvl[(e->y + dy) * width + (e->x + dx)] != bound) {
		e->y += dy;
		e->x += dx;
		return 1;
	}
	return 0;
}


int pew_move(entity_t* e, char const* lvl, int width, int bound) {
	assert(e != (void*) 0);
	assert(e->type == E_PEW);
	return entity_move(e, lvl, width, bound, e->pew.dir);
}

entity_t enemy_new(int x, int y) {
	entity_t newenemy;
	newenemy.x = x;
	newenemy.y = y;
	newenemy.type = E_ENEMY;
	newenemy.enemy.next_move = 60;
	newenemy.hp = 100;
	return newenemy;
}


void enemy_randmove(entity_t* e, char const* lvl, int width, int bound) {
	assert(e != (void*) 0);
	assert(e->type == E_ENEMY);

	if (e->enemy.next_move > 0) {
		e->enemy.next_move -= (rand() % 3) + 1;
		return;
	} else {
		e->enemy.next_move = 60;
	}
	enum dir rand_dir = rand() % 4;
	entity_move(e, lvl, width, bound, rand_dir);
}

void player_shoot(entity_t* e, alist_t* entities) {
	assert(e != (void*) 0);
	assert(e->type == E_PLAYER);

	entity_t newpew;
	newpew.x = e->x;
	newpew.y = e->y;
	newpew.hp = 1;
	newpew.type = E_PEW;
	newpew.pew.dir = e->player.dir;
	newpew.pew.dmg = e->player.dmg;
	alist_add(entities, &newpew);
}

void enemy_lockmove(entity_t* e, entity_t* e1, char const* lvl, int width, int bound) {
	assert(e != (void*) 0);
	assert(e->type == E_ENEMY);

	int dx, dy;
	enum dir dir;
	dx = e->x - e1->x;
	dy = e->y - e1->y;
	if (e->enemy.next_move > 0) {
		e->enemy.next_move -= (rand() % 3) + 1;
		return;
	} else {
		e->enemy.next_move = 60;
	}

	if (abs(dx) >= abs(dy)) {
		if (dx >= 0) {
			dir = DIR_LEFT;
		} else {
			dir = DIR_RIGHT;
		}
	} else {
		if (dy > 0) {
			dir = DIR_UP;
		} else {
			dir = DIR_DOWN;
		}
	}

	if (!entity_move(e, lvl, width, bound, dir)) {
		if (dir == DIR_RIGHT || dir == DIR_LEFT) {
			if (dy > 0) {
				dir = DIR_UP;
			} else {
				dir = DIR_DOWN;
			}
			entity_move(e, lvl, width, bound, dir);
		} else {
			if (dx >= 0) {
				dir = DIR_LEFT;
			} else {
				dir = DIR_RIGHT;
			}
			entity_move(e, lvl, width, bound, dir);
		}
	}
}
