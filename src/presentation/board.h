#ifndef PRESENTATION_BOARD_H
#define PRESENTATION_BOARD_H

#include "presentation/node.h"
#include "core/board.h"

struct presentation_board_data {
	struct presentation_node_data nodes[BOARD_SZ][BOARD_SZ];
};

void init_presentation_board(struct presentation_board_data *pbd);
void render_board(struct board *b, unsigned int sel_x, unsigned int sel_y);

#endif

