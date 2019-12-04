//
// Created by nik on 11/25/19.
//


#include "event/event.h"

void ev_level_start(state_t* state, ...) {

}

void ev_level_restart(state_t* state, ...) {
	int y, x;
	maze_clear(&state->level);
	maze_new(&state->level);
	alist_clear(state->light_emitters);

	entity_t source;
	for (y = 0; y < state->level.h; ++y) {
		for (x = 0; x < state->level.w; ++x) {
			if (state->level.doodads[y * state->level.w + x] == D_TORCH) {
				source.x = x;
				source.y = y;
				source.light.intensity = 1.0f;
				source.type = E_LIGHT;
				alist_add(state->light_emitters, &source);
			}
		}
	}
	while (!queue_isempty(state->event_queue)) { queue_dequeue(state->event_queue); }
	state->player.x = 1;
	state->player.y = 1;
	ev_enemies_destroy(state);

	overlay_solution(state->level.maze, state->level.exit_x, state->level.exit_y);
}

void ev_level_next(state_t* state, ...) {
	state->level_count++;
	ev_level_restart(state);
}

void ev_game_restart(state_t* state, ...) {
	ev_level_restart(state);
	state->level_count = 0;
	state->score = 0;
}

void ev_enemy_spawn(state_t* state, ...) {
	va_list argp;
	int x, y;
	entity_t e;
	va_start(argp, state);
	x = va_arg(argp, int);
	y = va_arg(argp, int);
	assert(x > -1 && x < state->level.w);
	assert(y > -1 && y < state->level.h);
	e = enemy_new(0, 0);
	e.x = x;
	e.y = y;
	enemy_search(&e, &state->player, state->level.maze, state->level.w, state->level.h, state->level.b_wall, 1);
	alist_add(state->entities, &e);
	state->current_enemies++;
	va_end(argp);
}

void ev_enemies_destroy(state_t* state, ...) {
	assert(state != NULL);
	assert(state->level.maze != NULL);
	assert(state->entities != NULL);
	entity_t* eptr;
	int i;
	// clear existing enemies
	for (i = 0; i < alist_size(state->entities); ++i) {
		eptr = alist_get(state->entities, i);
		if (eptr->type == E_ENEMY) {
			if (eptr->enemy.path != NULL) {
				stack_destroy(eptr->enemy.path);
			}
			alist_rm_idx(state->entities, i--);
		}
	}
	state->current_enemies = 0;
}

void ev_score_reset(state_t* state, ...) {
	state->score = 0;
}

void ev_score_incr(state_t* state, ...) {
	state->score++;
	state->current_enemies--;
}

void event_dispatch(state_t* state, void (* callback)(state_t*, ...)) {
	event_t ev;
	ev.callback = callback;
	queue_enqueue(state->event_queue, &ev);
}
