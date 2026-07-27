#include "ui.h"

#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
--------------------------------------------------------------------------------------------------------------------------------------------
| [Preview opponent] | Electric Hardware  Hackers United       RiscLabs                                                                    |
| [Stop input]       | Wire cross         Request transmitter  GPC                                                                         |
| [Reset All]        | $5                 $5                   $30                                                                         |
| [Flash Reset All]  | [Buy]              [Buy]                [Buy]                                                                       |
--------------------------------------------------------------------------------------------------------------------------------------------
| 0. copy A       |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  | RiscLabs (C)   |
| without change  |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  | Genral Purpose |
| to output       |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  | Computer (tm)  |
| 49. bitwise AND |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
| A and B, where  |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  | [Edit]         |
| B is data*0x101 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  | [Reset]        |
| to output       |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  | [Flash Reset]  |
| 168. ADD A and  |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  | [Preview]      |
| B, where B is   |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
| data * 0x101    |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |  r0    | r1    |
| to output       |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |  00 00 | 00 00 |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |  r2    | r3    |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |  00 00 | 00 00 |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |  PC    | LAST  |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |  1a    | RIGHT |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |   MOV r0 r1    |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |   ADD r2 r1 r0 |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |  >OUT r2 UP    |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |   JMP -8 <3    |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |   ins. back>   |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |   IN r0 DOWN   |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |   JMP -16 <7   |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |   ins. back>   |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |                |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |                |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v     v00  00v  |                |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........00 00........00 00........00 00........00 00........00 00........00 00........00 00........  |                |
|                 |  ........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........>> <<........  |                |
|                 |  ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^     ^00  00^  |                |
--------------------------------------------------------------------------------------------------------------------------------------------
*/

#define PROTOCOL_W 24
#define INSPECT_W 32
#define SHOP_ITEM_W 32
#define BOARD_MIN_W 64
#define BOARD_MIN_H 32

struct rect {
	unsigned int x, y;
	unsigned int w, h;
};

struct ui_button *misc_buttons;
unsigned int num_misc_buttons, cap_misc_buttons;

struct ui_shop_item *shop_items;
unsigned int num_shop_items, cap_shop_items;

struct ui_protocol_entry *protocol_entries;
unsigned int num_protocol_entries, cap_protocol_entries;

struct ui_board_cell board[BOARD_SZ][BOARD_SZ];
struct ui_port write_right_ports[BOARD_SZ-1][BOARD_SZ];
struct ui_port write_left_ports[BOARD_SZ-1][BOARD_SZ];
struct ui_port write_up_ports[BOARD_SZ][BOARD_SZ-1];
struct ui_port write_down_ports[BOARD_SZ][BOARD_SZ-1];
struct ui_port request_ports[BOARD_SZ];
struct ui_port response_ports[BOARD_SZ];

struct ui_button *inspect_buttons;
unsigned int num_inspect_buttons, cap_inspect_buttons;
char *inspect_text;
unsigned int cap_inspect_text;

enum ui_region current_focus;
unsigned int region_selected, region_scroll;
unsigned int board_x, board_y;

enum kb_state { PENDING_NONE, PENDING_C_W, PENDING_G };
enum kb_state pending;

unsigned int scr_x, scr_y;
struct rect header_rect, misc_rect, shop_rect, protocol_rect, board_rect, inspect_rect;

enum color_pair {
	CP_DEFAULT = 1,
	CP_UNAVAILABLE = 2,
	CP_BAD = 3,
	CP_ATTENTION = 4,
};

static enum color_pair style_to_cp(enum ui_style s);
static void set_style(enum ui_style s, int is_selected);
static void draw_header(struct rect r, char *title, int is_focused);
static unsigned int wrap_text(char *text, unsigned int max_width, char (*lines)[256], unsigned int cap_lines);
static unsigned int shop_item_height(struct ui_shop_item it);
static void recompute_layout();
static void draw_btn(unsigned int x, unsigned int y, struct ui_button btn, int is_selected);
static void draw_misc();
static void draw_shop();
static void draw_protocol();
static void draw_inspect();
static void move_focus(int key);

