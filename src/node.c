#include "node.h"

#include <stddef.h>

static void do_nothing(struct node *n);

struct node
create_empty_node() {
	struct node n;

	n.read_up = n.read_right = n.read_down = n.read_left = NULL;
	n.write_up = n.write_right = n.write_down = n.write_left = NULL;

	n.update = do_nothing;
	n.destroy = do_nothing;
	n.render = NULL;
	n.data = NULL;

	return n;
}

static void
do_nothing(struct node *n) {
	(void)n;
}

