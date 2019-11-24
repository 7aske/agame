//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_CLEANUP_FUNCTIONS_H
#define SDLGAME_CLEANUP_FUNCTIONS_H

#pragma once

#include <SDL2/SDL.h>

void __attribute__((used)) _sdldt(SDL_Texture** tex)  {
	SDL_DestroyTexture(*tex);
}

void __attribute__((used)) _sdlfs(SDL_Surface** surf) {
	SDL_FreeSurface(*surf);
}

void __attribute__((used)) _afree(void** ptr) {
	free(*ptr);
}

#endif //SDLGAME_CLEANUP_FUNCTIONS_H

