//
// Created by nik on 12/6/19.
//

#ifndef AGAME_GRAPH_H
#define AGAME_GRAPH_H

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <SDL2/SDL.h>

#include "util.h"

enum gnode_type {
	GNS = 1, // start
	GNE = 2, // end
	GNN = 3, // node
};

enum gnode_ntype {
	NTYPE_N,
	NTYPE_E,
	NTYPE_S,
	NTYPE_W,
	NTYPE_NE,
	NTYPE_NW,
	NTYPE_NS,
	NTYPE_ES,
	NTYPE_EW,
	NTYPE_SW,
	NTYPE_NES,
	NTYPE_NEW,
	NTYPE_NSW,
	NTYPE_ESW,
	NTYPE_NONE,
	NTYPE_ALL
};

struct gnode {
	int x;
	int y;
	float h; // heuristic
	int visited;
	int rendered;
	int ntype;
	enum gnode_type type;
	struct gnode* nodes[4];
};

struct mgraph {
	struct gnode* start;
	struct gnode* end;
};

static struct gnode* gnode_new(int x, int y, enum gnode_type type) {
	struct gnode* newgnode = (struct gnode*) malloc(sizeof(struct gnode));
	newgnode->type = type;
	newgnode->x = x;
	newgnode->y = y;
	newgnode->visited = 0;
	newgnode->rendered = 0;
	newgnode->ntype = NTYPE_NONE;
	newgnode->nodes[0] = NULL; // north
	newgnode->nodes[1] = NULL; // east
	newgnode->nodes[2] = NULL; // south
	newgnode->nodes[3] = NULL; // west
	return newgnode;
}


static struct mgraph* to_graph(char const* maze, int width, int height, char wall, int exit_x, int exit_y) {
	int curr, prev, next, x, y;
	struct mgraph* mgraph = (struct mgraph*) calloc(1, sizeof(struct mgraph));
	struct gnode* node = NULL,
			* top = NULL,
			* leftnode = NULL;
	// temporary top row of every search
	struct gnode** topnodes = (struct gnode**) calloc(width, sizeof(struct gnode*));

	for (y = 1; y < height - 1; ++y) {
		prev = wall;
		curr = wall;
		next = maze[y * width + 1];
		leftnode = NULL;
		for (x = 1; x < width - 1; ++x) {
			prev = curr;
			curr = next;
			next = maze[y * width + x + 1];
			node = NULL;

			if (curr == wall) {
				continue;
			}


			if (prev != wall) {
				if (next != wall) {
					// path path path
					if ((maze[(y - 1) * width + x] != wall) || (maze[(y + 1) * width + x] != wall)) {
						node = gnode_new(x, y, GNN);
						leftnode->nodes[1] = node;// left node east -> node
						node->nodes[3] = leftnode; // nod west -> left node
						leftnode = node;
					}
				} else {
					// path path wall
					node = gnode_new(x, y, GNN);
					leftnode->nodes[1] = node;
					node->nodes[3] = leftnode;
					leftnode = node;
				}

			} else {
				// wall path path
				if (next != wall) {
					node = gnode_new(x, y, GNN);
					leftnode = node;
				} else {
					// wall path wall
					if ((maze[(y - 1) * width + x] == wall) || (maze[(y + 1) * width + x] == wall)) {
						node = gnode_new(x, y, GNN);
					} else {
						if (x == exit_x && y == exit_y) {
							node = gnode_new(x, y, GNE);
							leftnode = node;
						}
					}
				}
			}

			if (x == exit_x && y == exit_y) {
				if (node != NULL) {
					node->type = GNE;
				} else {
					node = gnode_new(x, y, GNE);
					leftnode->nodes[1] = node;
					node->nodes[3] = leftnode;
					leftnode = node;
				}
				mgraph->end = node;
			} else if (x == 1 && y == 1) {
				if (node != NULL) {
					node->type = GNS;
				} else {
					node = gnode_new(x, y, GNS);
					leftnode->nodes[1] = node;
					node->nodes[3] = leftnode;
					leftnode = node;
				}
				mgraph->start = node;

			}

			if (node != NULL) {
				if ((maze[(y - 1) * width + x] != wall)) {
					top = topnodes[x];
					top->nodes[2] = node;
					node->nodes[0] = top;
				}
				if ((maze[(y + 1) * width + x] != wall)) {
					topnodes[x] = node;
				} else {
					topnodes[x] = NULL;
				}

			}
		}
	}
	free(topnodes);
	return mgraph;
}

