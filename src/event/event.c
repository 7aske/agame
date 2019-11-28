//
// Created by nik on 11/25/19.
//


#include "event/event.h"

void ev_level_start(state_t* state, ...) {

}

void ev_level_restart(state_t* state, ...) {
	maze_clear(&state->level);
	maze_new(&state->level);
	alist_clear(state->light_emitters);

	entity_t source;
	for (int y = 0; y < state->level.h; ++y) {
		for (int x = 0; x < state->level.w; ++x) {
			if (state->level.doodads[y * state->level.w + x] == D_TORCH) {
				printf("%d %d\n", x, y);
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
	event_dispatch(state, ev_enemies_spawn);
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

}

void ev_enemies_spawn(state_t* state, ...) {
	assert(state != NULL);
	assert(state->enemies != NULL);
	assert(state->level.maze != NULL);
	alist_clear(state->enemies);
	#define ENEMY_COUNT 30
	entity_t enemy;
	int i, dx, dy;
	for (i = 0; i < ENEMY_COUNT; ++i) {
		while (state->level.maze[(dy = rand() % state->level.h) * state->level.w + (dx = rand() % state->level.w)] !=
			   B_FLOOR);
		enemy = enemy_new(0, 0);
		enemy.x = dx;
		enemy.y = dy;
		enemy_search(&enemy, &state->player, state->level.maze, state->level.w, state->level.h, state->level.b_wall, 1);
		alist_add(state->enemies, &enemy);
	}
}

void ev_score_reset(state_t* state, ...) {
	state->score = 0;
}

void ev_score_incr(state_t* state, ...) {
	state->score++;
}

void event_dispatch(state_t* state, void (* callback)(state_t*, ...)) {
	event_t ev;
	ev.callback = callback;
	queue_enqueue(state->event_queue, &ev);
}