void
ui_init() {
	if(! initscr()) {
		fprintf(stderr, "something went very wrong: initscr\n");
		exit(1);
	}

	if(start_color() == ERR) {
		endwin();
		fprintf(stderr, "something went very wrong: start_color\n");
		exit(1);
	}
	init_pair(CP_DEFAULT, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_UNAVAILABLE, COLOR_YELLOW, COLOR_BLACK);
	init_pair(CP_BAD, COLOR_RED, COLOR_BLACK);
	init_pair(CP_ATTENTION, COLOR_CYAN, COLOR_BLACK);

	raw();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);

	misc_buttons = NULL;
	num_misc_buttons = cap_misc_buttons = 0;
	shop_items = NULL;
	num_shop_items = cap_shop_items = 0;
	protocol_entries = NULL;
	num_protocol_entries = cap_protocol_entries = 0;
	inspect_buttons = NULL;
	num_inspect_buttons = cap_inspect_buttons = 0;
	inspect_text = NULL;
	cap_inspect_text = 0;

	current_focus = UI_REGION_BOARD;
	board_x = board_y = 0;
	
	pending = PENDING_NONE;
}

void
ui_shutdown() {
	endwin();
}

void
ui_set_misc_btns(struct ui_button *btns, unsigned int num) {
	if(cap_misc_buttons < num) {
		cap_misc_buttons = num;
		misc_buttons = realloc(misc_buttons, cap_misc_buttons * sizeof(struct ui_button));
		if(! misc_buttons) {
			fprintf(stderr, "something went very wrong: realloc misc_buttons\n");
			exit(1);
		}
	}
	num_misc_buttons = num;
	memcpy(misc_buttons, btns, num_misc_buttons * sizeof(struct ui_button));
}

void
ui_set_shop_items(struct ui_shop_item *items, unsigned int num) {
	if(cap_shop_items < num) {
		cap_shop_items = num;
		shop_items = realloc(shop_items, cap_shop_items * sizeof(struct ui_shop_item));
		if(! shop_items) {
			fprintf(stderr, "something went very wrong: realloc shop_items\n");
			exit(1);
		}
	}
	num_shop_items = num;
	memcpy(shop_items, items, num_shop_items * sizeof(struct ui_shop_item));
}

void
ui_set_board(struct ui_board_cell board[BOARD_SZ][BOARD_SZ], struct ui_port write_right[BOARD_SZ-1][BOARD_SZ], struct ui_port write_left[BOARD_SZ-1][BOARD_SZ], struct ui_port write_up[BOARD_SZ][BOARD_SZ-1], struct ui_port write_down[BOARD_SZ][BOARD_SZ-1], struct ui_port request[BOARD_SZ], struct ui_port response[BOARD_SZ]) {
	fprintf(stderr, "not implemented");
	exit(1);
	(void)board;
	(void)write_right;
	(void)write_left;
	(void)write_up;
	(void)write_down;
	(void)request;
	(void)response;
}

void
ui_set_protocol(struct ui_protocol_entry *entries, unsigned int num) {
if(cap_protocol_entries < num) {
	cap_protocol_entries = num;
	protocol_entries = realloc(protocol_entries, cap_protocol_entries * sizeof(struct ui_protocol_entry));
	if(! protocol_entries) {
		fprintf(stderr, "something went very wrong: realloc protocol_entries\n");
		exit(1);
	}
}
num_protocol_entries = num;
memcpy(protocol_entries, entries, num_protocol_entries * sizeof(struct ui_protocol_entry));
}

void
ui_set_inspect(struct ui_button *btns, unsigned int btn_nr, char *text) {
	if(cap_inspect_buttons < btn_nr) {
		cap_inspect_buttons = btn_nr;
		inspect_buttons = realloc(inspect_buttons, cap_inspect_buttons * sizeof(struct ui_button));
		if(! inspect_buttons) {
			fprintf(stderr, "something went very wrong: realloc inspect_buttons\n");
			exit(1);
		}
	}
	num_inspect_buttons = btn_nr;
	memcpy(inspect_buttons, btns, num_inspect_buttons * sizeof(struct ui_button));

	if(cap_inspect_text < strlen(text)+1) {
		cap_inspect_text = strlen(text) + 1;
		inspect_text = realloc(inspect_text, cap_inspect_text);
		if(! inspect_text) {
			fprintf(stderr, "something went very wrong: realloc inspect_text\n");
			exit(1);
		}
	}
	strcpy(inspect_text, text);
}

void
ui_flush() {
	erase();
	char buf[256];

	recompute_layout();
	if(board_rect.w < BOARD_MIN_W || board_rect.h < BOARD_MIN_H) {
		mvprintw(0, 0, "Board too small (%ux%u). Please resize window.", board_rect.w, board_rect.h);
		return;
	}

	draw_misc();
	draw_shop();
	draw_protocol();
	/* TODO: draw_board(); */
	draw_inspect();

	snprintf(buf, sizeof(buf), "Network Sabotage Alpha | Region: %d | Pending: %d", (int)current_focus, (int)pending);
	mvprintw(header_rect.y, header_rect.x, "%-*.*s", header_rect.w, header_rect.w, buf);
	refresh();
}

