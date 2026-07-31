#include "core/risc-labs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core/util.h"
#include "logic/computer.h"

enum rl_asm_token_type { RL_ASM_TK_MNEMONIC, RL_ASM_TK_REG, RL_ASM_TK_PORT, RL_ASM_TK_COND, RL_ASM_TK_NUMBER, RL_ASM_TK_EQU, RL_ASM_TK_IDENT, RL_ASM_TK_COLON, RL_ASM_TK_EOF, RL_ASM_TK_ERROR };
#define RL_PSEUDOOP_DB (-1)
#define RL_PSEUDOOP_DW (-2)
#define RL_PSEUDOOP_JMP (-3)
#define RL_PSEUDOOP_JREL (-4)
struct rl_asm_token {
	enum rl_asm_token_type type;
	union {
		enum rl_opcode mnemonic;
		enum rl_reg reg;
		enum rl_port port;
		enum rl_condition cond;
		uint16_t number;
		struct {
			char *begin;
			size_t length;
		} ident;
	} data;
	unsigned int line, column;
};

static struct rl_asm_token next_tk(char **in_str, unsigned int *line, unsigned int *column);

struct assembly_result
rl_asm(uint8_t *out_buf, size_t out_size, char *in_str)
{
	struct rl_asm_token *tokens;
	unsigned int num_tokens, cap_tokens;
	struct assembly_result res;
	struct symbol { char *begin; size_t length; uint16_t value; };
	struct symbol *symbol_table;
	unsigned int num_symbols, cap_symbols;
	enum operand_type { OP_REGISTER, OP_PORT, OP_NUMBER, OP_RELATIVE, OP_COND, OP_NONE };
	struct operand { enum operand_type type; uint8_t start_index; uint8_t bit_width; };
	struct instr_shape { enum rl_opcode mnemonic; uint16_t template; struct operand operands[3]; };
	static struct instr_shape instruction_shapes[] = {
#define REGISTER(IDX) { OP_REGISTER, (IDX), 3 }
#define PORT(IDX) { OP_PORT, (IDX), 3 }
#define NUMBER(IDX,WIDTH) { OP_NUMBER, (IDX), (WIDTH) }
#define RELATIVE(IDX,WIDTH) { OP_RELATIVE, (IDX), (WIDTH) }
#define CONDITION(IDX) { OP_COND, (IDX), 3 }
#define NONE { OP_NONE, 0, 0 }
		{ RL_MOV,           0x0000, { REGISTER(5),   REGISTER(8),    NONE } },
		{ RL_ADD,           0x0001, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_SUB,           0x0002, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_MUL,           0x0800, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_DIV,           0x0801, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_MOD,           0x0802, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_AND,           0x1000, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_OR,            0x1001, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_NAND,          0x1800, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_NOR,           0x1801, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_XOR,           0x1802, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_XNOR,          0x1803, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_SHL,           0x2000, { REGISTER(5),   REGISTER(8),    NUMBER(12,4) } },
		{ RL_SHR,           0x2010, { REGISTER(5),   REGISTER(8),    NUMBER(12,4) } },
		{ RL_BSH,           0x2800, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_ABSH,          0x2801, { REGISTER(5),   REGISTER(8),    REGISTER(11) } },
		{ RL_XCH,           0x3000, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_TST,           0x3800, { REGISTER(8),   NONE,           NONE } },
		{ RL_CMP,           0x3801, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_IMM,           0x8000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_IMMS,          0x9000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_LOADBR,        0xF000, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_LOADBI,        0xA000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_LOADSR,        0xF200, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_LOADSI,        0xB000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_LOADWR,        0xF400, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_LOADWI,        0xC000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_STOREBR,       0xF600, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_STOREBI,       0xD000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_STOREWR,       0xF800, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_STOREWI,       0xE000, { REGISTER(4),   NUMBER(7,9),    NONE } },
		{ RL_MXCH,          0xFA00, { REGISTER(8),   REGISTER(11),   NONE } },
		{ RL_Jc,            0x4000, { CONDITION(5),  RELATIVE(8,8), NONE } },
		{ RL_JRELc,         0x4800, { CONDITION(5),  REGISTER(8),   NUMBER(11,5) } },
		{ RL_JEc,           0x5000, { NUMBER(5,4),   RELATIVE(10,6), NONE } },
		{ RL_JENc,          0x5040, { NUMBER(5,4),   RELATIVE(10,6), NONE } },
		{ RL_JAc,           0x5800, { NUMBER(5,4),   RELATIVE(10,6), NONE } },
		{ RL_JANc,          0x5840, { NUMBER(5,4),   RELATIVE(10,6), NONE } },
		{ RL_IN,            0x6000, { REGISTER(5),   PORT(8),        NONE } },
		{ RL_OUT,           0x6800, { REGISTER(5),   PORT(8),        NONE } },
		{ RL_INM,           0x7000, { REGISTER(5),   PORT(8),        NONE } },
		{ RL_OUTM,          0x7800, { REGISTER(5),   PORT(8),        NONE } },
		/* pseudo-ops */
		{ RL_PSEUDOOP_DB,   0x0000, { NUMBER(8,8),   NONE,           NONE } },
		{ RL_PSEUDOOP_DW,   0x0000, { NUMBER(0,16),  NONE,           NONE } },
		{ RL_PSEUDOOP_JMP,  0x4000, { RELATIVE(8,8), NONE,           NONE } },
		{ RL_PSEUDOOP_JREL, 0x4800, { REGISTER(8),   NUMBER(11,5),   NONE } },
#undef REGISTER
#undef PORT
#undef NUMBER
#undef RELATIVE
#undef NONE
	};
	struct instr_record { struct instr_shape shape; struct rl_asm_token operands[3]; };
	struct instr_record *instr_records;
	unsigned int num_instr_records, cap_instr_records;
	unsigned int i, j;
	uint16_t assembled_instr, instr_part, opcode_bitmask;

	tokens = NULL;
	num_tokens = cap_tokens = 0;
	res.line = res.column = 0;
	do {
		if(num_tokens == cap_tokens) {
			cap_tokens += 16;
			tokens = realloc(tokens, cap_tokens * sizeof(struct rl_asm_token));
			if(! tokens) {
				fprintf(stderr, "couldn't realloc tokens for more (%u)\n", cap_tokens);
				exit(1);
			}
		}
		tokens[num_tokens] = next_tk(&in_str, &res.line, &res.column);
		if(tokens[num_tokens].type == RL_ASM_TK_ERROR) {
			res.success = 0;
			snprintf(res.reason, sizeof(res.reason), "unknown token around %s", in_str);
			return res;
		}
	} while(tokens[num_tokens++].type != RL_ASM_TK_EOF);

	instr_records = NULL;
	num_instr_records = cap_instr_records = 0;
	symbol_table = NULL;
	num_symbols = cap_symbols = 0;
	i = 0;
	while(i < num_tokens && tokens[i].type != RL_ASM_TK_EOF) {
		switch(tokens[i].type) {
		case RL_ASM_TK_IDENT:
			if(num_symbols == cap_symbols) {
				cap_symbols += 16;
				symbol_table = realloc(symbol_table, cap_symbols * sizeof(struct symbol));
				if(! symbol_table) {
					fprintf(stderr, "couldn't realloc symbol_table for more (%u)\n", cap_symbols);
					exit(1);
				}
			}
			for(j = 0; j < num_symbols; ++j) {
				if( symbol_table[j].length == tokens[i].data.ident.length &&
					strncmp(symbol_table[j].begin, tokens[i].data.ident.begin, tokens[i].data.ident.length) == 0 ) {
					res.success = 0;
					res.line = tokens[i].line;
					res.column = tokens[i].column;
					snprintf(res.reason, sizeof(res.reason), "symbol %.*s redefined", (int)symbol_table[j].length, symbol_table[j].begin);
					return res;
				}
			}
			switch(tokens[i+1].type) {
			case RL_ASM_TK_COLON:
				symbol_table[num_symbols] = (struct symbol){ tokens[i].data.ident.begin, tokens[i].data.ident.length, 2 * num_instr_records };
				i += 2;
				break;
			case RL_ASM_TK_EQU:
				if(tokens[i+2].type != RL_ASM_TK_NUMBER) {
					res.success = 0;
					res.line = tokens[i+2].line;
					res.column = tokens[i+2].column;
					snprintf(res.reason, sizeof(res.reason), "expected NUMBER after IDENT EQU");
					return res;
				}
				symbol_table[num_symbols] = (struct symbol){ tokens[i].data.ident.begin, tokens[i].data.ident.length, tokens[i+1].data.number };
				i += 3;
				break;
			default:
				res.success = 0;
				res.line = tokens[i+1].line;
				res.column = tokens[i+1].column;
				snprintf(res.reason, sizeof(res.reason), "expected COLON or EQU after IDENT");
				return res;
			}
			++num_symbols;
			break;
		case RL_ASM_TK_MNEMONIC:
			for(j = 0; j < sizeof(instruction_shapes)/sizeof(*instruction_shapes); ++j) {
				if(tokens[i].data.mnemonic == instruction_shapes[j].mnemonic) break;
			}
			if(j == sizeof(instruction_shapes)/sizeof(*instruction_shapes)) {
				res.success = 0;
				res.line = tokens[i].line;
				res.column = tokens[i].column;
				snprintf(res.reason, sizeof(res.reason), "unknown instruction shape");
				return res;
			}
			if(num_instr_records == cap_instr_records) {
				cap_instr_records += 16;
				instr_records = realloc(instr_records, cap_instr_records * sizeof(struct instr_record));
				if(! instr_records) {
					fprintf(stderr, "couldn't realloc instr_records for more (%u)\n", cap_instr_records);
					exit(1);
				}
			}
			instr_records[num_instr_records].shape = instruction_shapes[j];
			++i;
#define HANDLE_OPERAND(OP_IDX) \
			switch(instruction_shapes[j].operands[OP_IDX].type) { \
			case OP_REGISTER: \
				if(tokens[i].type != RL_ASM_TK_REG) { res.success=0; res.line=tokens[i].line; res.column=tokens[i].column; snprintf(res.reason,sizeof(res.reason),"expected register as operand"); return res; } \
				instr_records[num_instr_records].operands[OP_IDX] = tokens[i]; \
				++i; \
				break; \
			case OP_PORT: \
				if(tokens[i].type != RL_ASM_TK_PORT) { res.success=0; res.line=tokens[i].line; res.column=tokens[i].column; snprintf(res.reason,sizeof(res.reason),"expected port as operand"); return res; } \
				instr_records[num_instr_records].operands[OP_IDX] = tokens[i]; \
				++i; \
				break; \
			case OP_NUMBER: \
			case OP_RELATIVE: \
				if(tokens[i].type != RL_ASM_TK_NUMBER && tokens[i].type != RL_ASM_TK_IDENT) { \
					res.success=0; \
					res.line=tokens[i].line; \
					res.column=tokens[i].column; \
					snprintf(res.reason,sizeof(res.reason),"expected number or identifier as operand"); \
					return res; \
				} \
				instr_records[num_instr_records].operands[OP_IDX] = tokens[i]; \
				++i; \
				break; \
			case OP_COND: \
				if(tokens[i].type != RL_ASM_TK_COND) { res.success=0; res.line=tokens[i].line; res.column=tokens[i].column; snprintf(res.reason,sizeof(res.reason),"expected condition as operand"); return res; } \
				instr_records[num_instr_records].operands[OP_IDX] = tokens[i]; \
				++i; \
				break; \
			case OP_NONE: \
				break; \
			}
			HANDLE_OPERAND(0);
			HANDLE_OPERAND(1);
			HANDLE_OPERAND(2);
#undef HANDLE_OPERAND
			++num_instr_records;
			break;
		default:
			res.success = 0;
			res.line = tokens[i].line;
			res.column = tokens[i].column;
			snprintf(res.reason, sizeof(res.reason), "expected IDENT or MNEMONIC in global ctx");
			return res;
		}
	}

	memset(out_buf, 0, out_size);
	for(i = 0; i < num_instr_records; ++i) {
		if(out_size < 2) {
			res.success = 0;
			res.line = 0;
			res.column = 0;
			snprintf(res.reason, sizeof(res.reason), "ran out of space for instructions");
			return res;
		}

		assembled_instr = instr_records[i].shape.template;
#define HANDLE_OPERAND(IDX) \
		if(instr_records[i].shape.operands[IDX].type != OP_NONE) { \
			switch(instr_records[i].operands[IDX].type) { \
			case RL_ASM_TK_REG: \
				instr_part = instr_records[i].operands[IDX].data.reg; \
				break; \
			case RL_ASM_TK_PORT: \
				instr_part = instr_records[i].operands[IDX].data.port; \
				break; \
			case RL_ASM_TK_COND: \
				instr_part = instr_records[i].operands[IDX].data.cond; \
				break; \
			case RL_ASM_TK_IDENT: \
				for(j = 0; j < num_symbols; ++j) { \
					if( instr_records[i].operands[IDX].data.ident.length == symbol_table[j].length && \
						strncmp(instr_records[i].operands[IDX].data.ident.begin, \
						symbol_table[j].begin, \
						symbol_table[j].length) == 0 ) { \
						break; \
					} \
				} \
				if(j == num_symbols) { \
					res.success = 0; \
					res.line = instr_records[i].operands[IDX].line; \
					res.column = instr_records[i].operands[IDX].column; \
					snprintf(res.reason, sizeof(res.reason), "couldn't find symbol: %.*s", \
							(int)instr_records[i].operands[IDX].data.ident.length, \
							instr_records[i].operands[IDX].data.ident.begin); \
					return res; \
				} \
				instr_part = symbol_table[j].value; \
				if(instr_records[i].shape.operands[IDX].type == OP_RELATIVE) instr_part -= 2 * (i + 1); \
				break; \
			case RL_ASM_TK_NUMBER: \
				instr_part = instr_records[i].operands[IDX].data.number; \
				break; \
			default: exit(1); /* something went very wrong */ \
			} \
			opcode_bitmask = ((1 << instr_records[i].shape.operands[IDX].bit_width) - 1); \
			if( (instr_part & opcode_bitmask) != instr_part && \
				(instr_part & (uint16_t)~opcode_bitmask) != (uint16_t)~opcode_bitmask ) { \
				res.success = 0; \
				res.line = instr_records[i].operands[IDX].line; \
				res.column = instr_records[i].operands[IDX].column; \
				snprintf(res.reason, sizeof(res.reason), "operand too big (bitwise)"); \
				return res; \
			} \
			assembled_instr |= instr_part << (16 - instr_records[i].shape.operands[IDX].start_index - instr_records[i].shape.operands[IDX].bit_width); \
		}
		HANDLE_OPERAND(0);
		HANDLE_OPERAND(1);
		HANDLE_OPERAND(2);
#undef HANDLE_OPERAND
		if(instr_records[i].shape.mnemonic != RL_PSEUDOOP_DB) {
			out_buf[0] = assembled_instr >> 8;
			out_buf[1] = assembled_instr;
			out_buf += 2;
			out_size -= 2;
		} else {
			*out_buf = assembled_instr & 0x00FF;
			++out_buf;
			--out_size;
		}
	}

	res.success = 1;
	return res;
}

