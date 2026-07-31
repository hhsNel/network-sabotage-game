#ifndef PRESENTATION_NODE_H
#define PRESENTATION_NODE_H

#include "core/node.h"
#include "platform/ui.h"

struct ui_board_cell node_render(struct node *);
void node_inspect(struct node *);
void node_interact(struct node *, unsigned int interaction_id);

#endif

