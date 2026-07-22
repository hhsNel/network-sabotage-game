#include "risc-labs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

#define CAP_MOV       0x0000000000000001ULL
#define CAP_ADD       0x0000000000000002ULL
#define CAP_SUB       0x0000000000000004ULL
#define CAP_MUL       0x0000000000000008ULL
#define CAP_DIV       0x0000000000000010ULL
#define CAP_MOD       0x0000000000000020ULL
#define CAP_AND       0x0000000000000040ULL
#define CAP_OR        0x0000000000000080ULL
#define CAP_NAND      0x0000000000000100ULL
#define CAP_NOR       0x0000000000000200ULL
#define CAP_XOR       0x0000000000000400ULL
#define CAP_XNOR      0x0000000000000800ULL
#define CAP_SHL       0x0000000000001000ULL
#define CAP_SHR       0x0000000000002000ULL
#define CAP_BSH       0x0000000000004000ULL
#define CAP_ABSH      0x0000000000008000ULL
#define CAP_XCH       0x0000000000010000ULL
#define CAP_TST       0x0000000000020000ULL
#define CAP_CMP       0x0000000000040000ULL
#define CAP_IMM       0x0000000000080000ULL
#define CAP_IMMS      0x0000000000100000ULL
#define CAP_LOADBR    0x0000000000200000ULL
#define CAP_LOADBI    0x0000000000400000ULL
#define CAP_LOADSR    0x0000000000800000ULL
#define CAP_LOADSI    0x0000000001000000ULL
#define CAP_LOADWR    0x0000000002000000ULL
#define CAP_LOADWI    0x0000000004000000ULL
#define CAP_STOREBR   0x0000000008000000ULL
#define CAP_STOREBI   0x0000000010000000ULL
#define CAP_STOREWR   0x0000000020000000ULL
#define CAP_STOREWI   0x0000000040000000ULL
#define CAP_MXCH      0x0000000080000000ULL
#define CAP_Jc        0x0000000100000000ULL
#define CAP_JRELc     0x0000000200000000ULL
#define CAP_JEc       0x0000000400000000ULL
#define CAP_JENc      0x0000000800000000ULL
#define CAP_JAc       0x0000001000000000ULL
#define CAP_JANc      0x0000002000000000ULL
#define CAP_IN        0x0000004000000000ULL
#define CAP_OUT       0x0000008000000000ULL
#define CAP_INM       0x0000010000000000ULL
#define CAP_OUTM      0x0000020000000000ULL
#define CAP_R4        0x0000040000000000ULL
#define CAP_R5        0x0000080000000000ULL
#define CAP_ANYLAST   0x0000100000000000ULL
#define CAP_OPPOSITE  0x0000200000000000ULL
#define CAP_CLOCKWISE 0x0000400000000000ULL
#define CAP_C         0x0000800000000000ULL
#define CAP_V         0x0001000000000000ULL
#define CAP_UP_OUT    0x0002000000000000ULL
#define CAP_RIGHT_OUT 0x0004000000000000ULL
#define CAP_DOWN_OUT  0x0008000000000000ULL
#define CAP_LEFT_OUT  0x0010000000000000ULL
#define CAP_UP_IN     0x0020000000000000ULL
#define CAP_RIGHT_IN  0x0040000000000000ULL
#define CAP_DOWN_IN   0x0080000000000000ULL
#define CAP_LEFT_IN   0x0100000000000000ULL
#define CAP_512MEM    0x0200000000000000ULL

#define CAP_BASE \
	(CAP_MOV | CAP_ADD | CAP_SUB | CAP_AND | CAP_OR | CAP_SHL | CAP_SHR | \
	 CAP_XCH | CAP_TST | CAP_IMM | CAP_IMMS | CAP_LOADBR | CAP_LOADBI | \
	 CAP_LOADSR | CAP_LOADSI | CAP_LOADWR | CAP_LOADWI | CAP_STOREBR | \
	 CAP_STOREBI | CAP_STOREWR | CAP_STOREWI | CAP_Jc | CAP_IN | CAP_OUT)
#define CAP_ADVANCED_ARITHMETIC \
	(CAP_MUL | CAP_DIV | CAP_MOD)
#define CAP_ADVANCED_LOGIC \
	(CAP_NAND | CAP_NOR | CAP_XOR | CAP_XNOR)
#define CAP_SHIFTS \
	(CAP_BSH | CAP_ABSH)
#define CAP_JUMPS \
	(CAP_JEc | CAP_JENc | CAP_JAc | CAP_JANc)
#define CAP_IOMEM \
	(CAP_INM | CAP_OUTM)
#define CAP_REGS \
	(CAP_R4 | CAP_R5)
#define CAP_PORTS \
	(CAP_ANYLAST | CAP_OPPOSITE | CAP_CLOCKWISE)
#define CAP_FLAGS \
	(CAP_C | CAP_V)
#define CAP_PORT_OUT \
	(CAP_UP_OUT | CAP_RIGHT_OUT | CAP_DOWN_OUT | CAP_LEFT_OUT)
#define CAP_PORT_IN \
	(CAP_UP_IN | CAP_RIGHT_IN | CAP_DOWN_IN | CAP_LEFT_IN)

