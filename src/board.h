#ifndef BOARD_H
#define BOARD_H

#include "node.h"
#include "port.h"

#define BOARD_SZ 8

struct board {
	struct node nodes[BOARD_SZ][BOARD_SZ];
	struct port write_right_ports[BOARD_SZ - 1][BOARD_SZ];
	struct port write_left_ports[BOARD_SZ - 1][BOARD_SZ];
	struct port write_up_ports[BOARD_SZ][BOARD_SZ - 1];
	struct port write_down_ports[BOARD_SZ][BOARD_SZ - 1];
	struct port request_ports[BOARD_SZ];
	struct port response_ports[BOARD_SZ];
};

void init_board(struct board *b);
void update_board(struct board *b);
void insert_node(struct board *b, unsigned int x, unsigned int y, struct node n);

#endif

