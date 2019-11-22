#ifndef AGAME_SPRITES_H
#define AGAME_SPRITES_H

#include <SDL2/SDL.h>

#define BIZE_SPR 16

typedef enum {
	SPR_PLAYER,
	SPR_FLOOR,
	SPR_WALL,
	SPR_TWALL,
	SPR_LFLOOR,
	SPR_TFLOOR,
	SPR_DFLOOR,
	SPR_COIN,
	SPR_ENEMY1,
	SPR_PIPE,
	SPR_PIPE2,
	SPR_MBRICK,
	SPR_GRATE,
	SPR_OOZE,
	SPR_OOZEF,
	SPR_SKULL,
} SPRITES;

void query_sprite(SPRITES spr, SDL_Rect* src) {
	src->w = BIZE_SPR;
	src->h = BIZE_SPR;
	switch (spr) {
		case SPR_PLAYER:
			src->x = 0;
			src->y = 4 * 16;
			break;
		case SPR_DFLOOR:
			src->x = 4 * 16;
			src->y = 7 * 16;
			break;
		case SPR_WALL:
			src->x = 16;
			src->y = 16;
			break;
		case SPR_LFLOOR:
			src->x = 4 * 16;
			src->y = 8 * 16;
			break;
		case SPR_COIN:
			src->x = 4 * 16;
			src->y = 9 * 16;
			break;
		case SPR_ENEMY1:
			src->x = 0;
			src->y = 6 * 16;
			break;
		case SPR_PIPE:
			src->x = 4 * 16;
			src->y = 0;
			break;
		case SPR_MBRICK:
			src->x = 4 * 16;
			src->y = 2 * 16;
			break;
		case SPR_TFLOOR:
			src->x = 0;
			src->y = 2 * 16;
			break;
		case SPR_FLOOR:
			src->x = 1 * 16;
			src->y = 2 * 16;
			break;
		case SPR_GRATE:
			src->x = 4 * 16;
			src->y = 1 * 16;
			break;
		case SPR_OOZE:
			src->x = 3 * 16;
			src->y = 1 * 16;
			break;
		case SPR_OOZEF:
			src->x = 3 * 16;
			src->y = 2 * 16;
			break;
		case SPR_TWALL:
			src->x = 0;
			src->y = 0;
			break;
		case SPR_PIPE2:
			src->x = 3 * 16;
			src->y = 0;
			break;
		case SPR_SKULL:
			src->x = 2 * 16;
			src->y = 2 * 16;
			break;
		default:
			src->x = 0;
			src->y = 0;
	}
}

#endif