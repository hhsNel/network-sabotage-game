#ifndef LOGIC_PORT_H
#define LOGIC_PORT_H

#include "core/port.h"

struct port create_port();
int port_read_available(struct port *p);
int port_write_available(struct port *p);
uint16_t port_read(struct port *p);
void port_write(struct port *p, uint16_t data);
void update_port(struct port *p);

#endif