struct ui_event
ui_poll(int ms) {
	int key;

	timeout(ms);
	key = getch();

	if(key == ERR) return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};

	if(pending == PENDING_C_W) {
		move_focus(key);
		pending = PENDING_NONE;
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	}

	if(pending == PENDING_G) {
		if(key == 'g') {
			if(current_focus == UI_REGION_BOARD) {
				board_x = board_y = 0;
			} else {
				region_selected = region_scroll = 0;
			}
		}
		pending = PENDING_NONE;
	}

	switch(key) {
	case 'q':
	case 3:
		return (struct ui_event){UI_EVENT_QUIT, current_focus, {0}};
	case 23:
		pending = PENDING_C_W;
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'g':
		pending = PENDING_G;
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'G':
		switch(current_focus) {
		case UI_REGION_MISC:
			if(num_misc_buttons) region_selected = region_scroll = num_misc_buttons - 1;
			break;
		case UI_REGION_SHOP:
			if(num_shop_items) region_selected = region_scroll = num_shop_items - 1;
			break;
		case UI_REGION_PROTOCOL:
			if(num_protocol_entries) region_selected = region_scroll = num_protocol_entries - 1;
			break;
		case UI_REGION_BOARD:
			board_x = board_y = BOARD_SZ - 1;
			break;
		case UI_REGION_INSPECT:
			if(num_inspect_buttons) region_selected = region_scroll = num_inspect_buttons - 1;
			break;
		}
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'h':
		if(current_focus != UI_REGION_BOARD) {
			if(region_selected) --region_selected;
			if(region_scroll) --region_scroll;
		} else {
			if(board_x) --board_x;
		}
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'j':
		if(current_focus != UI_REGION_BOARD) {
			++region_selected;
			++region_scroll;
		} else {
			if(board_y < BOARD_SZ-1) ++board_y;
		}
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'k':
		if(current_focus != UI_REGION_BOARD) {
			if(region_selected) --region_selected;
			if(region_scroll) --region_scroll;
		} else {
			if(board_y) --board_y;
		}
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case 'l':
		if(current_focus != UI_REGION_BOARD) {
			++region_selected;
			++region_scroll;
		} else {
			if(board_x < BOARD_SZ-1) ++board_x;
		}
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	case ' ':
	case '\r':
	case '\n':
		switch(current_focus) {
		case UI_REGION_MISC:
		case UI_REGION_SHOP:
		case UI_REGION_INSPECT:
			return (struct ui_event){UI_EVENT_ACTIVATE, current_focus, {.item_id=region_selected}};
		case UI_REGION_BOARD:
			return (struct ui_event){UI_EVENT_ACTIVATE, current_focus, {.board={board_x,board_y}}};
		default:
			break;
		}
		/* FALLTHROUGH */
	default:
		return (struct ui_event){UI_EVENT_NONE, current_focus, {0}};
	}
}

static enum color_pair
style_to_cp(enum ui_style s) {
	switch(s) {
	case UI_STYLE_DEFAULT:
		return CP_DEFAULT;
	case UI_STYLE_UNAVAILABLE:
		return CP_UNAVAILABLE;
	case UI_STYLE_BAD:
		return CP_BAD;
	case UI_STYLE_ATTENTION:
		return CP_ATTENTION;
	}
	return CP_DEFAULT;
}

static void
set_style(enum ui_style s, int is_selected) {
	attr_t a;

	a = A_NORMAL;
	if(is_selected) a |= A_REVERSE;
	attrset((attr_t)COLOR_PAIR(style_to_cp(s)) | a);
}

static void
draw_header(struct rect r, char *title, int is_focused) {
	set_style(UI_STYLE_ATTENTION, is_focused);
	mvprintw(r.y, r.x, "%-*.*s", r.w, r.w, title);
}

static unsigned int
wrap_text(char *text, unsigned int max_width, char (*lines)[256], unsigned int cap_lines) {
	unsigned int num_lines;
	char *lf;
	char *line_end;

	if(max_width == 0 || cap_lines == 0) return 0;
	if(max_width > 255) max_width = 255;
	if(! text) return 0;

	num_lines = 0;
	while(*text && num_lines < cap_lines) {
		lf = strchr(text, '\n');
		if(! lf) lf = text + strlen(text);
		if(lf > text + max_width) lf = text + max_width;

		line_end = lf;
		while(*line_end != ' ') {
			if(line_end == text) {
				line_end = lf;
				break;
			}
			--line_end;
		}

		strncpy(lines[num_lines], text, line_end - text);
		lines[num_lines++][line_end - text] = '\0';
		text = line_end + 1;
	}

	return num_lines;
}

