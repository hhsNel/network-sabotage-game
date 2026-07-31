#ifndef LOGIC_COMPUTER_H
#define LOGIC_COMPUTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "core/computer.h"
#include "logic/node.h"
#include "core/util.h"

struct node create_computer();
void computer_reset(struct node *n);
void computer_update(struct node *n);
struct assembly_result computer_flash(struct node *n, char *string);
void computer_destroy(struct node *n);

#endif

