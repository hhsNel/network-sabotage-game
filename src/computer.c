#include "computer.h"

#include <stdio.h>
#include <stdlib.h>

static void computer_update(struct node *n);
static void computer_destroy(struct node *n);

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
	n.up = n.right = n.down = n.left = NULL;
	n.update = computer_update;
	n.destroy = computer_destroy;

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
	cd->reset = NULL;
	cd->exec = NULL;
	cd->destroy = NULL;
	cd->data = NULL;

	return n;
}

static void
computer_update(struct node *n)
{
	struct computer_data *cd;

	cd = (struct computer_data *)n->data;
	cd->exec(n);
}

static void
computer_destroy(struct node *n)
{
	struct computer_data *cd;

	cd = (struct computer_data *)n->data;
	cd->exec(n);
}

