//
// Created by nik on 12/6/19.
//

#ifndef AGAME_GRAPH_H
#define AGAME_GRAPH_H

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum gnode_type {
	GNS = 0, // start
	GNE = 1, // end
	GNN = 2, // node
};

struct gnode {
	int x;
	int y;
	float h; // heuristic
	enum gnode_type type;
	struct gnode* nodes[4];
};

struct mgraph {
	struct gnode* start;
	struct gnode* end;
};

static struct gnode* gnode_new(int x, int y, enum gnode_type type) {
	struct gnode* newgnode = (struct gnode*) malloc(sizeof(struct gnode));
	newgnode->type = type > -1 ? type : GNN;
	newgnode->x = x;
	newgnode->y = y;
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

	topnodes[1] = gnode_new(1, 1, GNS);
	mgraph->start = topnodes[1];

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
					// floor floor floor
					if ((maze[(y - 1) * width + x] != wall) || (maze[(y + 1) * width + x] != wall)) {
						node = gnode_new(x, y, GNN);
						leftnode->nodes[1] = node;
						node->nodes[3] = leftnode;
						leftnode = node;
					}
				} else {
					// floor floor wall
					node = gnode_new(x, y, GNN);
					leftnode->nodes[3] = node;
					node->nodes[1] = leftnode;
					leftnode = node;
				}

			} else {
				// wall floor floor
				if (next != wall) {
					node = gnode_new(x, y, GNN);
					leftnode = node;
				} else {
					// wall path wall
					if ((maze[(y - 1) * width + x] == wall) || (maze[(y + 1) * width + x] == wall)) {
						node = gnode_new(x, y, GNN);
					}
				}
			}

			if (x == exit_x && y == exit_y) {
				if (node != NULL) {
					node->type = GNS;
				} else {
					node = gnode_new(x, y, GNE);
				}
				mgraph->end = node;
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



#endif //AGAME_GRAPH_H
