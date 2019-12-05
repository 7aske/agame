//
// Created by nik on 11/28/19.
//

#include "state.h"


void state_change_ren(state_t* state, int step) {
	assert(state != (void*) 0);
	assert(step == 1 || step == -1);
	// REN_BLOCKS, REN_ENTITIES, REN_SOLUTION
	switch (state->ren_mode) {
		case REN_ENTITIES:
			if (step == 1) {
				state->ren_mode = REN_SOLUTION;
			} else {
				state->ren_mode = REN_BLOCKS;
			}
			break;
		case REN_SOLUTION:
			if (step == 1) {
				state->ren_mode = REN_BLOCKS;
			} else {
				state->ren_mode = REN_ENTITIES;
			}
			break;
		case REN_BLOCKS:
		case REN_ALL:
			if (step == 1) {
				state->ren_mode = REN_ENTITIES;
			} else {
				state->ren_mode = REN_SOLUTION;
			}
			break;
		default:
			state->ren_mode = REN_BLOCKS;
	}
}

void state_change_light(state_t* state, int step) {
	assert(state != (void*) 0);
	assert(step == 1 || step == -1);
	// L_NONE, L_LOS, L_AREA, L_ALL
	switch (state->light_mode) {
		case L_NONE:
			break;
		case L_LOS:
			if (step == 1) {
				state->light_mode = L_AREA;
			} else {
				state->light_mode = L_ALL;
			}
			break;
		case L_AREA:
			if (step == 1) {
				state->light_mode = L_ALL;
			} else {
				state->light_mode = L_LOS;
			}
			break;
		case L_ALL:
			if (step == 1) {
				state->light_mode = L_LOS;
			} else {
				state->light_mode = L_AREA;
			}
			break;
	}
}

char const* get_ren_mode(enum render_mode mode) {
	switch (mode) {
		case REN_BLOCKS:
			return "Blocks";
		case REN_ENTITIES:
			return "Entities";
		case REN_SOLUTION:
			return "Solution";
		case REN_ALL:
			return "All";
	}
}

char const* get_light_mode(enum lights light) {
	switch (light) {
		case L_NONE:
			return "None";
		case L_LOS:
			return "LOS";
		case L_AREA:
			return "Area";
		case L_ALL:
			return "All";
	}
}
