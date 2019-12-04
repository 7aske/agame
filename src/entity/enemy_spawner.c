//
// Created by nik on 12/5/19.
//

#include "entity/enemy_spawner.h"

int spawner_spawn(entity_t* e, state_t* state) {
	assert(e != NULL && state != NULL);
	assert(e->type == E_ENEMY_SPAWNER);
	if (E_DEF_MAX_ENEMIES(state->level_count) <= state->current_enemies) {
		return 0;
	}
	if (e->espawner.next_spawn > 0) {
		e->espawner.next_spawn -= e->espawner.rate;
		return 0;
	} else {
		e->espawner.next_spawn = E_DEF_NEXT_SPAWN;
	}
	int x, y;
	while (state->level.maze[(y = rand() % state->level.h) * state->level.w + (x = rand() % state->level.w)] !=
		   B_FLOOR);
	ev_enemy_spawn(state, x, y);
	return 1;
}

entity_t spawner_new() {
	entity_t e;
	e.x = 0;
	e.y = 0;
	e.hp = 1000;
	e.type = E_ENEMY_SPAWNER;
	e.espawner.rate = 3;
	e.espawner.next_spawn = E_DEF_NEXT_SPAWN;
	return e;
}
