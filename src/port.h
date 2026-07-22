#ifndef PORT_H
#define PORT_H

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

int port_read_available(struct port *p);
int port_write_available(struct port *p);
uint16_t port_read(struct port *p);
void port_write(struct port *p, uint16_t data);
void update_port(struct port *p);

#endif

