#include "port.h"

#include <stdlib.h>
#include <stdio.h>

struct port
create_port() {
	return (struct port){ PORT_EMPTY, 0 };
}

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

struct ui_port
render_port(struct port *p) {
	struct ui_port up;

	up.style = UI_STYLE_DEFAULT;
	if(p) {
		switch(p->state) {
		case PORT_EMPTY:
			up.status[0][0] = up.status[0][1] = up.status[1][0] = up.status[1][1] = ' ';
			break;
		case PORT_CD:
			up.status[0][0] = up.status[1][0] = 'C';
			up.status[0][1] = up.status[1][1] = 'D';
			break;
		case PORT_FULL:
#define NIBBLE_TO_C(N) \
	((N) >= 10 ? ((N) - 10 + 'a') : ((N) + '0'))
			up.status[0][0] = NIBBLE_TO_C((p->value >> 12) & 0x0F);
			up.status[0][1] = NIBBLE_TO_C((p->value >> 8) & 0x0F);
			up.status[1][0] = NIBBLE_TO_C((p->value >> 4) & 0x0F);
			up.status[1][1] = NIBBLE_TO_C(p->value & 0x0F);
			break;
#undef NIBBLE_TO_C
		}
	} else {
		up.status[0][0] = up.status[0][1] = up.status[1][0] = up.status[1][1] = ' ';
	}

	return up;
}

