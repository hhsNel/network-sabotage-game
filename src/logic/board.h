#ifndef LOGIC_BOARD_H
#define LOGIC_BOARD_H

#include "core/board.h"

void init_board(struct board *b);
void update_board(struct board *b);
void insert_node(struct board *b, unsigned int x, unsigned int y, struct node n);

#endif

