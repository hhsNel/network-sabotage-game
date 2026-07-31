#include "logic/risc-labs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core/util.h"
#include "logic/computer.h"
#include "logic/port.h"

#define CAPS_1PT 10
#define CAPS_2PT CAPS_1PT + 4
#define CAPS_3PT CAPS_2PT + 1
static rl_capabilities all_caps[] = {
	CAP_ADVANCED_ARITHMETIC, CAP_ADVANCED_LOGIC, CAP_SHIFTS, CAP_CMP, CAP_MXCH, CAP_JRELc, CAP_JUMPS, CAP_IOMEM, CAP_ANYLAST, CAP_FLAGS, /* 1pt */
	CAP_REGS, CAP_PORTS, CAP_PORT_OUT, CAP_PORT_IN, /* 2pt */
	CAP_512MEM, /* 3pt */
};

static rl_capabilities generate_caps(unsigned int spare_points);
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

	return n;
}

static rl_capabilities
generate_caps(unsigned int spare_points)
{
	rl_capabilities c;
	unsigned int idx;
	unsigned int fails;

	c = CAP_BASE;
	fails = 0;
	while(spare_points > 0 && fails < 16) {
		idx = rand() / ((RAND_MAX+1u) / (sizeof(all_caps)/sizeof(*all_caps)));
		if(idx >= CAPS_2PT && spare_points < 3) { ++fails; continue; }
		if(idx >= CAPS_1PT && spare_points < 2) { ++fails; continue; }
		if((c & all_caps[idx]) == all_caps[idx]) { ++fails; continue; }

		c |= all_caps[idx];
		if(idx >= CAPS_2PT) spare_points -= 3;
		else if(idx >= CAPS_1PT) spare_points -= 2;
		else --spare_points;
	}

	return c;
}

void
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

void
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
	if(! port_write_available(PORT)) rld->regs[RL_REG_STATUS] |= (BIT);
		HANDLE_PORT(n->write_up, RL_STATUS_UP_OUT);
		HANDLE_PORT(n->write_right, RL_STATUS_RIGHT_OUT);
		HANDLE_PORT(n->write_down, RL_STATUS_DOWN_OUT);
		HANDLE_PORT(n->write_left, RL_STATUS_LEFT_OUT);
#undef HANDLE_PORT
	}
	if(rld->caps & CAP_PORT_IN) {
#define HANDLE_PORT(PORT, BIT) \
	rld->regs[RL_REG_STATUS] &= ~(BIT); \
	if(! port_read_available(PORT)) rld->regs[RL_REG_STATUS] |= (BIT);
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

struct assembly_result
rl_flash(struct node *n, char *string)
{
	struct computer_data *cd;

	cd = n->data;

	rl_reset(n);
	return rl_asm(cd->memory, cd->mem_sz, string);
}

void
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
	raise_N(n, res);

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
		b &= 0x000F;
		res = a >> b;
		if(b > 0) {
			raise_C(n, (a >> (b-1)) & 1);
		} else {
			raise_C(n, 0);
		}
	} else {
		b = (-b) & 0x000F;
		res = a << b;
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
		b &= 0x000F;
		res = a >> b;
		if(a & 0x8000) {
			res |= (0xFFFF << (16 - b)) & 0xFFFF;
		}
		if(b > 0) {
			raise_C(n, (a >> (b-1)) & 1);
		} else {
			raise_C(n, 0);
		}
	} else {
		b = -b;
		res = a << (b & 0x000F);
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

