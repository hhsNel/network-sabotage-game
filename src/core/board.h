#ifndef CORE_BOARD_H
#define CORE_BOARD_H

#include "core/util.h"
#include "core/node.h"
#include "core/port.h"

struct board {
	struct node nodes[BOARD_SZ][BOARD_SZ];
	struct port write_right_ports[BOARD_SZ - 1][BOARD_SZ];
	struct port write_left_ports[BOARD_SZ - 1][BOARD_SZ];
	struct port write_up_ports[BOARD_SZ][BOARD_SZ - 1];
	struct port write_down_ports[BOARD_SZ][BOARD_SZ - 1];
	struct port request_ports[BOARD_SZ];
	struct port response_ports[BOARD_SZ];
};

#endif