#define CAPS_1PT 10
#define CAPS_2PT CAPS_1PT + 4
#define CAPS_3PT CAPS_2PT + 1
static rl_capabilities all_caps[] = {
	CAP_ADVANCED_ARITHMETIC, CAP_ADVANCED_LOGIC, CAP_SHIFTS, CAP_CMP, CAP_MXCH, CAP_JRELc, CAP_JUMPS, CAP_IOMEM, CAP_ANYLAST, CAP_FLAGS, /* 1pt */
	CAP_REGS, CAP_PORTS, CAP_PORT_OUT, CAP_PORT_IN, /* 2pt */
	CAP_512MEM, /* 3pt */
};

struct decoded_instr {
	enum rl_opcode op;
	union {
		struct { enum rl_reg destination; enum rl_reg source; } reg_ds;
		struct { enum rl_reg destination; enum rl_reg a; enum rl_reg b; } reg_dab;
		struct { enum rl_reg destination; enum rl_reg source; uint8_t n : 4; } reg_dsn;
		struct { enum rl_reg reg; } reg_r;
		struct { enum rl_reg a; enum rl_reg b; } reg_ab;
		struct { enum rl_reg destination; uint16_t imm : 9; } im_di;
		struct { enum rl_reg destination; enum rl_reg source; } im_ds;
		struct { enum rl_reg destination; uint16_t addr : 9; } im_da;
		struct { enum rl_reg source; uint16_t addr : 9; } im_sa;
		struct { enum rl_reg a; enum rl_reg b; } im_ab;
		struct { enum rl_condition cond; int8_t relative; } j_cr;
		struct { enum rl_condition cond; enum rl_reg offset; uint8_t base : 5; } j_cob;
		struct { uint8_t mask : 4; int8_t relative : 6; } j_mr;
		struct { enum rl_reg destination; enum rl_port port; } io_dp;
		struct { enum rl_reg source; enum rl_port port; } io_sp;
	} data;
};

enum rl_asm_token_type { RL_ASM_TK_MNEMONIC, RL_ASM_TK_REG, RL_ASM_TK_PORT, RL_ASM_TK_NUMBER, RL_ASM_TK_EQU, RL_ASM_TK_IDENT, RL_ASM_TK_COLON, RL_ASM_TK_EOF, RL_ASM_TK_ERROR };
struct rl_asm_token {
	enum rl_asm_token_type type;
	union {
		enum rl_opcode mnemonic;
		enum rl_reg reg;
		enum rl_port port;
		uint16_t number;
		struct {
			char *begin;
			size_t length;
		} ident;
	} data;
	unsigned int line, column;
};

static struct rl_asm_token next_tk(char **in_str, unsigned int *line, unsigned int *column);
static rl_capabilities generate_caps(unsigned int spare_points);
static struct decoded_instr decode(uint16_t raw_instr);
static void rl_reset(struct node *n);
static void rl_exec(struct node *n);
static void rl_destroy(struct node *n);
static uint16_t get_reg(struct node *n, enum rl_reg r);
static void set_reg(struct node *n, enum rl_reg r, uint16_t value);
static void advance_pc(struct node *n);
static void raise_Z(struct node *n, uint16_t cmp_value);
static void raise_N(struct node *n, uint16_t cmp_value);
static void raise_C(struct node *n, int flag);
static void raise_V(struct node *n, int flag);
static int evaluate_condition(struct node *n, enum rl_condition cond);
static void MOV(struct node *n, struct decoded_instr di);
static void ADD(struct node *n, struct decoded_instr di);
static void SUB(struct node *n, struct decoded_instr di);
static void MUL(struct node *n, struct decoded_instr di);
static void DIV(struct node *n, struct decoded_instr di);
static void MOD(struct node *n, struct decoded_instr di);
static void AND(struct node *n, struct decoded_instr di);
static void OR(struct node *n, struct decoded_instr di);
static void NAND(struct node *n, struct decoded_instr di);
static void NOR(struct node *n, struct decoded_instr di);
static void XOR(struct node *n, struct decoded_instr di);
static void XNOR(struct node *n, struct decoded_instr di);
static void SHL(struct node *n, struct decoded_instr di);
static void SHR(struct node *n, struct decoded_instr di);
static void BSH(struct node *n, struct decoded_instr di);
static void ABSH(struct node *n, struct decoded_instr di);
static void XCH(struct node *n, struct decoded_instr di);
static void TST(struct node *n, struct decoded_instr di);
static void CMP(struct node *n, struct decoded_instr di);
static void IMM(struct node *n, struct decoded_instr di);
static void IMMS(struct node *n, struct decoded_instr di);
static void LOADBR(struct node *n, struct decoded_instr di);
static void LOADBI(struct node *n, struct decoded_instr di);
static void LOADSR(struct node *n, struct decoded_instr di);
static void LOADSI(struct node *n, struct decoded_instr di);
static void LOADWR(struct node *n, struct decoded_instr di);
static void LOADWI(struct node *n, struct decoded_instr di);
static void STOREBR(struct node *n, struct decoded_instr di);
static void STOREBI(struct node *n, struct decoded_instr di);
static void STOREWR(struct node *n, struct decoded_instr di);
static void STOREWI(struct node *n, struct decoded_instr di);
static void MXCH(struct node *n, struct decoded_instr di);
static void Jc(struct node *n, struct decoded_instr di);
static void JRELc(struct node *n, struct decoded_instr di);
static void JEc(struct node *n, struct decoded_instr di);
static void JENc(struct node *n, struct decoded_instr di);
static void JAc(struct node *n, struct decoded_instr di);
static void JANc(struct node *n, struct decoded_instr di);
static void IN(struct node *n, struct decoded_instr di);
static void OUT(struct node *n, struct decoded_instr di);
static void INM(struct node *n, struct decoded_instr di);
static void OUTM(struct node *n, struct decoded_instr di);

