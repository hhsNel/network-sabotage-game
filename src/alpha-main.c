#include "logic/protocol.h"
#include "presentation/protocol.h"
#include "logic/player-data.h"
#include "logic/board.h"
#include "presentation/board.h"
#include "presentation/node.h"
#include "platform/ui.h"
#include "platform/tmpfile.h"
#include "logic/risc-labs.h"

#include <stdlib.h>
#include <stdio.h>

void cleanup() {
	ui_shutdown();
	tmpfile_shutdown();
}

int main() {
	struct player_data pd;
	struct global_protocol gp;
	struct ui_event ev;
	struct ui_button misc_btns[3];
	struct ui_shop_item shop_items[1];
	char status_buf[256];
	enum player_ui_status { PLAYER_STATUS_NORMAL, PLAYER_STATUS_PLACING };
	enum player_ui_status ui_s;
	unsigned int shop_item_bought;
	unsigned int selected_board_x, selected_board_y;

	pd = create_player();
	gp = init_protocol();

	ui_init();
	tmpfile_init();
	atexit(cleanup);

	misc_btns[0] = (struct ui_button){"Stop input", UI_STYLE_DEFAULT};
	misc_btns[1] = (struct ui_button){"Reset all", UI_STYLE_DEFAULT};
	misc_btns[2] = (struct ui_button){"Flash all", UI_STYLE_DEFAULT};
	ui_set_misc_btns(misc_btns, sizeof(misc_btns)/sizeof(*misc_btns));

	shop_items[0] = (struct ui_shop_item){"RiscLabs", "General Purpose Processor", UI_STYLE_DEFAULT, "some details who cares\nthis is a\nnew\nline", {"buy",UI_STYLE_DEFAULT}};
	ui_set_shop_items(shop_items, sizeof(shop_items)/sizeof(*shop_items));

	ui_s = PLAYER_STATUS_NORMAL;
	selected_board_x = selected_board_y = 0;

	while(1) {
		if(rand() % 16 == 0) mutate_protocol(&gp, rand() % 8);
		update_player(&pd, &gp);
		render_board(&pd.board, selected_board_x, selected_board_y);
		if(ui_s == PLAYER_STATUS_NORMAL) {
			snprintf(status_buf, sizeof(status_buf), "Network Sabotage Alpha | Money: %u", pd.money);
		} else {
			snprintf(status_buf, sizeof(status_buf), "Network Sabotage Alpha | Select the node for placement");
		}
		ui_set_status(status_buf);
		node_inspect(&pd.board.nodes[selected_board_x][selected_board_y]);

		ui_flush();

		while((ev = ui_poll(125)).type != UI_EVENT_NONE) {
			switch(ev.type) {
			case UI_EVENT_ACTIVATE:
				if(ev.active_region == UI_REGION_BOARD && ui_s == PLAYER_STATUS_PLACING) {
					if(shop_item_bought != 0) {
						fprintf(stderr, "wtf\n");
						exit(1);
					}
					insert_node(&pd.board, ev.data.board.x, ev.data.board.y, create_rl_computer(0));
					ui_s = PLAYER_STATUS_NORMAL;
					break;
				}
				ui_s = PLAYER_STATUS_NORMAL;
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
					default:
						fprintf(stderr, "something went very wrong: item_id in MISC: %u\n", ev.data.item_id);
						exit(1);
					}
					break;
				case UI_REGION_SHOP:
					ui_s = PLAYER_STATUS_PLACING;
					shop_item_bought = ev.data.item_id;
					break;
				case UI_REGION_BOARD:
					selected_board_x = ev.data.board.x;
					selected_board_y = ev.data.board.y;
					break;
				case UI_REGION_INSPECT:
					node_interact(&pd.board.nodes[selected_board_x][selected_board_y], ev.data.item_id);
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

