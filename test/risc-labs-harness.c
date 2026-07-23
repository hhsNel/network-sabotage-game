#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include "risc-labs.h"

int main() {
	char cmd[1024];
	struct node rl;
	struct computer_data *cd;
	struct rl_data *rld;
	size_t sz, sz2;
	uint8_t byte;
	uint16_t word;
	unsigned int i;
	char assembly_line[1024];
	char assembly_buffer[4096];
	size_t assembly_sz;
	struct assembly_result asm_res;

	rl = create_rl_computer(0);
	cd = rl.data;
	rld = cd->data;

	rld->caps = -1; /* hack to enable all instructions */

	printf("READY\n");
	while(1) {
		scanf("%1023[^\n]%*c", cmd);

		if(strcmp(cmd, "reset") == 0) {
			cd->reset(&rl);
			printf("OK\n");
		} else if(strcmp(cmd, "exec") == 0) {
			cd->exec(&rl);
			printf("OK\n");
		} else if(strcmp(cmd, "pc") == 0) {
			printf("OK %zu\n", cd->pc);
		} else if(strncmp(cmd, "store_b", 7) == 0)  {
			if(sscanf(cmd, "store_b %zu %" SCNu8, &sz, &byte) != 2) {
				printf("ERROR\n");
				continue;
			}

			computer_store_byte(cd, sz, byte);
			printf("OK\n");
		} else if(strncmp(cmd, "store_w", 7) == 0)  {
			if(sscanf(cmd, "store_w %zu %" SCNu16, &sz, &word) != 2) {
				printf("ERROR\n");
				continue;
			}

			computer_store_word(cd, sz, word);
			printf("OK\n");
		} else if(strncmp(cmd, "load_b", 6) == 0)  {
			if(sscanf(cmd, "load_b %zu", &sz) != 1) {
				printf("ERROR\n");
				continue;
			}

			printf("OK %" PRIu8 "\n", computer_load_byte(cd, sz));
		} else if(strncmp(cmd, "load_w", 6) == 0)  {
			if(sscanf(cmd, "load_w %zu", &sz) != 1) {
				printf("ERROR\n");
				continue;
			}

			printf("OK %" PRIu16 "\n", computer_load_word(cd, sz));
		} else if(strcmp(cmd, "exit") == 0) {
			printf("OK\n");
			exit(0);
		} else if(strcmp(cmd, "regs") == 0)  {
			printf("OK"
					" %04" PRIx16
					" %04" PRIx16
					" %04" PRIx16
					" %04" PRIx16
					" %04" PRIx16
					" %04" PRIx16
					" %04" PRIx16
					"\n", rld->regs[0], rld->regs[1], rld->regs[2],
					rld->regs[3], rld->regs[4], rld->regs[5], rld->regs[6]);
		} else if(strncmp(cmd, "dump", 4) == 0) {
			if(sscanf(cmd, "dump %zu %zu", &sz, &sz2) != 2) {
				printf("ERROR\n");
				continue;
			}
			if(sz2 == 0) {
				printf("ERROR\n");
				continue;
			}

			printf("OK");
			for(i = 0; i < sz2; ++i) {
				printf(" %02" PRIx8, cd->memory[(sz+i) % cd->mem_sz]);
			}
			printf("\n");
		} else if(strcmp(cmd, "assemble") == 0) {
			assembly_sz = sizeof(assembly_buffer);
			assembly_buffer[0] = '\0';
			while(fgets(assembly_line, sizeof(assembly_line), stdin)) {
				if(strcmp(assembly_line, "\n") == 0) continue;
				if(strcmp(assembly_line, "EOF\n") == 0) break;
				if(strlen(assembly_line) < assembly_sz) {
					strcat(assembly_buffer, assembly_line);
					assembly_sz -= strlen(assembly_line);
				} else {
					break;
				}
			}
			asm_res = rl_asm(cd->memory, cd->mem_sz, assembly_buffer);
			if(asm_res.success) {
				printf("OK\n");
			} else {
				printf("FAIL %u:%u - %63s\n", asm_res.line, asm_res.column, asm_res.reason);
			}
		} else {
			printf("ERROR unknown command: %s\n", cmd);
		}
	}
}