static struct rl_asm_token
next_tk(char **in_str, unsigned int *line, unsigned int *column)
{
	struct tk_translation { char *str; struct rl_asm_token translation; };
	static struct tk_translation tk_lookup[] = {
		{ "mov",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MOV},0,0} },
		{ "MOV",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MOV},0,0} },
		{ "add",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_ADD},0,0} },
		{ "ADD",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_ADD},0,0} },
		{ "sub",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SUB},0,0} },
		{ "SUB",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SUB},0,0} },
		{ "mul",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MUL},0,0} },
		{ "MUL",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MUL},0,0} },
		{ "div",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_DIV},0,0} },
		{ "DIV",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_DIV},0,0} },
		{ "mod",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MOD},0,0} },
		{ "MOD",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MOD},0,0} },
		{ "and",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_AND},0,0} },
		{ "AND",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_AND},0,0} },
		{ "or",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OR},0,0} },
		{ "OR",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OR},0,0} },
		{ "nand",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_NAND},0,0} },
		{ "NAND",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_NAND},0,0} },
		{ "nor",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_NOR},0,0} },
		{ "NOR",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_NOR},0,0} },
		{ "xor",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XOR},0,0} },
		{ "XOR",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XOR},0,0} },
		{ "xnor",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XNOR},0,0} },
		{ "XNOR",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XNOR},0,0} },
		{ "shl",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SHL},0,0} },
		{ "SHL",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SHL},0,0} },
		{ "shr",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SHR},0,0} },
		{ "SHR",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_SHR},0,0} },
		{ "bsh",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_BSH},0,0} },
		{ "BSH",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_BSH},0,0} },
		{ "absh",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_ABSH},0,0} },
		{ "ABSH",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_ABSH},0,0} },
		{ "xch",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XCH},0,0} },
		{ "XCH",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_XCH},0,0} },
		{ "tst",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_TST},0,0} },
		{ "TST",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_TST},0,0} },
		{ "cmp",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_CMP},0,0} },
		{ "CMP",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_CMP},0,0} },
		{ "imm",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IMM},0,0} },
		{ "IMM",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IMM},0,0} },
		{ "imms",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IMMS},0,0} },
		{ "IMMS",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IMMS},0,0} },
		{ "loadbr",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADBR},0,0} },
		{ "LOADBR",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADBR},0,0} },
		{ "loadbi",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADBI},0,0} },
		{ "LOADBI",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADBI},0,0} },
		{ "loadsr",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADSR},0,0} },
		{ "LOADSR",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADSR},0,0} },
		{ "loadsi",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADSI},0,0} },
		{ "LOADSI",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADSI},0,0} },
		{ "loadwr",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADWR},0,0} },
		{ "LOADWR",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADWR},0,0} },
		{ "loadwi",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADWI},0,0} },
		{ "LOADWI",    {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_LOADWI},0,0} },
		{ "storebr",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREBR},0,0} },
		{ "STOREBR",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREBR},0,0} },
		{ "storebi",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREBI},0,0} },
		{ "STOREBI",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREBI},0,0} },
		{ "storewr",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREWR},0,0} },
		{ "STOREWR",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREWR},0,0} },
		{ "storewi",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREWI},0,0} },
		{ "STOREWI",   {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_STOREWI},0,0} },
		{ "mxch",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MXCH},0,0} },
		{ "MXCH",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_MXCH},0,0} },
		{ "jc",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_Jc},0,0} },
		{ "Jc",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_Jc},0,0} },
		{ "JC",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_Jc},0,0} },
		{ "jrelc",     {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JRELc},0,0} },
		{ "JRELc",     {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JRELc},0,0} },
		{ "JRELC",     {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JRELc},0,0} },
		{ "jec",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JEc},0,0} },
		{ "JEc",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JEc},0,0} },
		{ "JEC",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JEc},0,0} },
		{ "jenc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JENc},0,0} },
		{ "JENc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JENc},0,0} },
		{ "JENC",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JENc},0,0} },
		{ "jac",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JAc},0,0} },
		{ "JAc",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JAc},0,0} },
		{ "JAC",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JAc},0,0} },
		{ "janc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JANc},0,0} },
		{ "JANc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JANc},0,0} },
		{ "JANC",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JANc},0,0} },
		{ "in",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IN},0,0} },
		{ "IN",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IN},0,0} },
		{ "out",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUT},0,0} },
		{ "OUT",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUT},0,0} },
		{ "inm",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_INM},0,0} },
		{ "INM",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_INM},0,0} },
		{ "outm",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUTM},0,0} },
		{ "OUTM",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUTM},0,0} },
		/* pseudo-ops */
		{ "db",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_DB},0,0} },
		{ "DB",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_DB},0,0} },
		{ "dw",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_DW},0,0} },
		{ "DW",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_DW},0,0} },
		{ "jmp",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_JMP},0,0} },
		{ "JMP",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_JMP},0,0} },
		{ "jrel",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_JREL},0,0} },
		{ "JREL",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_PSEUDOOP_JREL},0,0} },
		/* end of pseudo-ops */
		{ "r0",        {RL_ASM_TK_REG,{.reg=RL_REG_R0},0,0} },
		{ "R0",        {RL_ASM_TK_REG,{.reg=RL_REG_R0},0,0} },
		{ "r1",        {RL_ASM_TK_REG,{.reg=RL_REG_R1},0,0} },
		{ "R1",        {RL_ASM_TK_REG,{.reg=RL_REG_R1},0,0} },
		{ "r2",        {RL_ASM_TK_REG,{.reg=RL_REG_R2},0,0} },
		{ "R2",        {RL_ASM_TK_REG,{.reg=RL_REG_R2},0,0} },
		{ "r3",        {RL_ASM_TK_REG,{.reg=RL_REG_R3},0,0} },
		{ "R3",        {RL_ASM_TK_REG,{.reg=RL_REG_R3},0,0} },
		{ "r4",        {RL_ASM_TK_REG,{.reg=RL_REG_R4},0,0} },
		{ "R4",        {RL_ASM_TK_REG,{.reg=RL_REG_R4},0,0} },
		{ "r5",        {RL_ASM_TK_REG,{.reg=RL_REG_R5},0,0} },
		{ "R5",        {RL_ASM_TK_REG,{.reg=RL_REG_R5},0,0} },
		{ "status",    {RL_ASM_TK_REG,{.reg=RL_REG_STATUS},0,0} },
		{ "STATUS",    {RL_ASM_TK_REG,{.reg=RL_REG_STATUS},0,0} },
		{ "zero",      {RL_ASM_TK_REG,{.reg=RL_REG_ZERO},0,0} },
		{ "ZERO",      {RL_ASM_TK_REG,{.reg=RL_REG_ZERO},0,0} },
		{ "up",        {RL_ASM_TK_PORT,{.port=RL_PORT_UP},0,0} },
		{ "UP",        {RL_ASM_TK_PORT,{.port=RL_PORT_UP},0,0} },
		{ "right",     {RL_ASM_TK_PORT,{.port=RL_PORT_RIGHT},0,0} },
		{ "RIGHT",     {RL_ASM_TK_PORT,{.port=RL_PORT_RIGHT},0,0} },
		{ "down",      {RL_ASM_TK_PORT,{.port=RL_PORT_DOWN},0,0} },
		{ "DOWN",      {RL_ASM_TK_PORT,{.port=RL_PORT_DOWN},0,0} },
		{ "left",      {RL_ASM_TK_PORT,{.port=RL_PORT_LEFT},0,0} },
		{ "LEFT",      {RL_ASM_TK_PORT,{.port=RL_PORT_LEFT},0,0} },
		{ "any",       {RL_ASM_TK_PORT,{.port=RL_PORT_ANY},0,0} },
		{ "ANY",       {RL_ASM_TK_PORT,{.port=RL_PORT_ANY},0,0} },
		{ "last",      {RL_ASM_TK_PORT,{.port=RL_PORT_LAST},0,0} },
		{ "LAST",      {RL_ASM_TK_PORT,{.port=RL_PORT_LAST},0,0} },
		{ "opposite",  {RL_ASM_TK_PORT,{.port=RL_PORT_OPPOSITE},0,0} },
		{ "OPPOSITE",  {RL_ASM_TK_PORT,{.port=RL_PORT_OPPOSITE},0,0} },
		{ "clockwise", {RL_ASM_TK_PORT,{.port=RL_PORT_CLOCKWISE},0,0} },
		{ "CLOCKWISE", {RL_ASM_TK_PORT,{.port=RL_PORT_CLOCKWISE},0,0} },
		{ "always",    {RL_ASM_TK_COND,{.cond=RL_COND_ALWAYS},0,0} },
		{ "ALWAYS",    {RL_ASM_TK_COND,{.cond=RL_COND_ALWAYS},0,0} },
		{ "zro",       {RL_ASM_TK_COND,{.cond=RL_COND_ZERO},0,0} },
		{ "ZRO",       {RL_ASM_TK_COND,{.cond=RL_COND_ZERO},0,0} },
		{ "neg",       {RL_ASM_TK_COND,{.cond=RL_COND_NEG},0,0} },
		{ "NEG",       {RL_ASM_TK_COND,{.cond=RL_COND_NEG},0,0} },
		{ "carry",     {RL_ASM_TK_COND,{.cond=RL_COND_CARRY},0,0} },
		{ "CARRY",     {RL_ASM_TK_COND,{.cond=RL_COND_CARRY},0,0} },
		{ "no_carry",  {RL_ASM_TK_COND,{.cond=RL_COND_NO_CARRY},0,0} },
		{ "NO_CARRY",  {RL_ASM_TK_COND,{.cond=RL_COND_NO_CARRY},0,0} },
		{ "overflow",  {RL_ASM_TK_COND,{.cond=RL_COND_OVERFLOW},0,0} },
		{ "OVERFLOW",  {RL_ASM_TK_COND,{.cond=RL_COND_OVERFLOW},0,0} },
		{ "nonzero",   {RL_ASM_TK_COND,{.cond=RL_COND_NONZERO},0,0} },
		{ "NONZERO",   {RL_ASM_TK_COND,{.cond=RL_COND_NONZERO},0,0} },
		{ "pos",       {RL_ASM_TK_COND,{.cond=RL_COND_POS},0,0} },
		{ "POS",       {RL_ASM_TK_COND,{.cond=RL_COND_POS},0,0} },
		{ "equ",       {RL_ASM_TK_EQU,{0},0,0} },
		{ "EQU",       {RL_ASM_TK_EQU,{0},0,0} },
		{ ":",         {RL_ASM_TK_COLON,{0},0,0} },
	};
	unsigned int i;
	uint16_t num;
	unsigned int begin_col;
	struct rl_asm_token tk;
	char next_c;

	while(**in_str == ' ' || **in_str == '\t' || **in_str == '\n') {
		if(**in_str == '\n') {
			*column = 0;
			++ *line;
		} else {
			++ *column;
		}
		++ *in_str;
	}
	if(! **in_str) return (struct rl_asm_token){RL_ASM_TK_EOF,{0},0,0};

	for(i = 0; i < sizeof(tk_lookup)/sizeof(*tk_lookup); ++i) {
		if(strncmp(*in_str, tk_lookup[i].str, strlen(tk_lookup[i].str)) == 0) {
			next_c = (*in_str)[strlen(tk_lookup[i].str)];
			if( (next_c >= 'a' && next_c <= 'z') ||
				(next_c >= 'A' && next_c <= 'Z') ||
				(next_c >= '0' && next_c <= '9') ||
				next_c == '_' ) continue;
			begin_col = *column;
			*in_str += strlen(tk_lookup[i].str);
			*column += strlen(tk_lookup[i].str);
			tk = tk_lookup[i].translation;
			tk.line = *line;
			tk.column = begin_col;
			return tk;
		}
	}

	if( (*in_str)[0] == '0' && (*in_str)[1] == 'x' &&
			(((*in_str)[2] >= '0' && (*in_str)[2] <= '9') ||
			 ((*in_str)[2] >= 'a' && (*in_str)[2] <= 'f') ||
			 ((*in_str)[2] >= 'A' && (*in_str)[2] <= 'F')) ) {
		num = 0;
		begin_col = *column;
		*in_str += 2;
		*column += 2;
		while(
				(**in_str >= '0' && **in_str <= '9') ||
				(**in_str >= 'a' && **in_str <= 'f') ||
				(**in_str >= 'A' && **in_str <= 'F')) {
			num *= 16;
			if(**in_str >= '0' && **in_str <= '9') num += **in_str - '0';
			else if(**in_str >= 'a' && **in_str <= 'f') num += **in_str - 'a' + 10;
			else num += **in_str - 'A' + 10;
			++ *in_str;
			++ *column;
		}
		tk.type = RL_ASM_TK_NUMBER;
		tk.data.number = num;
		tk.line = *line;
		tk.column = begin_col;
		return tk;
	}

	if( (*in_str)[0] == '0' && (*in_str)[1] == 'b' &&
			((*in_str)[2] == '0' || (*in_str)[2] == '1') ) {
		num = 0;
		begin_col = *column;
		*in_str += 2;
		*column += 2;
		while(**in_str == '0' || **in_str == '1') {
			num *= 2;
			num += **in_str - '0';
			++ *in_str;
			++ *column;
		}
		tk.type = RL_ASM_TK_NUMBER;
		tk.data.number = num;
		tk.line = *line;
		tk.column = begin_col;
		return tk;
	}

	if( **in_str >= '0' && **in_str <= '9' ) {
		num = 0;
		begin_col = *column;
		while(**in_str >= '0' && **in_str <= '9') {
			num *= 10;
			num += **in_str - '0';
			++ *in_str;
			++ *column;
		}
		tk.type = RL_ASM_TK_NUMBER;
		tk.data.number = num;
		tk.line = *line;
		tk.column = begin_col;
		return tk;
	}

	if( (**in_str >= 'a' && **in_str <= 'z') ||
		(**in_str >= 'A' && **in_str <= 'Z') ||
		**in_str == '_' ) {
		tk.data.ident.begin = *in_str;
		tk.data.ident.length = 0;
		begin_col = *column;
		while( (**in_str >= 'a' && **in_str <= 'z') ||
			(**in_str >= 'A' && **in_str <= 'Z') ||
			**in_str == '_' ||
			(**in_str >= '0' && **in_str <= '9') ) {
			++ *in_str;
			++ *column;
			++ tk.data.ident.length;
		}
		tk.type = RL_ASM_TK_IDENT;
		tk.line = *line;
		tk.column = begin_col;
		return tk;
	}

	return (struct rl_asm_token){RL_ASM_TK_ERROR,{0},0,0};
}

