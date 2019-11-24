//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_ENTITY_H
#define SDLGAME_ENTITY_H

#pragma once

#include <assert.h>
#include <stdlib.h>
#include "structs/arraylist.h"
#include "util.h"

#define DEFAULT_HP 100
#define DEFAULT_NEXT_SEARCH 400
#define DEFAULT_NEXT_MOVE 60
#define DEFAULT_DMG 50

enum dir {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

enum entities {
	E_NONE,
	E_PLAYER,
	E_LIGHT,
	E_ENEMY,
	E_PEW,
};

typedef struct player {
	enum dir dir;
	int dmg;
} player_t;

typedef struct light {
	float intensity;
} light_t;

typedef struct enemy {
	int next_move;
	int next_search;
	astack_t* path;
} enemy_t;

typedef struct pew {
	enum dir dir;
	int dmg;
	int speed;
} pew_t;


typedef struct entity {
	int x;
	int y;
	int hp;
	union {
		player_t player;
		enemy_t enemy;
		light_t light;
		pew_t pew;
	};
	enum entities type;
} entity_t;

extern int pew_move(entity_t* e, char const* lvl, int width, int bound);

extern void player_shoot(entity_t* e, alist_t* entities);

extern entity_t enemy_new(int x, int y);

extern int entity_move(entity_t* e, char const* lvl, int width, int bound, enum dir dir);

void enemy_search(entity_t* e, entity_t* tar, char const* level, int width, int height, int boundary, int force_search);

extern void enemy_randmove(entity_t* e, char const* lvl, int width, int bound);

void enemy_fpath(entity_t* e, char const* lvl, int width, int bound);

extern void enemy_lockmove(entity_t* e, entity_t* e1, char const* lvl, int width, int bound);

#endif //SDLGAME_ENTITY_H
