#include "presentation/board.h"

#include "presentation/node.h"
#include "presentation/port.h"

void
render_board(struct board *b, unsigned int sel_x, unsigned int sel_y) {
	unsigned int i, j;
	struct ui_board_cell ui_board[BOARD_SZ][BOARD_SZ];
	struct ui_port ui_right[BOARD_SZ-1][BOARD_SZ], ui_left[BOARD_SZ-1][BOARD_SZ], ui_up[BOARD_SZ][BOARD_SZ-1], ui_down[BOARD_SZ][BOARD_SZ-1];
	struct ui_port ui_request[BOARD_SZ], ui_response[BOARD_SZ];

	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			ui_board[i][j] = node_render(&b->nodes[i][j]);
		}
	}

	for(i = 0; i < BOARD_SZ - 1; ++i) {
		for(j = 0; j < BOARD_SZ; ++j) {
			ui_right[i][j] = render_port(&b->write_right_ports[i][j]);
			ui_left[i][j] = render_port(&b->write_left_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		for(j = 0; j < BOARD_SZ - 1; ++j) {
			ui_up[i][j] = render_port(&b->write_up_ports[i][j]);
			ui_down[i][j] = render_port(&b->write_down_ports[i][j]);
		}
	}
	for(i = 0; i < BOARD_SZ; ++i) {
		ui_request[i] = render_port(&b->request_ports[i]);
		ui_response[i] = render_port(&b->response_ports[i]);
	}

	if(ui_board[sel_x][sel_y].style == UI_STYLE_DEFAULT) {
		ui_board[sel_x][sel_y].style = UI_STYLE_ATTENTION;
	}
	ui_set_board(ui_board, ui_right, ui_left, ui_up, ui_down, ui_request, ui_response);
}

