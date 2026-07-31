#ifndef LOGIC_NODE_H
#define LOGIC_NODE_H

#include "core/node.h"

struct node create_empty_node();
void node_update(struct node *n);
void node_destroy(struct node *n);

#endif