static unsigned int
shop_item_height(struct ui_shop_item it) {
	char lines[256][256];

	return wrap_text(it.detail, SHOP_ITEM_W, lines, sizeof(lines)/sizeof(*lines)) + 3;
}

static void
recompute_layout() {
	unsigned int i;
	unsigned int top_height;
	unsigned int item_dim, area_width;

	getmaxyx(stdscr, scr_y, scr_x);

	top_height = num_misc_buttons + 1;
	for(i = 0; i < num_shop_items; ++i) {
		item_dim = shop_item_height(shop_items[i]) + 1;
		if(item_dim > top_height) top_height = item_dim;
	}
	++top_height; /* header */

	area_width = 0;
	for(i = 0; i < num_misc_buttons; ++i) {
		item_dim = strnlen(misc_buttons[i].label, sizeof(misc_buttons[i].label));
		if(item_dim > area_width) area_width = item_dim;
	}
	area_width += 4;

	if(top_height > scr_y) top_height = scr_y;
	if(area_width > scr_x) area_width = scr_x;

	header_rect.x = 0;
	header_rect.y = 0;
	header_rect.w = scr_x;
	if(scr_y >= 1) header_rect.h = 1;
	else header_rect.h = 0;

	misc_rect.x = 0;
	if(scr_y >= 1) misc_rect.y = 1;
	else misc_rect.y = 0;
	misc_rect.w = area_width;
	misc_rect.h = top_height;

	shop_rect.x = area_width;
	if(scr_y >= 1) shop_rect.y = 1;
	else shop_rect.y = 0;
	shop_rect.w = scr_x - area_width;
	shop_rect.h = top_height;

	protocol_rect.x = 0;
	protocol_rect.y = top_height;
	if(PROTOCOL_W < scr_x) protocol_rect.w = PROTOCOL_W;
	else protocol_rect.w = scr_x;
	protocol_rect.h = scr_y - top_height;

	if(PROTOCOL_W < scr_x) board_rect.x = PROTOCOL_W;
	else board_rect.x = scr_x;
	board_rect.y = top_height;
	if(PROTOCOL_W + INSPECT_W < scr_x) board_rect.w = scr_x - PROTOCOL_W - INSPECT_W;
	else board_rect.w = 0;
	board_rect.h = scr_y - top_height;

	if(PROTOCOL_W + INSPECT_W < scr_x) inspect_rect.x = scr_x - INSPECT_W;
	else if(PROTOCOL_W < scr_x) inspect_rect.x = PROTOCOL_W;
	else inspect_rect.x = scr_x;
	inspect_rect.y = top_height;
	if(PROTOCOL_W + INSPECT_W < scr_x) inspect_rect.w = INSPECT_W;
	else if(PROTOCOL_W < scr_x) inspect_rect.w = scr_x - PROTOCOL_W;
	else inspect_rect.w = 0;
	inspect_rect.h = scr_y - top_height;
}

static void
draw_btn(unsigned int x, unsigned int y, struct ui_button btn, int is_selected) {
	set_style(btn.style, is_selected);
	mvprintw(y, x, "[%.*s]", (int)strnlen(btn.label, sizeof(btn.label)), btn.label);
}

static void
draw_misc() {
	unsigned int i;

	draw_header(misc_rect, "MISC", current_focus == UI_REGION_MISC);
	for(i = 0; i < num_misc_buttons; ++i) {
		draw_btn(misc_rect.x, misc_rect.y + 1 + i, misc_buttons[i], current_focus == UI_REGION_MISC && region_selected == i);
	}
}

static void
draw_shop() {
	unsigned int i, j;
	unsigned int cur_x, num_lines;
	struct ui_shop_item it;
	char lines[256][256];

	draw_header(shop_rect, "SHOP", current_focus == UI_REGION_SHOP);

	cur_x = shop_rect.x;
	i = current_focus==UI_REGION_SHOP ? region_scroll : 0;
	while(i < num_shop_items && cur_x + SHOP_ITEM_W < scr_x) {
		it = shop_items[i];
		set_style(it.text_style, 0);
		mvprintw(shop_rect.y+1, cur_x,
				"%-*.*s",
				SHOP_ITEM_W,
				(int)strnlen(it.company, sizeof(it.company)),
				it.company);
		mvprintw(shop_rect.y+2, cur_x,
				"%-*.*s",
				SHOP_ITEM_W,
				(int)strnlen(it.name, sizeof(it.name)),
				it.name);
		num_lines = wrap_text(it.detail, SHOP_ITEM_W, lines, sizeof(lines)/sizeof(*lines));
		for(j = 0; j < num_lines; ++j) {
			mvprintw(shop_rect.y+3+j, cur_x, "%-*s", SHOP_ITEM_W, lines[j]);
		}
		draw_btn(cur_x, shop_rect.y+3+num_lines, it.btn, current_focus == UI_REGION_SHOP && region_selected == i);
		++i;
		cur_x += SHOP_ITEM_W + 2;
	}
}

