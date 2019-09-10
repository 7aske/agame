#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <stdint.h>

#define S_HEIGHT 15
#define S_WIDTH 20

#define BSIZE 16
const int WIDTH = 640;
const int HEIGHT = 480;

typedef struct player {
	uint32_t x;
	uint32_t y;
} player_t;

char test[30][40] = {
		"########################################",
		"#                                      #",
		"# ##### ############# ################ #",
		"# ##          ####### #####       #### #",
		"# ##   .      ####### #####       #### #",
		"#           .         ##### .     #### #",
		"# ##          ####### #####       #### #",
		"# ##          ####### #####       #### #",
		"# ##### ############# ######## ####### #",
		"# ###      ########## ######## ####### #",
		"# ###      ########## ######## ####### #",
		"# ###      ########## ######## ####### #",
		"# ###      ########## ######## ####### #",
		"# ##### ######         .       . ##### #",
		"# #####         .                ##### #",
		"# ############              .    ##### #",
		"# #                              ##### #",
		"# ##### ######                   ##### #",
		"# ##### ######    .      .       ##### #",
		"# ##### ######                   ##### #",
		"# ##### ################# ############ #",
		"# ##### ################# ############ #",
		"# ##### #############      .  ######## #",
		"# ####     ##########         ######## #",
		"# #### .   ########## .       ######## #",
		"# ####     ##########     .   ######## #",
		"# ###################         ######## #",
		"# #################################### #",
		"#                                      #",
		"########################################",
};
volatile __sig_atomic_t running = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

player_t player = {1, 1};


void sig_handler(int sig) {
	if (sig == SIGINT) {
		running = 0;
	}
}

void sdl_error() {
	fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(1);
}

void sdl_quit() {
	running = 0;
}

void sdl_event_handler(SDL_Event* ev) {
	printf("%d\n", ev->key.keysym.scancode);
	switch (ev->type) {
		case SDL_QUIT:
			sdl_quit();
			break;
		case SDL_KEYDOWN:
			switch (ev->key.keysym.scancode) {
				case SDL_SCANCODE_W:
				case SDL_SCANCODE_UP:
					if (test[player.y - 1][player.x] != '#') {
						player.y -= 1;
					}
					break;
				case SDL_SCANCODE_A:
				case SDL_SCANCODE_LEFT:
					if (test[player.y][player.x - 1] != '#') {
						player.x -= 1;
					}
					break;
				case SDL_SCANCODE_S:
				case SDL_SCANCODE_DOWN:
					if (test[player.y + 1][player.x] != '#') {
						player.y += 1;
					}
					break;
				case SDL_SCANCODE_D:
				case SDL_SCANCODE_RIGHT:
					if (test[player.y][player.x + 1] != '#') {
						player.x += 1;
					}
					break;
			}
			break;
	}
}


int main(int argc, char* args[]) {
	signal(SIGINT, sig_handler);
	//The window we'll be rendering to

	//The surface contained by the window
	SDL_Surface* surface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdl_error();

	//Create window
	window = SDL_CreateWindow("A Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH,
							  HEIGHT, SDL_WINDOW_SHOWN);
	if (!window)
		sdl_error();

	renderer = SDL_CreateRenderer(window, -1, render_flags);

	if (!renderer)
		sdl_error();

	surface = IMG_Load("res/0x72_16x16DungeonTileset.v1.png");
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!tex)
		sdl_error();


	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			sdl_event_handler(&event);
		}

		SDL_RenderClear(renderer);

		int xoff = (int) (player.x / S_WIDTH) * S_WIDTH;
		int yoff = (int) (player.y / S_HEIGHT) * S_HEIGHT;
		printf("%d %d\n", xoff, yoff);

		for (int y = 0; y < S_HEIGHT; ++y) {
			for (int x = 0; x < S_WIDTH; ++x) {
				SDL_Rect dest;
				dest.h = BSIZE * 2;
				dest.w = BSIZE * 2;
				dest.x = BSIZE * 2 * x;
				dest.y = BSIZE * 2 * y;
				SDL_Rect src;
				src.h = BSIZE;
				src.w = BSIZE;
				switch (test[y + yoff][x + xoff]) {
					case '#':
						src.x = 16;
						src.y = 16;
						break;
					case ' ':
						src.x = 4 * 16;
						src.y = 7 * 16;
						break;

				}
				SDL_RenderCopy(renderer, tex, &src, &dest);

				// switch (test[y + yoff][x + xoff]){
				// 	case '.':
				// 		src.x = 0;
				// 		src.y = 5 * 16;
				// 		src.h = 4;
				// 		src.w = 4;
				// 		dest.x = BSIZE * 2 * x + 4;
				// 		dest.y = BSIZE * 2 * y + 4;
				// 		dest.h = BSIZE;
				// 		dest.w = BSIZE;
				// 		SDL_RenderCopy(renderer, tex, &src, &dest);
				// 		dest.h = BSIZE * 2;
				// 		dest.w = BSIZE * 2;
				// 		dest.x = BSIZE * 2 * x;
				// 		dest.y = BSIZE * 2 * y;
				// 		src.h = BSIZE;
				// 		src.w = BSIZE;
				// 		break;
				// }


				if (x + xoff == player.x && y + yoff == player.y) {
					src.x = 0;
					src.y = 4 * 16;
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}

			}
		}

		SDL_RenderPresent(renderer);

		//Update the surface
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / 60);
	}

	return 0;
}