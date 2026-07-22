#ifndef NODE_H
#define NODE_H

#include "port.h"

enum node_type {
	NODE_COMPUTER = 0,
	NODE_OTHER = 1,
};

struct node {
	enum node_type type;
	struct port *read_up, *read_right, *read_down, *read_left;
	struct port *write_up, *write_right, *write_down, *write_left;
	void (*update)(struct node *);
	void (*destroy)(struct node *);
	void *data;
};

#endif

