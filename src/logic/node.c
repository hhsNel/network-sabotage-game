#include "logic/node.h"

#include <stddef.h>

void node_none_update(struct node *n);
void node_none_destroy(struct node *n);
void computer_update(struct node *n);
void computer_destroy(struct node *n);

struct {
	void (*update)(struct node *);
	void (*destroy)(struct node *);
} node_logic_vt[NUM_NODE_TYPES] = {
	{ node_none_update, node_none_destroy }, /* node_empty */
	{ computer_update,  computer_destroy }, /* node_computer */
};

struct node
create_empty_node() {
	struct node n;

	n.read_up = n.read_right = n.read_down = n.read_left = NULL;
	n.write_up = n.write_right = n.write_down = n.write_left = NULL;

	n.data = NULL;

	return n;
}

void
node_none_update(struct node *n) {
	(void)n;
}

void
node_none_destroy(struct node *n) {
	(void)n;
}

void
node_update(struct node *n) {
	node_logic_vt[n->type].update(n);
}

void
node_destroy(struct node *n) {
	node_logic_vt[n->type].destroy(n);
}

