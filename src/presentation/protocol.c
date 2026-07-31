#include "presentation/protocol.h"

#include <stdio.h>
#include <inttypes.h>

#include "platform/ui.h"

void
render_protocol(struct global_protocol *gp) {
	unsigned int id;
	struct ui_protocol_entry ui_entries[256];
	unsigned int num_entries;

	num_entries = 0;
	for(id = 0; id < 256; ++id) {
		if(gp->tasks[id].enabled) {
			snprintf(ui_entries[num_entries].description, sizeof(ui_entries[num_entries].description),
						"%" PRIu8 ": <DESCRIPTIONS TBD>", id);
			ui_entries[num_entries++].style = UI_STYLE_DEFAULT;
		}
	}
	ui_set_protocol(ui_entries, num_entries);
}

