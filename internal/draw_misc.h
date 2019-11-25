//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_DRAW_MISC_H
#define SDLGAME_DRAW_MISC_H

#define LINE_H 16
#define CHAR_W 8

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "macro_definitions.h"
#include "cleanup_functions.h"


extern void draw_text(SDL_Renderer* renderer, TTF_Font* font, char const* text, int x, int y, SDL_Color* c) {
	assert(renderer != (void*) 0);
	assert(font != (void*) 0);
	assert(text != (void*) 0);
	SDL_Rect msg_rect;
	SDL_Color color = {255, 255, 255, 255};

	if (c != NULL) {
		color = *c;
	}

	SDL_Surface* msg_surf __attribute__((__cleanup__(_sdlfs))) = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* msg_tex __attribute__((__cleanup__(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf);

	msg_rect.x = x;
	msg_rect.y = y;
	msg_rect.w = (int) strnlen(text, WIDTH) * CHAR_W;
	msg_rect.h = LINE_H;
	SDL_RenderCopy(renderer, msg_tex, NULL, &msg_rect);
}

extern void draw_help(SDL_Renderer* renderer, TTF_Font* font) {

	static const char
			* const line1 = "O - maze_solve maze",
			* const line2 = "L - toggle light",
			* const line3 = "R - generate new maze",
			* const line4 = "Q - quit game",
			* const line5 = "Spc - shoot";

	SDL_Color color = {255, 255, 255, 128};

	draw_text(renderer, font, line1, 5, HEIGHT - 1 * LINE_H, &color);
	draw_text(renderer, font, line2, 5, HEIGHT - 2 * LINE_H, &color);
	draw_text(renderer, font, line3, 5, HEIGHT - 3 * LINE_H, &color);
	draw_text(renderer, font, line4, 5, HEIGHT - 4 * LINE_H, &color);
	draw_text(renderer, font, line5, 5, HEIGHT - 5 * LINE_H, &color);
}

void draw_fps(SDL_Renderer* renderer, TTF_Font* font, int const* fps) {
	char buf[32];
	snprintf(buf, 31, "FPS %d", *fps);
	draw_text(renderer, font, buf, 5, 5, NULL);
}

#endif //SDLGAME_DRAW_MISC_H