static void
draw_protocol() {
	unsigned int i, j;
	unsigned int cur_h, num_lines;
	char lines[256][256];

	draw_header(protocol_rect, "PROTOCOL", current_focus == UI_REGION_PROTOCOL);

	cur_h = protocol_rect.y + 1;
	for(i = current_focus == UI_REGION_PROTOCOL ? region_scroll : 0; i < num_protocol_entries; ++i) {
		num_lines = wrap_text(protocol_entries[i].description, PROTOCOL_W, lines, sizeof(lines)/sizeof(*lines));
		set_style(protocol_entries[i].style, current_focus == UI_REGION_PROTOCOL && i == region_selected);
		for(j = 0; j < num_lines; ++j) {
			mvprintw(cur_h + j, protocol_rect.x, "%-s", lines[j]);
		}
		cur_h += num_lines;
	}
}

static void
draw_inspect() {
	unsigned int i;
	unsigned int num_lines;
	char lines[256][256];

	draw_header(inspect_rect, "INSPECT", current_focus == UI_REGION_INSPECT);

	for(i = 0; i < num_inspect_buttons; ++i) {
		draw_btn(inspect_rect.x, inspect_rect.y + 1 + i, inspect_buttons[i], current_focus == UI_REGION_INSPECT && region_selected == i);
	}

	num_lines = wrap_text(inspect_text, INSPECT_W, lines, 256);
	set_style(UI_STYLE_DEFAULT, 0);
	for(i = 0; i < num_lines; ++i) {
		mvprintw(inspect_rect.y + 1 + num_inspect_buttons + i, inspect_rect.x, "%-s", lines[i]);
	}
}

static void
move_focus(int key) {
	switch(current_focus) {
	case UI_REGION_MISC:
		switch(key) {
		case 'j':
			current_focus = UI_REGION_PROTOCOL;
			return;
		case 'l':
			current_focus = UI_REGION_SHOP;
			region_selected = region_scroll = 0;
			return;
		}
		break;
	case UI_REGION_SHOP:
		switch(key) {
		case 'h':
			current_focus = UI_REGION_MISC;
			region_selected = region_scroll = 0;
			return;
		case 'j':
			current_focus = UI_REGION_BOARD;
			board_x = board_y = 0;
			return;
		}
		break;
	case UI_REGION_PROTOCOL:
		switch(key) {
		case 'k':
			current_focus = UI_REGION_MISC;
			region_selected = region_scroll = 0;
			return;
		case 'l':
			current_focus = UI_REGION_BOARD;
			board_x = board_y = 0;
			return;
		}
		break;
	case UI_REGION_BOARD:
		switch(key) {
		case 'h':
			current_focus = UI_REGION_PROTOCOL;
			return;
		case 'k':
			current_focus = UI_REGION_SHOP;
			region_selected = region_scroll = 0;
			return;
		case 'l':
			current_focus = UI_REGION_INSPECT;
			region_selected = region_scroll = 0;
			return;
		}
		break;
	case UI_REGION_INSPECT:
		switch(key) {
		case 'h':
			current_focus = UI_REGION_BOARD;
			board_x = board_y = 0;
			return;
		case 'k':
			current_focus = UI_REGION_SHOP;
			region_selected = region_scroll = 0;
			return;
		}
		break;
	}

	if(key == 'w' || key == 23) {
		switch(current_focus) {
		case UI_REGION_MISC:
			current_focus = UI_REGION_SHOP;
			region_selected = region_scroll = 0;
			return;
		case UI_REGION_SHOP:
			current_focus = UI_REGION_PROTOCOL;
			return;
		case UI_REGION_PROTOCOL:
			current_focus = UI_REGION_BOARD;
			board_x = board_y = 0;
			return;
		case UI_REGION_BOARD:
			current_focus = UI_REGION_INSPECT;
			region_selected = region_scroll = 0;
			return;
		case UI_REGION_INSPECT:
			current_focus = UI_REGION_MISC;
			region_selected = region_scroll = 0;
			return;
		}
	}
}

