//
// Created by nik on 11/23/19.
//

#ifndef AGAME_MACRO_DEFINITIONS_H
#define AGAME_MACRO_DEFINITIONS_H

// screen size
#define WIDTH (1024)
#define HEIGHT (768)

// apparent block size on the screen
#define BSIZE (32)

// blocks count in the viewport
#define SCR_W (WIDTH / BSIZE)
#define SCR_H (HEIGHT / BSIZE)
#define WRLD_W (3)
#define WRLD_H (2)

#define LVL_W (WRLD_W * SCR_W)
#define LVL_H (WRLD_H * SCR_H)

enum blocks_e {
	B_NONE,
	B_PATH,
	B_WALL,
	B_FLOOR,
	B_EXIT
};

enum doodads_e {
	D_NONE,
	D_SKULL,
	D_PIPE1,
	D_PIPE2,
	D_OOZE,
	D_GRATE,
	D_BRICK,
	D_TORCH
};


#endif //AGAME_MACRO_DEFINITIONS_H
