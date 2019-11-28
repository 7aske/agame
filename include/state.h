//
// Created by nik on 11/25/19.
//

#ifndef SDLGAME_STATE_H
#define SDLGAME_STATE_H

#pragma once

#include <entity/entity.h>
#include "maze.h"

typedef enum render_mode {
	REN_BLOCKS,
	REN_ENTIIES,
	REN_SOLUTION
} ren_mode_e;
typedef enum lights {
	L_NONE,
	L_LOS,
	L_NOLOS,
	L_DISABLED
} light_e;

typedef struct state {
	entity_t player;

	alist_t* light_emitters;
	alist_t* enemies;
	alist_t* entities;
	queue_t* event_queue;

	maze_t level;

	int level_count;
	int score;

	light_e l_sw;
	light_e l_type[3];

	ren_mode_e ren_mode;
} state_t;


#endif //SDLGAME_STATE_H
