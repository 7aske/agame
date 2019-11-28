//
// Created by nik on 11/28/19.
//

#include "state.h"


void state_change_ren(state_t* state, int step) {
	assert(state != (void*) 0);
	assert(step == 1 || step == -1);
	// REN_BLOCKS, REN_ENTITIES, REN_SOLUTION
	switch (state->ren_mode) {
		case REN_BLOCKS:
			if (step == 1) {
				state->ren_mode = REN_ENTITIES;
			} else {
				state->ren_mode = REN_SOLUTION;
			}
			break;
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
		case REN_ALL:
			break;
		default:
			state->ren_mode = REN_BLOCKS;
	}
}

void state_change_light(state_t* state, int step) {
	assert(state != (void*) 0);
	assert(step == 1 || step == -1);
	// L_NONE, L_LOS, L_AREA, L_ALL
	switch (state->light) {
		case L_NONE:
			break;
		case L_LOS:
			if (step == 1) {
				state->light = L_AREA;
			} else {
				state->light = L_ALL;
			}
			break;
		case L_AREA:
			if (step == 1) {
				state->light = L_ALL;
			} else {
				state->light = L_LOS;
			}
			break;
		case L_ALL:
			if (step == 1) {
				state->light = L_LOS;
			} else {
				state->light = L_AREA;
			}
			break;
	}
}
