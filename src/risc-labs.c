#include "risc-labs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

typedef uint64_t capabilities;

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
static capabilities all_caps[] = {
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

struct rl_data {
	capabilities caps;
	uint16_t regs[7]; /* plus the ZERO register */
	uint8_t last_port : 3;
};

static capabilities generate_caps(unsigned int spare_points);
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
static void MOV(struct node *n, struct decoded_instr di);
static void ADD(struct node *n, struct decoded_instr di);
static void SUB(struct node *n, struct decoded_instr di);

static void (*instruction_lut[])(struct node *n, struct decoded_instr di) = {
#define INSTR(NAME) [CONCAT2(RL_,NAME)] = NAME
	INSTR(MOV),
	INSTR(ADD),
	INSTR(SUB),
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

static capabilities
generate_caps(unsigned int spare_points)
{
	capabilities c;
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
						/* JEc */
						return (struct decoded_instr){ RL_JEc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					} else {
						/* JENc */
						return (struct decoded_instr){ RL_JENc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					}
				case 0x3:
					/* JA(N)c */
					if(raw_instr & 0x0040) {
						/* JAc */
						return (struct decoded_instr){ RL_JAc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
					} else {
						/* JANc */
						return (struct decoded_instr){ RL_JANc, {.j_mr={ EXTRACT(5, 4), EXTRACT(10, 6) }} };
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
					return (struct decoded_instr){ RL_ABSH, {.reg_dsn={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
				} else {
					/* SHL */
					return (struct decoded_instr){ RL_BSH, {.reg_dsn={ EXTRACT(5, 3), EXTRACT(8, 3), EXTRACT(11, 3) }} };
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
	rld->last_port = RL_PORT_UP;
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

	if(di.op != RL_ILLEGAL_INSTR && rld->caps & (1 << di.op)) {
		if(instruction_lut[di.op]) {
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

	rld->regs[RL_REG_STATUS] &= ~RL_STATUS_V;
	if(flag) {
		rld->regs[RL_REG_STATUS] |= RL_STATUS_V;
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
	raise_V(n, ((~(a ^ b) & (a ^ res)) & 0x8000) != 0);

	advance_pc(n);
}

