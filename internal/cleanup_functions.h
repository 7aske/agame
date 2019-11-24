//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_CLEANUP_FUNCTIONS_H
#define SDLGAME_CLEANUP_FUNCTIONS_H

#pragma once

#include <SDL2/SDL.h>

static void __attribute__((used)) _sdldt(SDL_Texture** tex)  {
	SDL_DestroyTexture(*tex);
}

static void __attribute__((used)) _sdlfs(SDL_Surface** surf) {
	SDL_FreeSurface(*surf);
}

static void __attribute__((used)) _afree(void** ptr) {
	free(*ptr);
}

#endif //SDLGAME_CLEANUP_FUNCTIONS_H

