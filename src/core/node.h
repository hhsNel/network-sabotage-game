#ifndef CORE_NODE_H
#define CORE_NODE_H

#include <stdint.h>

#include "core/port.h"

enum {
	NODE_EMPTY = 0,
	NODE_COMPUTER,
	NUM_NODE_TYPES,
};
typedef uint16_t node_type;

struct node {
	node_type type;
	struct port *read_up, *read_right, *read_down, *read_left;
	struct port *write_up, *write_right, *write_down, *write_left;
	void *data;
};

#endif

