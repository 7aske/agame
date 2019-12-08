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
#include <limits.h>
#include <SDL2/SDL.h>

#include "structs/pqueue.h"
#include "util.h"

enum gnode_type {
	GNS = 1, // start
	GNE = 2, // end
	GNN = 3, // node
};

struct gnode {
	int x;
	int y;

	int h; // heuristic
	int f; // heuristic
	int g; // heuristic

	int visited;
	int rendered;
	int path;

	struct gnode* came_from;

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

	newgnode->h = INT_MAX;
	newgnode->f = INT_MAX;
	newgnode->g = INT_MAX;

	newgnode->came_from = NULL;
	newgnode->path = 0;

	newgnode->nodes[0] = NULL; // north
	newgnode->nodes[1] = NULL; // east
	newgnode->nodes[2] = NULL; // south
	newgnode->nodes[3] = NULL; // west
	return newgnode;
}


static struct mgraph*
to_graph(char const* maze, int width, int height, char wall, int start_x, int start_y, int exit_x, int exit_y) {
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
			} else if (x == start_x && y == start_y) {
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

static void draw_node(SDL_Renderer* ren, int xoff, int yoff, struct gnode* node) {
	int x1, y1, x2, y2, i;
	struct gnode* n;
	SDL_Rect rect;
	#define VISITED_COLOR SDL_SetRenderDrawColor(ren, 183, 65, 14 , 200);
	#define PATH_COLOR SDL_SetRenderDrawColor(ren, 0, 158, 96, 200);
	#define DEFAULT_COLOR SDL_SetRenderDrawColor(ren, 91, 52, 46, 200);

	y1 = (node->y - yoff) * BSIZE + BSIZE / 2;
	x1 = (node->x - xoff) * BSIZE + BSIZE / 2;
	node->rendered = 1;

	for (i = 0; i < 4; ++i) {
		n = node->nodes[i];
		if (n != NULL) {
			x2 = (n->x - xoff) * BSIZE + BSIZE / 2;
			y2 = (n->y - yoff) * BSIZE + BSIZE / 2;
			DEFAULT_COLOR
			if (node->visited && n->visited)
				VISITED_COLOR
			if (node->path && n->path)
				PATH_COLOR

			SDL_RenderDrawLine(ren, x1, y1, x2, y2);
			SDL_RenderDrawLine(ren, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
			SDL_SetRenderDrawColor(ren, 12, 12, 12, 255);
			if (!n->rendered) {
				draw_node(ren, xoff, yoff, n);
			}
		}
	}


	DEFAULT_COLOR
	if (node->visited) {
		VISITED_COLOR
	}
	if (node->path) {
		PATH_COLOR
	}

	rect.x = x1 - 8;
	rect.y = y1 - 8;
	rect.w = 16;
	rect.h = 16;
	SDL_RenderFillRect(ren, &rect);
	SDL_SetRenderDrawColor(ren, 12, 12, 12, 255);

	node->rendered = 0;
}

static int gnodecmp(void const* n1, void const* n2, size_t size) {
	int p1 = *(int*) n1;
	int p2 = *(int*) n2;

	if (p1 < p2) {
		return 1;
	} else if (p1 > p2) {
		return -1;
	} else {
		return 0;
	}
}

static astack_t* solve_astar(struct mgraph* graph) {
	struct gnode* start = graph->start;
	struct gnode* end = graph->end;
	struct gnode** node = NULL;
	struct gnode* n = NULL;
	int coord[2];
	astack_t* solution = stack_new(sizeof(int[2]));
	int inf = INT_MAX, i;

	node = &graph->start;
	(*node)->f = inf;
	(*node)->came_from = NULL;
	(*node)->path = 1;

	coord[0] = graph->start->x;
	coord[1] = graph->start->y;

	pqueue_t* visited = pqueue_new(sizeof(struct gnode*), sizeof(int));
	pqueue_set_cmp(visited, gnodecmp);

	stack_push(solution, coord);
	pqueue_enqueue(visited, &start, &(*node)->f);

	while (!pqueue_isempty(visited)) {
		node = (struct gnode**) pqueue_dequeue(visited);
		(*node)->visited = 1;
		if ((*node)->x == end->x && (*node)->y == end->y) {
			printf("holy shit\n");
			break;
		}

		for (i = 0; i < 4; ++i) {
			n = (*node)->nodes[i];
			if (n != NULL) {
				n->h = (int) euclidean_dist(n->x, n->y, end->x, end->y);
				n->g = (*node)->f + manhattan_dist(n->x, n->y, (*node)->x, (*node)->y);
				n->f = n->g + n->h;
				if (!n->visited) {
					n->came_from = *node;
					// printf("%p\n", *node);
					pqueue_enqueue(visited, &n, &n->f);
				}
			}
		}
		free(node);
	}
	n = *node;
	n->path = 1;
	while (n->came_from != NULL) {
		n->path = 1;
		stack_push(solution, coord);
		n = n->came_from;
	}
	coord[0] = end->x;
	coord[1] = end->y;
	stack_push(solution, coord);

	pqueue_destroy(&visited);
	return solution;
}

static void mgraph_destroy(struct mgraph** graph) {
	queue_t* queue = queue_new(sizeof(struct gnode*));
	int i;
	struct gnode** node;
	struct gnode* n;
	struct gnode* start = (*graph)->start;

	queue_enqueue(queue, &start);

	while (!queue_isempty(queue)) {
		node = queue_dequeue(queue);
		for (i = 0; i < 4; ++i) {
			n = (*node)->nodes[i];
			if (n != NULL) {
				n->nodes[(i + 2) % 4] = NULL;
				queue_enqueue(queue, &n);
			}
		}
		free(node);
	}
	queue_destroy(queue);
	free(*graph);
	*graph = NULL;
}

#endif //AGAME_GRAPH_H
