#ifndef CORE_PORT_H
#define CORE_PORT_H

#include <stdint.h>

enum port_state {
	PORT_EMPTY, /* no data */
	PORT_FULL, /* data */
	PORT_CD, /* data was written this tick */
};

struct port {
	enum port_state state;
	uint16_t value;
};

#endif

