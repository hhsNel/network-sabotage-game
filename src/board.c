#include "board.h"

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
	struct ui_board_cell ui_board[BOARD_SZ][BOARD_SZ];
	struct ui_port ui_right[BOARD_SZ-1][BOARD_SZ], ui_left[BOARD_SZ-1][BOARD_SZ], ui_up[BOARD_SZ][BOARD_SZ-1], ui_down[BOARD_SZ][BOARD_SZ-1];
	struct ui_port ui_request[BOARD_SZ], ui_response[BOARD_SZ];

	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			b->nodes[i][j].update(&b->nodes[i][j]);
			if(b->nodes[i][j].render) ui_board[i][j] = b->nodes[i][j].render(&b->nodes[i][j]);
			else ui_board[i][j] = (struct ui_board_cell){"EMPTY\nEMPTY\nEMPTY\nEMPTY",UI_STYLE_DEFAULT};
		}
	}

	for(i = 0; i < BOARD_SZ - 1; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			update_port(&b->write_right_ports[i][j]);
			update_port(&b->write_left_ports[i][j]);
			ui_right[i][j] = render_port(&b->write_right_ports[i][j]);
			ui_left[i][j] = render_port(&b->write_left_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ - 1; ++j) {
			update_port(&b->write_up_ports[i][j]);
			update_port(&b->write_down_ports[i][j]);
			ui_up[i][j] = render_port(&b->write_up_ports[i][j]);
			ui_down[i][j] = render_port(&b->write_down_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		update_port(&b->request_ports[i]);
		update_port(&b->response_ports[i]);
		ui_request[i] = render_port(&b->request_ports[i]);
		ui_response[i] = render_port(&b->response_ports[i]);
	}

	ui_set_board(ui_board, ui_right, ui_left, ui_up, ui_down, ui_request, ui_response);
}

void
insert_node(struct board *b, unsigned int x, unsigned int y, struct node n) {
	if(x >= BOARD_SZ || y >= BOARD_SZ) {
		fprintf(stderr, "something went very wrong: insert_node in illegal spot");
		exit(1);
	}

	if(b->nodes[x][y].destroy) b->nodes[x][y].destroy(&b->nodes[x][y]);

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

