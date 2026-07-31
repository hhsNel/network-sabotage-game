#include "logic/computer.h"

#include <stdio.h>
#include <stdlib.h>

void rl_reset(struct node *n);
void rl_exec(struct node *n);
struct assembly_result rl_flash(struct node *n, char *string);
void rl_destroy(struct node *n);

struct {
	void (*reset)(struct node *);
	void (*exec)(struct node *);
	struct assembly_result (*flash)(struct node *, char *string);
	void (*destroy)(struct node *);
} computer_logic_vt[NUM_COMPUTER_TYPES] = {
	{ rl_reset, rl_exec, rl_flash, rl_destroy }, /* computer_risc_labs */
};

uint8_t
computer_load_byte(struct computer_data *data, size_t addr)
{
	return data->memory[addr % data->mem_sz];
}

uint16_t
computer_load_word(struct computer_data *data, size_t addr)
{
	return
		((uint16_t)computer_load_byte(data, addr+0) << 8) |
		((uint16_t)computer_load_byte(data, addr+1) << 0);
}

void
computer_store_byte(struct computer_data *data, size_t addr, uint8_t value)
{
	data->memory[addr % data->mem_sz] = value;
}

void
computer_store_word(struct computer_data *data, size_t addr, uint16_t value)
{
	computer_store_byte(data, addr+0, value >> 8);
	computer_store_byte(data, addr+1, value >> 0);
}

struct node
create_computer()
{
	struct node n;
	struct computer_data *cd;

	n.type = NODE_COMPUTER;

	n.read_up = n.read_right = n.read_down = n.read_left = NULL;
	n.write_up = n.write_right = n.write_down = n.write_left = NULL;

	cd = (struct computer_data *)malloc(sizeof(struct computer_data));
	if(! cd) {
		fprintf(stderr, "couldn't malloc struct computer_data");
		exit(1);
	}
	n.data = cd;

	cd->memory = NULL;
	cd->mem_sz = 0;
	cd->pc = 0;
	cd->global_clock_frac = 2;
	cd->local_clock = 0;
	cd->data = NULL;

	return n;
}

void
computer_reset(struct node *n)
{
	struct computer_data *cd;

	cd = n->data;
	computer_logic_vt[cd->type].reset(n);
}

void
computer_update(struct node *n)
{
	struct computer_data *cd;

	cd = n->data;

	if(! cd->local_clock) computer_logic_vt[cd->type].exec(n);
	++ cd->local_clock;
	cd->local_clock %= cd->global_clock_frac;
}

struct assembly_result
computer_flash(struct node *n, char *string)
{
	struct computer_data *cd;

	cd = n->data;
	return computer_logic_vt[cd->type].flash(n, string);
}

void
computer_destroy(struct node *n)
{
	struct computer_data *cd;

	cd = n->data;
	computer_logic_vt[cd->type].destroy(n);

	free(n->data);
}

