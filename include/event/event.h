//
// Created by nik on 11/25/19.
//

#ifndef AGAME_EVENT_H
#define AGAME_EVENT_H

#pragma once

#include "state.h"
#include "entity/enemy.h"

typedef struct event {
	void (* callback)(state_t*, ...);
} event_t;

void ev_score_incr(state_t* state, ...);

void ev_score_reset(state_t* state, ...);

void ev_enemies_spawn(state_t* state, ...);

void ev_enemy_spawn(state_t* state, ...);

void ev_level_restart(state_t* state, ...);

void ev_level_start(state_t* state, ...);

void ev_level_next(state_t* state, ...);

void ev_game_restart(state_t* state, ...);

void event_dispatch(state_t* state, void (* callback)(state_t*, ...));

#endif //AGAME_EVENT_H
