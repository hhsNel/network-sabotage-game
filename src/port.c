#include "port.h"

#include <stdlib.h>
#include <stdio.h>

int
port_read_available(struct port *p) {
	if(! p) return 0;

	return p->state == PORT_FULL;
}

int
port_write_available(struct port *p) {
	if(! p) return 0;

	return p->state == PORT_EMPTY;
}

uint16_t
port_read(struct port *p) {
	if(! port_read_available(p)) return 0;

	p->state = PORT_EMPTY;
	return p->value;
}

void
port_write(struct port *p, uint16_t data) {
	if(! port_write_available(p)) return;

	p->state = PORT_CD;
	p->value = data;
}

void
update_port(struct port *p) {
	if(! p) {
		fprintf(stderr, "NULL passed as p to update_port\n");
		exit(1);
	}

	if(p->state == PORT_CD) p->state = PORT_FULL;
}

