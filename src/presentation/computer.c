#include "presentation/computer.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/config.h"

struct ui_board_cell rl_render(struct node *);
void rl_inspect(struct node *);

struct {
	struct ui_board_cell (*render)(struct node *);
	void (*inspect)(struct node *); /* expected to call computer_inspect_text with some text */
} computer_presentation_vt[NUM_COMPUTER_TYPES] = {
	{ rl_render, rl_inspect }, /* computer_risc_labs */
};

void
computer_inspect_text(char *text)
{
	struct ui_button computer_btns[3];

	computer_btns[0] = (struct ui_button){"Edit",UI_STYLE_DEFAULT};
	computer_btns[1] = (struct ui_button){"Reset",UI_STYLE_DEFAULT};
	computer_btns[2] = (struct ui_button){"Flash",UI_STYLE_DEFAULT};

	ui_set_inspect(computer_btns, sizeof(computer_btns)/sizeof(*computer_btns), text);
}

struct ui_board_cell computer_render(struct node *n) {
	struct computer_data *cd;

	cd = n->data;

	return computer_presentation_vt[cd->type].render(n);
}

void computer_inspect(struct node *n) {
	struct computer_data *cd;

	cd = n->data;

	computer_presentation_vt[cd->type].inspect(n);
}

void
computer_interact(struct node *n, unsigned int interaction_id)
{
	(void)n;

	switch(interaction_id) {
	case 0:
		/* EDIT */
		break;
	case 1:
		/* RESET */
		break;
	case 2:
		/* FLASH */
		break;
	default:
		break;
	}
}

