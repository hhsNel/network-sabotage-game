#include "logic/board.h"

#include "logic/node.h"
#include "logic/port.h"

#include <stdio.h>
#include <stdlib.h>

void
init_board(struct board *b) {
	unsigned int i, j;

	*b = (struct board){0};

	for(i = 0; i < BOARD_SZ - 1; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			b->write_right_ports[i][j] = create_port();
			b->write_left_ports[i][j] = create_port();
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ - 1; ++j) {
			b->write_up_ports[i][j] = create_port();
			b->write_down_ports[i][j] = create_port();
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		b->request_ports[i] = create_port();
		b->response_ports[i] = create_port();
	}

	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			insert_node(b, i, j, create_empty_node());
		}
	}
}

void
update_board(struct board *b) {
	unsigned int i, j;

	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			node_update(&b->nodes[i][j]);
		}
	}

	for(i = 0; i < BOARD_SZ - 1; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			update_port(&b->write_right_ports[i][j]);
			update_port(&b->write_left_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ - 1; ++j) {
			update_port(&b->write_up_ports[i][j]);
			update_port(&b->write_down_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		update_port(&b->request_ports[i]);
		update_port(&b->response_ports[i]);
	}
}

void
insert_node(struct board *b, unsigned int x, unsigned int y, struct node n) {
	if(x >= BOARD_SZ || y >= BOARD_SZ) {
		fprintf(stderr, "something went very wrong: insert_node in illegal spot");
		exit(1);
	}

	node_destroy(&b->nodes[x][y]);

	if(x != 0) {
		n.read_left = &b->write_right_ports[x - 1][y];
		n.write_left = &b->write_left_ports[x - 1][y];
	}

	if(x != BOARD_SZ - 1) {
		n.read_right = &b->write_left_ports[x][y];
		n.write_right = &b->write_right_ports[x][y];
	}

	if(y == 0) {
		n.write_up = &b->response_ports[x];
	} else {
		n.read_up = &b->write_down_ports[x][y - 1];
		n.write_up = &b->write_up_ports[x][y - 1];
	}

	if(y == BOARD_SZ - 1) {
		n.read_down = &b->request_ports[x];
	} else {
		n.read_down = &b->write_up_ports[x][y];
		n.write_down = &b->write_down_ports[x][y];
	}

	b->nodes[x][y] = n;
}

