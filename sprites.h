#ifndef AGAME_SPRITES_H
#define AGAME_SPRITES_H

#include <SDL2/SDL.h>

#define BIZE_SPR 16

typedef enum {
	SPR_PLAYER = 0,
	SPR_FLOOR = 1,
	SPR_WALL = 2,
	SPR_COIN = 3,
} SPRITES;

void query_sprite(SPRITES spr, SDL_Rect* src) {
	src->w = BIZE_SPR;
	src->h = BIZE_SPR;
	switch (spr) {
		case SPR_PLAYER:
			src->x = 0;
			src->y = 4 * 16;
			break;
		case SPR_FLOOR:
			src->x = 4 * 16;
			src->y = 7 * 16;
			break;
		case SPR_WALL:
			src->x = 16;
			src->y = 16;
			break;
		case SPR_COIN:
			src->x = 0;
			src->y = 5 *16;
			break;
		default:
			src->x = 0;
			src->y = 0;
	}
}

#endif