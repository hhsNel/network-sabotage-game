#include "protocol.h"
#include "player-data.h"
#include "platform/ui.h"

#include <stdlib.h>
#include <stdio.h>

void cleanup() {
	ui_shutdown();
}

int main() {
	struct player_data pd;
	struct global_protocol gp;
	struct ui_event ev;
	struct ui_button misc_btns[4];
	struct ui_shop_item shop_items[1];

	pd = create_player();
	gp = init_protocol();

	ui_init();
	atexit(cleanup);

	misc_btns[0] = (struct ui_button){"Preview opponent", UI_STYLE_DEFAULT};
	misc_btns[1] = (struct ui_button){"Stop input", UI_STYLE_DEFAULT};
	misc_btns[2] = (struct ui_button){"Reset all", UI_STYLE_DEFAULT};
	misc_btns[3] = (struct ui_button){"Flash all", UI_STYLE_DEFAULT};
	ui_set_misc_btns(misc_btns, sizeof(misc_btns)/sizeof(*misc_btns));

	shop_items[0] = (struct ui_shop_item){"RiscLabs", "General Purpose Processor", UI_STYLE_DEFAULT, "some details who cares\nthis is a\nnew\nline", {"buy",UI_STYLE_DEFAULT}};
	ui_set_shop_items(shop_items, sizeof(shop_items)/sizeof(*shop_items));

	while(1) {
		if(rand() % 16 == 0) mutate_protocol(&gp, rand() % 8);
		update_player(&pd, &gp);

		ui_flush();

		while((ev = ui_poll(125)).type != UI_EVENT_NONE) {
			switch(ev.type) {
			case UI_EVENT_ACTIVATE:
				switch(ev.active_region) {
				case UI_REGION_MISC:
					switch(ev.data.item_id) {
					case 0:
						/* TODO */
						fprintf(stderr, "not implemented\n");
						exit(1);
					case 1:
						/* TODO */
						fprintf(stderr, "not implemented\n");
						exit(1);
					case 2:
						/* TODO */
						fprintf(stderr, "not implemented\n");
						exit(1);
					case 3:
						/* TODO */
						fprintf(stderr, "not implemented\n");
						exit(1);
					default:
						fprintf(stderr, "something went very wrong: item_id in MISC: %u\n", ev.data.item_id);
						exit(1);
					}
					break;
				default:
					break;
				}
				break;
			case UI_EVENT_QUIT:
				exit(0);
			default:
				break;
			}
		}
	}
}

