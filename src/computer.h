#ifndef COMPUTER_H
#define COMPUTER_H

#include <stdint.h>
#include <stddef.h>

#include "node.h"

struct computer_data {
	uint8_t *memory;
	size_t mem_sz;
	size_t pc;
	uint8_t global_clock_frac;
	uint8_t local_clock;
	void (*reset)(struct node *);
	void (*exec)(struct node *);
	void (*destroy)(struct node *);
	void *data;
};

uint8_t computer_load_byte(struct computer_data *data, size_t addr);
uint16_t computer_load_word(struct computer_data *data, size_t addr);
void computer_store_byte(struct computer_data *data, size_t addr, uint8_t value);
void computer_store_word(struct computer_data *data, size_t addr, uint16_t value);
struct node create_computer();

#endif