static void (*instruction_lut[])(struct node *n, struct decoded_instr di) = {
#define INSTR(NAME) [CONCAT2(RL_,NAME)] = NAME
	INSTR(MOV),
	INSTR(ADD),
	INSTR(SUB),
	INSTR(MUL),
	INSTR(DIV),
	INSTR(MOD),
	INSTR(AND),
	INSTR(OR),
	INSTR(NAND),
	INSTR(NOR),
	INSTR(XOR),
	INSTR(XNOR),
	INSTR(SHL),
	INSTR(SHR),
	INSTR(BSH),
	INSTR(ABSH),
	INSTR(XCH),
	INSTR(TST),
	INSTR(CMP),
	INSTR(IMM),
	INSTR(IMMS),
	INSTR(LOADBR),
	INSTR(LOADBI),
	INSTR(LOADSR),
	INSTR(LOADSI),
	INSTR(LOADWR),
	INSTR(LOADWI),
	INSTR(STOREBR),
	INSTR(STOREBI),
	INSTR(STOREWR),
	INSTR(STOREWI),
	INSTR(MXCH),
	INSTR(Jc),
	INSTR(JRELc),
	INSTR(JEc),
	INSTR(JENc),
	INSTR(JAc),
	INSTR(JANc),
	INSTR(IN),
	INSTR(OUT),
	INSTR(INM),
	INSTR(OUTM),
#undef INSTR
};

struct node
create_rl_computer(unsigned int points)
{
	struct node n;
	struct computer_data *cd;
	struct rl_data *rld;

	n = create_computer();
	cd = n.data;

	cd->data = malloc(sizeof(struct rl_data));
	if(! cd->data) {
		fprintf(stderr, "couldn't malloc rl_data\n");
		exit(1);
	}
	rld = cd->data;

	rld->caps = generate_caps(points);

	if(rld->caps & CAP_512MEM) {
		cd->mem_sz = 512;
	} else {
		cd->mem_sz = 256;
	}
	cd->memory = malloc(cd->mem_sz);
	if(! cd->memory) {
		fprintf(stderr, "couldn't malloc rl memory (%zu)\n", cd->mem_sz);
		exit(1);
	}

	rl_reset(&n);

	cd->reset = rl_reset;
	cd->exec = rl_exec;
	cd->destroy = rl_destroy;

	return n;
}

