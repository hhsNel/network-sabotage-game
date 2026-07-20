#ifndef PORT_H
#define PORT_H

#include <stdint.h>

enum port_state {
	PORT_EMPTY, /* no data */
	PORT_FULL, /* data */
	PORT_CD, /* data was written this tick */
};

struct port {
	uint16_t value;
	uint8_t enabled;
};

#endif

