#include "presentation/node.h"

#include <stddef.h>

struct ui_board_cell node_empty_render(struct node *);
void node_empty_inspect(struct node *);
void node_empty_interact(struct node *, unsigned int interaction_id);
struct ui_board_cell computer_render(struct node *);
void computer_inspect(struct node *);
void computer_interact(struct node *, unsigned int interaction_id);

struct {
	struct ui_board_cell (*render)(struct node *);
	void (*inspect)(struct node *);
	void (*interact)(struct node *, unsigned int interaction_id);
} node_presentation_vt[NUM_NODE_TYPES] = {
	{ node_empty_render, node_empty_inspect, node_empty_interact }, /* node_empty */
	{ computer_render,   computer_inspect,   computer_interact }, /* node_computer */
};

struct ui_board_cell
node_empty_render(struct node *n) {
	(void)n;
	return (struct ui_board_cell){"EMPTY NODE",UI_STYLE_DEFAULT};
}

void
node_empty_inspect(struct node *n) {
	(void)n;
	ui_set_inspect(NULL, 0, "This is an empty node.");
}

void
node_empty_interact(struct node *n, unsigned int interaction_id) {
	(void)n;
	(void)interaction_id;
}

struct ui_board_cell
node_render(struct node *n) {
	return node_presentation_vt[n->type].render(n);
}

void
node_inspect(struct node *n) {
	node_presentation_vt[n->type].inspect(n);
}

void
node_interact(struct node *n, unsigned int interaction_id) {
	node_presentation_vt[n->type].interact(n, interaction_id);
}

