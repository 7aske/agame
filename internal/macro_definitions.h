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

#define SOL_PATH 38
#define B_WALL 35
#define B_FLOOR 32

#define D_SKULL 1
#define D_PIPE1 2
#define D_PIPE2 3
#define D_OOZE 4
#define D_GRATE 5
#define D_BRICK 6


#endif //AGAME_MACRO_DEFINITIONS_H
