#ifndef CORE_COMPUTER_H
#define CORE_COMPUTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "core/node.h"
#include "core/util.h"

enum {
	COMPUTER_RISC_LABS = 0,
	NUM_COMPUTER_TYPES,
};
typedef uint16_t computer_type;

struct computer_data {
	computer_type type;
	uint8_t *memory;
	size_t mem_sz;
	size_t pc;
	uint8_t global_clock_frac;
	uint8_t local_clock;
	void *data;
};

uint8_t computer_load_byte(struct computer_data *data, size_t addr);
uint16_t computer_load_word(struct computer_data *data, size_t addr);
void computer_store_byte(struct computer_data *data, size_t addr, uint8_t value);
void computer_store_word(struct computer_data *data, size_t addr, uint16_t value);

#endif