struct assembly_result
rl_asm(uint8_t *out_buf, size_t out_size, char *in_str)
{
	struct rl_asm_token *tokens;
	unsigned int num_tokens, cap_tokens;
	struct assembly_result res;
	struct symbol { char *begin; size_t length; uint16_t value; };
	struct symbol *symbol_table;
	unsigned int num_symbols, cap_symbols;
	enum operand_type { OP_REGISTER, OP_PORT, OP_NUMBER, OP_RELATIVE, OP_NONE };
	struct operand { enum operand_type type; uint8_t start_index; uint8_t bit_width; };
	struct instr_shape { enum rl_opcode mnemonic; uint16_t template; struct operand operands[3]; };
	static struct instr_shape instruction_shapes[] = {
#define REGISTER(IDX) { OP_REGISTER, (IDX), 3 }
#define PORT(IDX) { OP_PORT, (IDX), 3 }
#define NUMBER(IDX,WIDTH) { OP_NUMBER, (IDX), (WIDTH) }
#define RELATIVE(IDX,WIDTH) { OP_RELATIVE, (IDX), (WIDTH) }
#define NONE { OP_NONE, 0, 0 }
		{ RL_MOV,     0x0000, { REGISTER(5),  REGISTER(8),    NONE } },
		{ RL_ADD,     0x0001, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_SUB,     0x0002, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_MUL,     0x0800, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_DIV,     0x0801, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_MOD,     0x0802, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_AND,     0x1000, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_OR,      0x1001, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_NAND,    0x1800, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_NOR,     0x1801, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_XOR,     0x1802, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_XNOR,    0x1803, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_SHL,     0x2000, { REGISTER(5),  REGISTER(8),    NUMBER(12,4) } },
		{ RL_SHR,     0x2010, { REGISTER(5),  REGISTER(8),    NUMBER(12,4) } },
		{ RL_BSH,     0x2800, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_ABSH,    0x2801, { REGISTER(5),  REGISTER(8),    REGISTER(11) } },
		{ RL_XCH,     0x3000, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_TST,     0x3800, { REGISTER(8),  NONE,           NONE } },
		{ RL_CMP,     0x3801, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_IMM,     0x8000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_IMMS,    0x9000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_LOADBR,  0xF000, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_LOADBI,  0xA000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_LOADSR,  0xF200, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_LOADSI,  0xB000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_LOADWR,  0xF400, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_LOADWI,  0xC000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_STOREBR, 0xF600, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_STOREBI, 0xD000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_STOREWR, 0xF800, { REGISTER(8),  REGISTER(11),   NONE } },
		{ RL_STOREWI, 0xE000, { REGISTER(4),  NUMBER(7,9),    NONE } },
		{ RL_MXCH,    0xFA00, { REGISTER(8),  REGISTER(11),   NONE } },
		/* TODO conditions */
		{ RL_JEc,     0x5000, { NUMBER(5,4),  RELATIVE(10,6), NONE } },
		{ RL_JENc,    0x5040, { NUMBER(5,4),  RELATIVE(10,6), NONE } },
		{ RL_JAc,     0x5800, { NUMBER(5,4),  RELATIVE(10,6), NONE } },
		{ RL_JANc,    0x5840, { NUMBER(5,4),  RELATIVE(10,6), NONE } },
		{ RL_IN,      0x6000, { REGISTER(5),  PORT(8),        NONE } },
		{ RL_OUT,     0x6800, { REGISTER(5),  PORT(8),        NONE } },
		{ RL_INM,     0x7000, { REGISTER(5),  PORT(8),        NONE } },
		{ RL_OUTM,    0x7800, { REGISTER(5),  PORT(8),        NONE } },
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
			for(j = 0; j < num_symbols; ++j) {
				if( symbol_table[j].length == symbol_table[num_symbols].length &&
					strncmp(symbol_table[j].begin, symbol_table[num_symbols].begin, symbol_table[num_symbols].length) == 0 ) {
					res.success = 0;
					res.line = tokens[i].line;
					res.column = tokens[i].column;
					snprintf(res.reason, sizeof(res.reason), "symbol %.*s redefined", (int)symbol_table[j].length, symbol_table[j].begin);
					return res;
				}
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
					res.line = 0; \
					res.column = 0; \
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
				(instr_part & ~opcode_bitmask) != ~opcode_bitmask ) { \
				res.success = 0; \
				res.line = 0; \
				res.column = 0; \
				snprintf(res.reason, sizeof(res.reason), "operand too big (bitwise)"); \
				return res; \
			} \
			assembled_instr |= instr_part << (16 - instr_records[i].shape.operands[IDX].start_index - instr_records[i].shape.operands[IDX].bit_width); \
		}
		HANDLE_OPERAND(0);
		HANDLE_OPERAND(1);
		HANDLE_OPERAND(2);
#undef HANDLE_OPERAND
		out_buf[0] = assembled_instr >> 8;
		out_buf[1] = assembled_instr;
		out_buf += 2;
		out_size -= 2;
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
		/* TODO JMP */
		{ "jc",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_Jc},0,0} },
		{ "JC",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_Jc},0,0} },
		/* TODO JREL */
		{ "jrelc",     {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JRELc},0,0} },
		{ "JRELC",     {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JRELc},0,0} },
		{ "jec",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JEc},0,0} },
		{ "JEC",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JEc},0,0} },
		{ "jenc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JENc},0,0} },
		{ "JENC",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JENc},0,0} },
		{ "jac",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JAc},0,0} },
		{ "JAC",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JAc},0,0} },
		{ "janc",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JANc},0,0} },
		{ "JANC",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_JANc},0,0} },
		{ "in",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IN},0,0} },
		{ "IN",        {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_IN},0,0} },
		{ "out",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUT},0,0} },
		{ "OUT",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUT},0,0} },
		{ "inm",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_INM},0,0} },
		{ "INM",       {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_INM},0,0} },
		{ "outm",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUTM},0,0} },
		{ "OUTM",      {RL_ASM_TK_MNEMONIC,{.mnemonic=RL_OUTM},0,0} },
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

static rl_capabilities
generate_caps(unsigned int spare_points)
{
	rl_capabilities c;
	unsigned int idx;

	c = 0;
	while(spare_points > 0) {
		idx = rand() / ((RAND_MAX+1u) / (sizeof(all_caps)/sizeof(*all_caps)));
		if(idx >= CAPS_2PT && spare_points < 3) continue;
		if(idx >= CAPS_1PT && spare_points < 2) continue;
		if((c & all_caps[idx]) == all_caps[idx]) continue;

		c |= all_caps[idx];
		if(idx >= CAPS_2PT) spare_points -= 3;
		else if(idx >= CAPS_1PT) spare_points -= 2;
		else --spare_points;
	}

	return c;
}

static struct decoded_instr
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

static void
rl_reset(struct node *n)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	memset(cd->memory, 0, cd->mem_sz);
	cd->pc = 0;
	cd->local_clock = 0;

	memset(&rld->regs, 0, sizeof(rld->regs));
	rld->last_port = 0;
}

static void
rl_exec(struct node *n)
{
	uint16_t raw_instr;
	struct decoded_instr di;
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	raw_instr = computer_load_word(cd, cd->pc);
	di = decode(raw_instr);

	if(rld->caps & CAP_PORT_OUT) {
#define HANDLE_PORT(PORT, BIT) \
	rld->regs[RL_REG_STATUS] &= ~(BIT); \
	if(port_write_available(PORT)) rld->regs[RL_REG_STATUS] |= (BIT);
		HANDLE_PORT(n->write_up, RL_STATUS_UP_OUT);
		HANDLE_PORT(n->write_right, RL_STATUS_RIGHT_OUT);
		HANDLE_PORT(n->write_down, RL_STATUS_DOWN_OUT);
		HANDLE_PORT(n->write_left, RL_STATUS_LEFT_OUT);
#undef HANDLE_PORT
	}
	if(rld->caps & CAP_PORT_IN) {
#define HANDLE_PORT(PORT, BIT) \
	rld->regs[RL_REG_STATUS] &= ~(BIT); \
	if(port_read_available(PORT)) rld->regs[RL_REG_STATUS] |= (BIT);
		HANDLE_PORT(n->read_up, RL_STATUS_UP_IN);
		HANDLE_PORT(n->read_right, RL_STATUS_RIGHT_IN);
		HANDLE_PORT(n->read_down, RL_STATUS_DOWN_IN);
		HANDLE_PORT(n->read_left, RL_STATUS_LEFT_IN);
#undef HANDLE_PORT
	}

	if(di.op != RL_ILLEGAL_INSTR && rld->caps & (1 << di.op)) {
		if((unsigned int)di.op < sizeof(instruction_lut)/sizeof(*instruction_lut) && instruction_lut[di.op]) {
			instruction_lut[di.op](n, di);
		} else {
			/* probably an unimplemented instr, or something went very wrong */
			fprintf(stderr, "unimplemented %d\n", (int)di.op);
		}
	} else {
		/* illegal instr */
		cd->pc += 2;
	}
}

