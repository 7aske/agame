//
// Created by nik on 11/25/19.
//

#ifndef SDLGAME_STATE_H
#define SDLGAME_STATE_H

#pragma once

#include "entity/entity.h"
#include "maze.h"

typedef enum render_mode {
	REN_BLOCKS,
	REN_ENTITIES,
	REN_SOLUTION,
	REN_ALL
} ren_mode_e;

typedef enum lights {
	L_NONE,
	L_LOS,
	L_AREA,
	L_ALL
} light_e;

typedef struct state {
	entity_t player;

	alist_t* light_emitters;
	alist_t* entities;
	queue_t* event_queue;

	maze_t level;

	int level_count;
	int score;
	int current_enemies;

	light_e light;
	ren_mode_e ren_mode;
} state_t;

void state_change_ren(state_t* state, int step);

void state_change_light(state_t* state, int step);

#endif //SDLGAME_STATE_H
