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
computer_interact(struct node *n, struct presentation_node_data *pn, unsigned int interaction_id)
{
	char editor_cmd[512];
	FILE *file;
	char *asm_buffer;
	long asm_length;

	(void)n;

	switch(interaction_id) {
	case 0:
		/* EDIT */
		snprintf(editor_cmd, sizeof(editor_cmd), editor_fmt, tmpfile_get_name(pn->bound_tmpfile));
		system(editor_cmd);
		break;
	case 1:
		/* RESET */
		/* send client-to-server command to reset this node */
		break;
	case 2:
		/* FLASH */
		file = fopen(tmpfile_get_name(pn->bound_tmpfile), "rb");
		if(file) {
			if(fseek(file, 0, SEEK_END) < 0) {
				fclose(file);
				break;
			}
			asm_length = ftell(file);
			rewind(file);
			asm_buffer = malloc(asm_length);
			if(! asm_buffer) {
				fclose(file);
				break;
			}
			fread(asm_buffer, 1, asm_length, file);
			fclose(file);
			/* send client-to-server flash request with the text to flash in in asm_buffer */
		}
		break;
	default:
		break;
	}
}