static void assign_ntype(struct gnode* node) {

	if (node->nodes[0] != NULL && node->nodes[1] == NULL && node->nodes[2] == NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_N;
	} else if (node->nodes[0] == NULL && node->nodes[1] != NULL && node->nodes[2] == NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_E;
	} else if (node->nodes[0] == NULL && node->nodes[1] == NULL && node->nodes[2] != NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_S;
	} else if (node->nodes[0] == NULL && node->nodes[1] == NULL && node->nodes[2] == NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_W;
	} else if (node->nodes[0] != NULL && node->nodes[1] != NULL && node->nodes[2] == NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_NE;
	} else if (node->nodes[0] != NULL && node->nodes[1] == NULL && node->nodes[2] != NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_NS;
	} else if (node->nodes[0] != NULL && node->nodes[1] == NULL && node->nodes[2] == NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_NW;
	} else if (node->nodes[0] == NULL && node->nodes[1] != NULL && node->nodes[2] != NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_ES;
	} else if (node->nodes[0] == NULL && node->nodes[1] != NULL && node->nodes[2] == NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_EW;
	} else if (node->nodes[0] == NULL && node->nodes[1] == NULL && node->nodes[2] != NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_SW;
	} else if (node->nodes[0] != NULL && node->nodes[1] != NULL && node->nodes[2] != NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_NES;
	} else if (node->nodes[0] != NULL && node->nodes[1] != NULL && node->nodes[2] == NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_NEW;
	} else if (node->nodes[0] != NULL && node->nodes[1] == NULL && node->nodes[2] != NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_NSW;
	} else if (node->nodes[0] == NULL && node->nodes[1] != NULL && node->nodes[2] != NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_ESW;
	} else if (node->nodes[0] != NULL && node->nodes[1] != NULL && node->nodes[2] != NULL && node->nodes[3] != NULL) {
		node->ntype = NTYPE_ALL;
	} else if (node->nodes[0] == NULL && node->nodes[1] == NULL && node->nodes[2] == NULL && node->nodes[3] == NULL) {
		node->ntype = NTYPE_NONE;
	}
}

static void draw_node(SDL_Renderer* ren, int xoff, int yoff, struct gnode* node) {
	int x1, y1, x2, y2, i;
	struct gnode* n;
	SDL_Rect rect;

	y1 = (node->y - yoff) * BSIZE + BSIZE / 2;
	x1 = (node->x - xoff) * BSIZE + BSIZE / 2;

	node->rendered = 1;

	SDL_SetRenderDrawColor(ren, 255, 177, 124, 200);
	rect.x = x1 - 8;
	rect.y = y1 - 8;
	rect.w = 16;
	rect.h = 16;
	SDL_RenderFillRect(ren, &rect);
	SDL_SetRenderDrawColor(ren, 12, 12, 12, 255);


	for (i = 0; i < 4; ++i) {
		n = node->nodes[i];
		if (n != NULL) {
			x2 = (n->x - xoff) * BSIZE + BSIZE / 2;
			y2 = (n->y - yoff) * BSIZE + BSIZE / 2;
			SDL_SetRenderDrawColor(ren, 255, 177, 124, 255);
			SDL_RenderDrawLine(ren, x1, y1, x2, y2);
			SDL_RenderDrawLine(ren, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
			SDL_SetRenderDrawColor(ren, 12, 12, 12, 255);
			if (!n->rendered) {
				draw_node(ren, xoff, yoff, node->nodes[i]);
			}
		}
	}
	node->rendered = 0;
}

static void calc_heuristic(struct gnode* node, struct gnode* end) {
	if (node->visited) {
		return;
	}
	assert(node != NULL);
	assert(end != NULL);
	if (node == end) {
		node->h = FLT_MAX;
		return;
	}
	struct gnode* n;
	int i;
	float h = dist_to(node->x, node->y, end->x, end->y);
	node->visited = 1;
	// printf("%d %d\n", node->x, node->y);
	printf("%f\n", h);
	node->h = h;
	for (i = 0; i < 4; ++i) {
		n = node->nodes[i];
		if (n != NULL) {
			calc_heuristic(n, end);
		}
	}
}

#endif //AGAME_GRAPH_H
