#ifndef PRESENTATION_NODE_H
#define PRESENTATION_NODE_H

#include "core/node.h"
#include "platform/ui.h"
#include "platform/tmpfile.h"

struct presentation_node_data {
	tmpfile_id bound_tmpfile;
};

struct presentation_node_data init_presentation_node();
struct ui_board_cell node_render(struct node *);
void node_inspect(struct node *);
void node_interact(struct node *, struct presentation_node_data *, unsigned int interaction_id);

#endif