static void
rl_destroy(struct node *n)
{
	struct computer_data *cd;

	cd = n->data;

	free(cd->memory);
	free(cd->data);
}

static uint16_t
get_reg(struct node *n, enum rl_reg r)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	switch(r) {
	case RL_REG_ZERO:
		return 0;
	case RL_REG_R4:
		if(rld->caps & CAP_R4) return rld->regs[RL_REG_R4];
		return 0;
	case RL_REG_R5:
		if(rld->caps & CAP_R5) return rld->regs[RL_REG_R5];
		return 0;
	default:
		return rld->regs[r];
	}
}

static void
set_reg(struct node *n, enum rl_reg r, uint16_t value)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	switch(r) {
	case RL_REG_ZERO:
	case RL_REG_STATUS:
		return;
	case RL_REG_R4:
		if(! (rld->caps & CAP_R4)) return;
		break;
	case RL_REG_R5:
		if(! (rld->caps & CAP_R5)) return;
		break;
	default:
		break; /* anti compiler warning */
	}

	rld->regs[r] = value;
}

static void
advance_pc(struct node *n)
{
	struct computer_data *cd;

	cd = n->data;

	cd->pc += 2;
	cd->pc %= cd->mem_sz;
}

static void
raise_Z(struct node *n, uint16_t cmp_value)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	rld->regs[RL_REG_STATUS] &= ~RL_STATUS_Z;
	if(cmp_value == 0) {
		rld->regs[RL_REG_STATUS] |= RL_STATUS_Z;
	}
}

static void
raise_N(struct node *n, uint16_t cmp_value)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	rld->regs[RL_REG_STATUS] &= ~RL_STATUS_N;
	if(cmp_value & 0x8000) {
		rld->regs[RL_REG_STATUS] |= RL_STATUS_N;
	}
}

static void
raise_C(struct node *n, int flag)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	if(! (rld->caps & CAP_C)) return;

	rld->regs[RL_REG_STATUS] &= ~RL_STATUS_C;
	if(flag) {
		rld->regs[RL_REG_STATUS] |= RL_STATUS_C;
	}
}

static void
raise_V(struct node *n, int flag)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	if(! (rld->caps & CAP_V)) return;

	rld->regs[RL_REG_STATUS] &= ~RL_STATUS_V;
	if(flag) {
		rld->regs[RL_REG_STATUS] |= RL_STATUS_V;
	}
}

static int
evaluate_condition(struct node *n, enum rl_condition cond)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	switch(cond) {
	case RL_COND_ALWAYS:
		return 1;
	case RL_COND_ZERO:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_Z) != 0;
	case RL_COND_NEG:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_N) != 0;
	case RL_COND_CARRY:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_C) != 0;
	case RL_COND_NO_CARRY:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_C) == 0;
	case RL_COND_OVERFLOW:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_V) != 0;
	case RL_COND_NONZERO:
		return (rld->regs[RL_REG_STATUS] & RL_STATUS_Z) == 0;
	case RL_COND_POS:
		return (rld->regs[RL_REG_STATUS] & (RL_STATUS_Z|RL_STATUS_N)) == 0;
	default:
		fprintf(stderr, "somehow a condition got corrupted?\n");
		exit(1);
	}
}

static void
MOV(struct node *n, struct decoded_instr di)
{
	set_reg(n, di.data.reg_ds.destination, get_reg(n, di.data.reg_ds.source));
	advance_pc(n);
}

static void
ADD(struct node *n, struct decoded_instr di)
{
	uint32_t a, b, res;

	a = get_reg(n, di.data.reg_dab.a);
	b = get_reg(n, di.data.reg_dab.b);
	res = a + b;

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);
	raise_C(n, (res & 0xffff0000) != 0);
	raise_V(n, ((~(a ^ b) & (a ^ res)) & 0x8000) != 0);

	advance_pc(n);
}

static void
SUB(struct node *n, struct decoded_instr di)
{
	uint32_t a, b, res;

	a = get_reg(n, di.data.reg_dab.a);
	b = get_reg(n, di.data.reg_dab.b);
	res = a - b;

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);
	raise_C(n, (res & 0xffff0000) != 0);
	raise_V(n, (((a ^ b) & (a ^ res)) & 0x8000) != 0);

	advance_pc(n);
}

