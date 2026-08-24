#ifndef PRESENTATION_COMPUTER_H
#define PRESENTATION_COMPUTER_H

#include "core/computer.h"
#include "presentation/node.h"

void computer_inspect_text(char *text);

struct ui_board_cell computer_render(struct node *);
void computer_inspect(struct node *);
void computer_interact(struct node *, struct presentation_node_data *pn, unsigned int interaction_id);

#endif