struct decoded_instr
decode(uint16_t raw_instr)
{
#define EXTRACT(POS,BITS) \
	((raw_instr & ((0x8000 >> (POS) << 1) - 1)) >> (16 - (POS) - (BITS)))
#define ILLEGAL_INSTR (struct decoded_instr){.op=RL_ILLEGAL_INSTR}

	if(raw_instr & 0x8000) {
		/* immediate/memory */
		switch(EXTRACT(1, 3)) {
		case 0x0:
			/* IMM */
			return (struct decoded_instr){ RL_IMM, {.im_di={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x1:
			/* IMMS */
			return (struct decoded_instr){ RL_IMMS, {.im_di={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x2:
			/* LOADBI */
			return (struct decoded_instr){ RL_LOADBI, {.im_da={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x3:
			/* LOADSI */
			return (struct decoded_instr){ RL_LOADSI, {.im_da={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x4:
			/* LOADWI */
			return (struct decoded_instr){ RL_LOADWI, {.im_da={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x5:
			/* STOREBI */
			return (struct decoded_instr){ RL_STOREBI, {.im_sa={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x6:
			/* STOREWI */
			return (struct decoded_instr){ RL_STOREWI, {.im_sa={ EXTRACT(4, 3), EXTRACT(7, 9) }} };
		case 0x7:
			switch(EXTRACT(4, 3)) {
			case 0x0:
				/* LOADBR */
				return (struct decoded_instr){ RL_LOADBR, {.im_ds={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x1:
				/* LOADSR */
				return (struct decoded_instr){ RL_LOADSR, {.im_ds={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x2:
				/* LOADWR */
				return (struct decoded_instr){ RL_LOADWR, {.im_ds={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x3:
				/* STOREBR */
				return (struct decoded_instr){ RL_STOREBR, {.im_ds={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x4:
				/* STOREWR */
				return (struct decoded_instr){ RL_STOREWR, {.im_ds={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x5:
				/* MXCH */
				return (struct decoded_instr){ RL_MXCH, {.im_ab={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			default:
				return ILLEGAL_INSTR;
			}
		}
	} else {
		if(raw_instr & 0x4000) {
			/* jump or IO */
			if(raw_instr & 0x2000) {
				/* IO */
				switch(EXTRACT(3, 2)) {
				case 0x0:
					/* IN */
					return (struct decoded_instr){ RL_IN, {.io_dp={ EXTRACT(5, 3), EXTRACT(8, 3) }} };
				case 0x1:
					/* OUT */
					return (struct decoded_instr){ RL_OUT, {.io_sp={ EXTRACT(5, 3), EXTRACT(8, 3) }} };
				case 0x2:
					/* INM */
					return (struct decoded_instr){ RL_INM, {.io_dp={ EXTRACT(5, 3), EXTRACT(8, 3) }} };
				case 0x3:
					/* OUTM */
					return (struct decoded_instr){ RL_OUTM, {.io_sp={ EXTRACT(5, 3), EXTRACT(8, 3) }} };
				}
			} else {
				/* jump */
				switch(EXTRACT(3, 2)) {
				case 0x0:
					/* Jc */
					return (struct decoded_instr){ RL_Jc, {.j_cr={ EXTRACT(5, 3), EXTRACT(8, 8) }} };
				case 0x1:
					/* JRELc */
					return (struct decoded_instr){ RL_JRELc, {.j_cob={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 5) }} };
				case 0x2:
					/* JE(N)c */
					if(raw_instr & 0x0040) {
						/* JENc */
						return (struct decoded_instr){ RL_JENc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					} else {
						/* JEc */
						return (struct decoded_instr){ RL_JEc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					}
				case 0x3:
					/* JA(N)c */
					if(raw_instr & 0x0040) {
						/* JANc */
						return (struct decoded_instr){ RL_JANc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					} else {
						/* JAc */
						return (struct decoded_instr){ RL_JAc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					}
				}
			}
		} else {
			/* reg */
			switch(EXTRACT(2, 3)) {
			case 0x0:
				/* simple arithmetic */
				switch(EXTRACT(14, 2)) {
				case 0x0:
					/* MOV */
					return (struct decoded_instr){ RL_MOV, {.reg_ds={ EXTRACT(5, 3), EXTRACT(8, 3) }} };
				case 0x1:
					/* ADD */
					return (struct decoded_instr){ RL_ADD, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x2:
					/* SUB */
					return (struct decoded_instr){ RL_SUB, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				default:
					return ILLEGAL_INSTR;
				}
			case 0x1:
				/* advanced arithmetic */
				switch(EXTRACT(14, 2)) {
				case 0x0:
					/* MUL */
					return (struct decoded_instr){ RL_MUL, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x1:
					/* DIV */
					return (struct decoded_instr){ RL_DIV, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x2:
					/* MOD */
					return (struct decoded_instr){ RL_MOD, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				default:
					return ILLEGAL_INSTR;
				}
			case 0x2:
				/* simple logic */
				if(raw_instr & 0x0001) {
					/* OR */
					return (struct decoded_instr){ RL_OR, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				} else {
					/* AND */
					return (struct decoded_instr){ RL_AND, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				}
			case 0x3:
				/* advanced logic */
				switch(EXTRACT(14, 2)) {
				case 0x0:
					/* NAND */
					return (struct decoded_instr){ RL_NAND, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x1:
					/* NOR */
					return (struct decoded_instr){ RL_NOR, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x2:
					/* XOR */
					return (struct decoded_instr){ RL_XOR, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				case 0x3:
					/* XNOR */
					return (struct decoded_instr){ RL_XNOR, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				}
				return ILLEGAL_INSTR; /* anti compiler warning */
			case 0x4:
				/* SH L/R */
				if(raw_instr & 0x0010) {
					/* SHR */
					return (struct decoded_instr){ RL_SHR, {.reg_dsn={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(12, 4) }} };
				} else {
					/* SHL */
					return (struct decoded_instr){ RL_SHL, {.reg_dsn={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(12, 4) }} };
				}
			case 0x5:
				/* (A)BSH */
				if(raw_instr & 0x0001) {
					/* ABSH */
					return (struct decoded_instr){ RL_ABSH, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				} else {
					/* BSH */
					return (struct decoded_instr){ RL_BSH, {.reg_dab={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				}
			case 0x6:
				/* XCH */
				return (struct decoded_instr){ RL_XCH, {.reg_ab={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
			case 0x7:
				/* TST or CMP */
				if(raw_instr & 0x0001) {
					/* CMP */
					return (struct decoded_instr){ RL_CMP, {.reg_ab={ EXTRACT(8, 3), EXTRACT(11, 3) }} };
				} else {
					/* TST */
					return (struct decoded_instr){ RL_TST, {.reg_r={ EXTRACT(8, 3) }} };
				}
			}
		}
	}

	return ILLEGAL_INSTR; /* anti compiler warning */

#undef EXTRACT
#undef ILLEGAL_INSTR
}