static void
MUL(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = get_reg(n, di.data.reg_dab.a) * get_reg(n, di.data.reg_dab.b);

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
DIV(struct node *n, struct decoded_instr di)
{
	uint16_t res, b;

	b = get_reg(n, di.data.reg_dab.b);
	if(b != 0) {
		res = (int16_t)get_reg(n, di.data.reg_dab.a) / (int16_t)b;
	} else {
		res = 0;
	}

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
MOD(struct node *n, struct decoded_instr di)
{
	uint16_t res, b;

	b = get_reg(n, di.data.reg_dab.b);
	if(b != 0) {
		res = (int16_t)get_reg(n, di.data.reg_dab.a) % (int16_t)b;
	} else {
		res = 0;
	}

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);

	advance_pc(n);
}

static void
AND(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = get_reg(n, di.data.reg_dab.a) & get_reg(n, di.data.reg_dab.b);

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
OR(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = get_reg(n, di.data.reg_dab.a) | get_reg(n, di.data.reg_dab.b);

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
NAND(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = ~(get_reg(n, di.data.reg_dab.a) & get_reg(n, di.data.reg_dab.b));

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
NOR(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = ~(get_reg(n, di.data.reg_dab.a) | get_reg(n, di.data.reg_dab.b));

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
XOR(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = get_reg(n, di.data.reg_dab.a) ^ get_reg(n, di.data.reg_dab.b);

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
XNOR(struct node *n, struct decoded_instr di)
{
	uint16_t res;

	res = ~(get_reg(n, di.data.reg_dab.a) ^ get_reg(n, di.data.reg_dab.b));

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
SHL(struct node *n, struct decoded_instr di)
{
	uint16_t res, src;

	src = get_reg(n, di.data.reg_dsn.source);
	res = src << di.data.reg_dsn.n;

	set_reg(n, di.data.reg_dsn.destination, res);

	raise_Z(n, res);
	raise_N(n, res);
	if(di.data.reg_dsn.n > 0) {
		raise_C(n, (src >> (16-di.data.reg_dsn.n)) & 1);
	} else {
		raise_C(n, 0);
	}

	advance_pc(n);
}

static void
SHR(struct node *n, struct decoded_instr di)
{
	uint16_t res, src;

	src = get_reg(n, di.data.reg_dsn.source);
	res = src >> di.data.reg_dsn.n;

	set_reg(n, di.data.reg_dsn.destination, res);

	raise_Z(n, res);
	raise_N(n, res);
	if(di.data.reg_dsn.n > 0) {
		raise_C(n, (src >> (di.data.reg_dsn.n-1)) & 1);
	} else {
		raise_C(n, 0);
	}

	advance_pc(n);
}

static void
BSH(struct node *n, struct decoded_instr di)
{
	uint16_t res, a, b;

	a = get_reg(n, di.data.reg_dab.a);
	b = get_reg(n, di.data.reg_dab.b);
	if((b & 0x8000) == 0) {
		res = a >> (b & 0x000F);
		if(di.data.reg_dsn.n > 0) {
			raise_C(n, (a >> (b-1)) & 1);
		} else {
			raise_C(n, 0);
		}
	} else {
		res = a << ((-b) & 0x000F);
		if(b > 0) {
			raise_C(n, (a >> (16-b)) & 1);
		} else {
			raise_C(n, 0);
		}
	}

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
ABSH(struct node *n, struct decoded_instr di)
{
	uint16_t res, a, b;

	a = get_reg(n, di.data.reg_dab.a);
	b = get_reg(n, di.data.reg_dab.b);
	if((b & 0x8000) == 0) {
		res = a >> (b & 0x000F);
		if(a & 0x8000) {
			res |= (0xFFFF << (16 - (b & 0x000F))) & 0xFFFF;
		}
		if(di.data.reg_dsn.n > 0) {
			raise_C(n, (a >> (b-1)) & 1);
		} else {
			raise_C(n, 0);
		}
	} else {
		res = a << ((-b) & 0x000F);
		if(b > 0) {
			raise_C(n, (a >> (16-b)) & 1);
		} else {
			raise_C(n, 0);
		}
	}

	set_reg(n, di.data.reg_dab.destination, res);

	raise_Z(n, res);
	raise_N(n, res);

	advance_pc(n);
}

static void
XCH(struct node *n, struct decoded_instr di)
{
	uint16_t a, b;

	a = get_reg(n, di.data.reg_ab.a);
	b = get_reg(n, di.data.reg_ab.b);

	set_reg(n, di.data.reg_ab.a, b);
	set_reg(n, di.data.reg_ab.b, a);

	advance_pc(n);
}

static void
TST(struct node *n, struct decoded_instr di)
{
	uint16_t r;

	r = get_reg(n, di.data.reg_r.reg);

	raise_Z(n, r);
	raise_N(n, r);

	advance_pc(n);
}

static void
CMP(struct node *n, struct decoded_instr di)
{
	uint16_t sub;

	sub = get_reg(n, di.data.reg_ab.a) - get_reg(n, di.data.reg_ab.b);

	raise_Z(n, sub);
	raise_N(n, sub);

	advance_pc(n);
}

static void
IMM(struct node *n, struct decoded_instr di)
{
	set_reg(n, di.data.im_di.destination, di.data.im_di.imm);

	advance_pc(n);
}

static void
IMMS(struct node *n, struct decoded_instr di)
{
	uint16_t imm; /* ironically */

	imm = di.data.im_di.imm;
	if(imm & 0x0100) {
		imm |= 0xFE00;
	}
	set_reg(n, di.data.im_di.destination, imm);

	advance_pc(n);
}

static void
LOADBR(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_byte(n->data, get_reg(n, di.data.im_ds.source));
	set_reg(n, di.data.im_ds.destination, val);

	advance_pc(n);
}

static void
LOADBI(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_byte(n->data, di.data.im_da.addr);
	set_reg(n, di.data.im_da.destination, val);

	advance_pc(n);
}

static void
LOADSR(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_byte(n->data, get_reg(n, di.data.im_ds.source));
	if(val & 0x0080) {
		val |= 0xFF00;
	}
	set_reg(n, di.data.im_ds.destination, val);

	advance_pc(n);
}

static void
LOADSI(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_byte(n->data, di.data.im_da.addr);
	if(val & 0x0080) {
		val |= 0xFF00;
	}
	set_reg(n, di.data.im_da.destination, val);

	advance_pc(n);
}

static void
LOADWR(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_word(n->data, get_reg(n, di.data.im_ds.source));
	set_reg(n, di.data.im_ds.destination, val);

	advance_pc(n);
}

static void
LOADWI(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = computer_load_word(n->data, di.data.im_da.addr);
	set_reg(n, di.data.im_da.destination, val);

	advance_pc(n);
}

static void
STOREBR(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = get_reg(n, di.data.im_ds.source);
	computer_store_byte(n->data, get_reg(n, di.data.im_ds.destination), val);

	advance_pc(n);
}

static void
STOREBI(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = get_reg(n, di.data.im_sa.source);
	computer_store_byte(n->data, di.data.im_sa.addr, val);

	advance_pc(n);
}

static void
STOREWR(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = get_reg(n, di.data.im_ds.source);
	computer_store_word(n->data, get_reg(n, di.data.im_ds.destination), val);

	advance_pc(n);
}

static void
STOREWI(struct node *n, struct decoded_instr di)
{
	uint16_t val;

	val = get_reg(n, di.data.im_sa.source);
	computer_store_word(n->data, di.data.im_sa.addr, val);

	advance_pc(n);
}

static void
MXCH(struct node *n, struct decoded_instr di)
{
	uint8_t a, b;

	a = computer_load_byte(n->data, get_reg(n, di.data.im_ab.a));
	b = computer_load_byte(n->data, get_reg(n, di.data.im_ab.b));
	
	computer_store_byte(n->data, get_reg(n, di.data.im_ab.b), a);
	computer_store_byte(n->data, get_reg(n, di.data.im_ab.a), b);

	advance_pc(n);
}

static void
Jc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;

	cd = n->data;

	advance_pc(n);

	if(evaluate_condition(n, di.data.j_cr.cond)) {
		cd->pc += di.data.j_cr.relative;
		cd->pc %= cd->mem_sz;
	}
}

static void
JRELc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;

	cd = n->data;

	if(evaluate_condition(n, di.data.j_cob.cond)) {
		cd->pc = di.data.j_cob.base + get_reg(n, di.data.j_cob.offset);
		cd->pc %= cd->mem_sz;
	} else {
		advance_pc(n);
	}
}

static void
JEc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	advance_pc(n);
	if( (rld->regs[RL_REG_STATUS] & di.data.j_mr.mask) != 0 ) {
		cd->pc += di.data.j_mr.relative;
		cd->pc %= cd->mem_sz;
	}
}

static void
JENc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	advance_pc(n);
	if( (rld->regs[RL_REG_STATUS] & di.data.j_mr.mask) == 0 ) {
		cd->pc += di.data.j_mr.relative;
		cd->pc %= cd->mem_sz;
	}
}

static void
JAc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	advance_pc(n);
	if( (rld->regs[RL_REG_STATUS] & di.data.j_mr.mask) == di.data.j_mr.mask ) {
		cd->pc += di.data.j_mr.relative;
		cd->pc %= cd->mem_sz;
	}
}

static void
JANc(struct node *n, struct decoded_instr di)
{
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	advance_pc(n);
	if( (rld->regs[RL_REG_STATUS] & di.data.j_mr.mask) != di.data.j_mr.mask ) {
		cd->pc += di.data.j_mr.relative;
		cd->pc %= cd->mem_sz;
	}
}

static void
IN(struct node *n, struct decoded_instr di)
{
	struct port *p;
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	p = NULL;
	switch(di.data.io_dp.port) {
	case RL_PORT_UP: p = n->read_up; break;
	case RL_PORT_RIGHT: p = n->read_right; break;
	case RL_PORT_DOWN: p = n->read_down; break;
	case RL_PORT_LEFT: p = n->read_left; break;
	case RL_PORT_ANY:
		if(port_read_available(n->read_up))    { rld->last_port = 0; p = n->read_up; break; }
		if(port_read_available(n->read_right)) { rld->last_port = 1; p = n->read_right; break; }
		if(port_read_available(n->read_down))  { rld->last_port = 2; p = n->read_down; break; }
		if(port_read_available(n->read_left))  { rld->last_port = 3; p = n->read_left; break; }
		break;
	case RL_PORT_LAST:
		switch(rld->last_port) {
		case 0: p = n->read_up; break;
		case 1: p = n->read_right; break;
		case 2: p = n->read_down; break;
		case 3: p = n->read_left; break;
		}
		break;
	case RL_PORT_OPPOSITE:
		switch(rld->last_port) {
		case 0: p = n->read_down; break;
		case 1: p = n->read_left; break;
		case 2: p = n->read_up; break;
		case 3: p = n->read_right; break;
		}
		break;
	case RL_PORT_CLOCKWISE:
		switch(rld->last_port) {
		case 0: p = n->read_right; break;
		case 1: p = n->read_down; break;
		case 2: p = n->read_left; break;
		case 3: p = n->read_up; break;
		}
		break;
	}

	if(port_read_available(p)) {
		set_reg(n, di.data.io_dp.destination, port_read(p));
		advance_pc(n);
	} else {
		/* don't advance the PC -> same instruction executed next time */
	}
}

static void
OUT(struct node *n, struct decoded_instr di)
{
	struct port *p;
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	p = NULL;
	switch(di.data.io_sp.port) {
	case RL_PORT_UP: p = n->write_up; break;
	case RL_PORT_RIGHT: p = n->write_right; break;
	case RL_PORT_DOWN: p = n->write_down; break;
	case RL_PORT_LEFT: p = n->write_left; break;
	case RL_PORT_ANY:
	case RL_PORT_LAST:
		switch(rld->last_port) {
		case 0: p = n->write_up; break;
		case 1: p = n->write_right; break;
		case 2: p = n->write_down; break;
		case 3: p = n->write_left; break;
		}
		break;
	case RL_PORT_OPPOSITE:
		switch(rld->last_port) {
		case 0: p = n->write_down; break;
		case 1: p = n->write_left; break;
		case 2: p = n->write_up; break;
		case 3: p = n->write_right; break;
		}
		break;
	case RL_PORT_CLOCKWISE:
		switch(rld->last_port) {
		case 0: p = n->write_right; break;
		case 1: p = n->write_down; break;
		case 2: p = n->write_left; break;
		case 3: p = n->write_up; break;
		}
		break;
	}

	if(port_write_available(p)) {
		port_write(p, get_reg(n, di.data.io_sp.source));
		advance_pc(n);
	} else {
		/* don't advance the PC -> same instruction executed next time */
	}
}

static void
INM(struct node *n, struct decoded_instr di)
{
	struct port *p;
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	p = NULL;
	switch(di.data.io_dp.port) {
	case RL_PORT_UP: p = n->read_up; break;
	case RL_PORT_RIGHT: p = n->read_right; break;
	case RL_PORT_DOWN: p = n->read_down; break;
	case RL_PORT_LEFT: p = n->read_left; break;
	case RL_PORT_ANY:
		if(port_read_available(n->read_up))    { rld->last_port = 0; p = n->read_up; break; }
		if(port_read_available(n->read_right)) { rld->last_port = 1; p = n->read_right; break; }
		if(port_read_available(n->read_down))  { rld->last_port = 2; p = n->read_down; break; }
		if(port_read_available(n->read_left))  { rld->last_port = 3; p = n->read_left; break; }
		break;
	case RL_PORT_LAST:
		switch(rld->last_port) {
		case 0: p = n->read_up; break;
		case 1: p = n->read_right; break;
		case 2: p = n->read_down; break;
		case 3: p = n->read_left; break;
		}
		break;
	case RL_PORT_OPPOSITE:
		switch(rld->last_port) {
		case 0: p = n->read_down; break;
		case 1: p = n->read_left; break;
		case 2: p = n->read_up; break;
		case 3: p = n->read_right; break;
		}
		break;
	case RL_PORT_CLOCKWISE:
		switch(rld->last_port) {
		case 0: p = n->read_right; break;
		case 1: p = n->read_down; break;
		case 2: p = n->read_left; break;
		case 3: p = n->read_up; break;
		}
		break;
	}

	if(port_read_available(p)) {
		computer_store_word(cd, get_reg(n, di.data.io_dp.destination), port_read(p));
		advance_pc(n);
	} else {
		/* don't advance the PC -> same instruction executed next time */
	}
}

static void
OUTM(struct node *n, struct decoded_instr di)
{
	struct port *p;
	struct computer_data *cd;
	struct rl_data *rld;

	cd = n->data;
	rld = cd->data;

	switch(di.data.io_sp.port) {
	case RL_PORT_UP: p = n->write_up; break;
	case RL_PORT_RIGHT: p = n->write_right; break;
	case RL_PORT_DOWN: p = n->write_down; break;
	case RL_PORT_LEFT: p = n->write_left; break;
	case RL_PORT_ANY:
	case RL_PORT_LAST:
		switch(rld->last_port) {
		case 0: p = n->write_up; break;
		case 1: p = n->write_right; break;
		case 2: p = n->write_down; break;
		case 3: p = n->write_left; break;
		}
		break;
	case RL_PORT_OPPOSITE:
		switch(rld->last_port) {
		case 0: p = n->write_down; break;
		case 1: p = n->write_left; break;
		case 2: p = n->write_up; break;
		case 3: p = n->write_right; break;
		}
		break;
	case RL_PORT_CLOCKWISE:
		switch(rld->last_port) {
		case 0: p = n->write_right; break;
		case 1: p = n->write_down; break;
		case 2: p = n->write_left; break;
		case 3: p = n->write_up; break;
		}
		break;
	}

	if(port_write_available(p)) {
		port_write(p, computer_load_word(cd, get_reg(n, di.data.io_sp.source)));
		advance_pc(n);
	} else {
		/* don't advance the PC -> same instruction executed next time */
	}
}

