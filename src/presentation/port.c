#include "presentation/port.h"

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

