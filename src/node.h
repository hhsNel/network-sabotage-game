#ifndef NODE_H
#define NODE_H

#include "port.h"

struct node {
	struct port *read_up, *read_right, *read_down, *read_left;
	struct port *write_up, *write_right, *write_down, *write_left;
	void (*update)(struct node *);
	void (*destroy)(struct node *);
	void *data;
};

struct node create_empty_node();

#endif

