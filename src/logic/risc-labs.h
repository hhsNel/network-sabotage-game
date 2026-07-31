#ifndef LOGIC_RISC_LABS_H
#define LOGIC_RISC_LABS_H

#include "core/risc-labs.h"

void rl_reset(struct node *n);
void rl_exec(struct node *n);
struct assembly_result rl_flash(struct node *n, char *string);
void rl_destroy(struct node *n);

struct node create_rl_computer(unsigned int points);

#endif

