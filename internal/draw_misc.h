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


void draw_fps(SDL_Renderer* renderer, TTF_Font* font, int const* fps) {
	char buf[32];
	snprintf(buf, 32, "fps %d", *fps);
	SDL_Color white = {255, 255, 255};

	SDL_Surface* msg_surf __attribute__((cleanup(_sdlfs))) = TTF_RenderText_Solid(font, buf, white);
	SDL_Texture* msg_tex __attribute__((cleanup(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf);
	SDL_Rect msg_rect;
	msg_rect.x = 5;
	msg_rect.y = 5;
	msg_rect.w = strlen(buf) * CHAR_W;
	msg_rect.h = LINE_H;

	SDL_RenderCopy(renderer, msg_tex, NULL, &msg_rect);
}


extern void draw_help(SDL_Renderer* renderer, TTF_Font* font) {

	static const char
			* const line1 = "O - solve maze",
			* const line2 = "L - toggle light",
			* const line3 = "R - generate new maze",
			* const line4 = "Q - quit game";

	SDL_Color white = {255, 255, 255};

	SDL_Surface* msg_surf1 __attribute__((__cleanup__(_sdlfs))) = TTF_RenderText_Solid(font, line1, white);
	SDL_Surface* msg_surf2 __attribute__((__cleanup__(_sdlfs))) = TTF_RenderText_Solid(font, line2, white);
	SDL_Surface* msg_surf3 __attribute__((__cleanup__(_sdlfs))) = TTF_RenderText_Solid(font, line3, white);
	SDL_Surface* msg_surf4 __attribute__((__cleanup__(_sdlfs))) = TTF_RenderText_Solid(font, line4, white);
	SDL_Texture* msg_tex1 __attribute__((__cleanup__(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf1);
	SDL_Texture* msg_tex2 __attribute__((__cleanup__(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf2);
	SDL_Texture* msg_tex3 __attribute__((__cleanup__(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf3);
	SDL_Texture* msg_tex4 __attribute__((__cleanup__(_sdldt))) = SDL_CreateTextureFromSurface(renderer, msg_surf4);

	SDL_Rect msg_rect;

	msg_rect.x = 5;
	msg_rect.y = HEIGHT - 1 * LINE_H;
	msg_rect.w = strlen(line1) * CHAR_W;
	msg_rect.h = LINE_H;
	SDL_RenderCopy(renderer, msg_tex1, NULL, &msg_rect);

	msg_rect.x = 5;
	msg_rect.y = HEIGHT - 2 * LINE_H;
	msg_rect.w = strlen(line2) * CHAR_W;
	msg_rect.h = LINE_H;
	SDL_RenderCopy(renderer, msg_tex2, NULL, &msg_rect);

	msg_rect.x = 5;
	msg_rect.y = HEIGHT - 3 * LINE_H;
	msg_rect.w = strlen(line3) * CHAR_W;
	msg_rect.h = LINE_H;
	SDL_RenderCopy(renderer, msg_tex3, NULL, &msg_rect);

	msg_rect.x = 5;
	msg_rect.y = HEIGHT - 4 * LINE_H;
	msg_rect.w = strlen(line4) * CHAR_W;
	msg_rect.h = LINE_H;
	SDL_RenderCopy(renderer, msg_tex4, NULL, &msg_rect);
}



#endif //SDLGAME_DRAW_MISC_H
