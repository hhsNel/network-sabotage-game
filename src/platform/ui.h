#ifndef PLATFORM_UI_H
#define PLATFORM_UI_H

#include "board.h"

#include <stdint.h>

enum ui_style {
	UI_STYLE_DEFAULT,
	UI_STYLE_UNAVAILABLE,
	UI_STYLE_BAD,
	UI_STYLE_ATTENTION,
};

struct ui_button {
	char label[24];
	enum ui_style style;
};

struct ui_shop_item {
	char company[32];
	char name[32];
	enum ui_style text_style;
	char detail[256];
	struct ui_button btn;
};

struct ui_board_cell {
	char status[32];
	enum ui_style style;
};

struct ui_port {
	char status[2][2];
	enum ui_style style;
};

struct ui_protocol_entry {
	char description[128];
	enum ui_style style;
};

enum ui_region {
	UI_REGION_MISC,
	UI_REGION_SHOP,
	UI_REGION_PROTOCOL,
	UI_REGION_BOARD,
	UI_REGION_INSPECT,
};

enum ui_event_type {
	UI_EVENT_NONE,
	UI_EVENT_ACTIVATE,
	UI_EVENT_QUIT,
};

struct ui_event {
	enum ui_event_type type;
	enum ui_region active_region;
	union {
		unsigned int item_id;
		struct {
			unsigned int x, y;
		} board;
	} data;
};

void ui_init();
void ui_shutdown();
void ui_set_misc_btns(struct ui_button *btns, unsigned int num);
void ui_set_shop_items(struct ui_shop_item *items, unsigned int num);
void ui_set_board(struct ui_board_cell board[BOARD_SZ][BOARD_SZ], struct ui_port write_right[BOARD_SZ-1][BOARD_SZ], struct ui_port write_left[BOARD_SZ-1][BOARD_SZ], struct ui_port write_up[BOARD_SZ][BOARD_SZ-1], struct ui_port write_down[BOARD_SZ][BOARD_SZ-1], struct ui_port request[BOARD_SZ], struct ui_port response[BOARD_SZ]);
void ui_set_protocol(struct ui_protocol_entry *entries, unsigned int num);
void ui_set_inspect(struct ui_button *btns, unsigned int btn_nr, char *text);
void ui_flush();
struct ui_event ui_poll(int ms); /* <0 blocks; ==0 returns immediately */

#endif

