#include "presentation/risc-labs.h"

#include <stdio.h>
#include <inttypes.h>

#include "presentation/computer.h"

static void rl_disassemble_instr(char *outbuf, size_t out_sz, uint16_t instr);

static void
rl_disassemble_instr(char *outbuf, size_t out_sz, uint16_t instr)
{
	struct decoded_instr di;
	uint16_t temp;

	di = decode(instr);

	switch(di.op) {
#define DISASM(...) \
		snprintf(outbuf, out_sz, __VA_ARGS__); \
		return;
#define REGISTER(NR) \
		(((char *[]){"r0","r1","r2","r3","r4","r5","status","zero"})[NR])
#define COND(NR) \
		(((char *[]){"always","zro","neg","carry","no_carry","overflow","nonzero","pos"})[NR])
#define PORT(NR) \
		(((char *[]){"up","right","down","left","any","last","opposite","clockwise"})[NR])
	case RL_MOV:
		DISASM("MOV %s %s", REGISTER(di.data.reg_ds.destination), REGISTER(di.data.reg_ds.source));
	case RL_ADD:
		DISASM("ADD %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_SUB:
		DISASM("SUB %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_MUL:
		DISASM("MUL %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_DIV:
		DISASM("DIV %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_MOD:
		DISASM("MOD %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_AND:
		DISASM("AND %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_OR:
		DISASM("OR %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_NAND:
		DISASM("NAND %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_NOR:
		DISASM("NOR %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_XOR:
		DISASM("XOR %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_XNOR:
		DISASM("XNOR %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_SHL:
		DISASM("SHL %s %s %u", REGISTER(di.data.reg_dsn.destination), REGISTER(di.data.reg_dsn.source), (unsigned int)di.data.reg_dsn.n);
	case RL_SHR:
		DISASM("SHR %s %s %u", REGISTER(di.data.reg_dsn.destination), REGISTER(di.data.reg_dsn.source), (unsigned int)di.data.reg_dsn.n);
	case RL_BSH:
		DISASM("BSH %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_ABSH:
		DISASM("ABSH %s %s %s", REGISTER(di.data.reg_dab.destination), REGISTER(di.data.reg_dab.a), REGISTER(di.data.reg_dab.b));
	case RL_XCH:
		DISASM("XCH %s %s", REGISTER(di.data.reg_ab.a), REGISTER(di.data.reg_ab.b));
	case RL_TST:
		DISASM("TST %s", REGISTER(di.data.reg_r.reg));
	case RL_CMP:
		DISASM("CMP %s %s", REGISTER(di.data.reg_ab.a), REGISTER(di.data.reg_ab.b));
	case RL_IMM:
		DISASM("IMM %s %u", REGISTER(di.data.im_di.destination), (unsigned int)di.data.im_di.imm);
	case RL_IMMS:
		temp = di.data.im_di.imm;
		if(temp & 0x01FF) temp |= 0xFE00;
		DISASM("IMMS %s %d", REGISTER(di.data.im_di.destination), (int)temp);
	case RL_LOADBR:
		DISASM("LOADBR %s %s", REGISTER(di.data.im_ds.destination), REGISTER(di.data.im_ds.source));
	case RL_LOADBI:
		DISASM("LOADBI %s %03x", REGISTER(di.data.im_da.destination), (unsigned int)di.data.im_da.addr);
	case RL_LOADSR:
		DISASM("LOADSR %s %s", REGISTER(di.data.im_ds.destination), REGISTER(di.data.im_ds.source));
	case RL_LOADSI:
		DISASM("LOADSI %s %03x", REGISTER(di.data.im_da.destination), (unsigned int)di.data.im_da.addr);
	case RL_LOADWR:
		DISASM("LOADWR %s %s", REGISTER(di.data.im_ds.destination), REGISTER(di.data.im_ds.source));
	case RL_LOADWI:
		DISASM("LOADWI %s %03x", REGISTER(di.data.im_da.destination), (unsigned int)di.data.im_da.addr);
	case RL_STOREBR:
		DISASM("STOREBR %s %s", REGISTER(di.data.im_ds.destination), REGISTER(di.data.im_ds.source));
	case RL_STOREBI:
		DISASM("STOREBI %s %03x", REGISTER(di.data.im_sa.source), (unsigned int)di.data.im_sa.addr);
	case RL_STOREWR:
		DISASM("STOREWR %s %s", REGISTER(di.data.im_ds.source), REGISTER(di.data.im_ds.source));
	case RL_STOREWI:
		DISASM("STOREWI %s %03x", REGISTER(di.data.im_sa.source), (unsigned int)di.data.im_sa.addr);
	case RL_MXCH:
		DISASM("MXCH %s %s", REGISTER(di.data.im_ab.a), REGISTER(di.data.im_ab.b));
	case RL_Jc:
		if(di.data.j_cr.cond == RL_COND_ALWAYS) {
			DISASM("JMP %02x", (unsigned int)di.data.j_cr.relative);
		} else {
			DISASM("Jc %s %02x", COND(di.data.j_cr.cond), (unsigned int)di.data.j_cr.relative);
		}
	case RL_JRELc:
		if(di.data.j_cob.cond == RL_COND_ALWAYS) {
			DISASM("JREL %s %02x", REGISTER(di.data.j_cob.offset), (unsigned int)di.data.j_cob.base);
		} else {
			DISASM("Jc %s %s %02x", COND(di.data.j_cob.cond), REGISTER(di.data.j_cob.offset), (unsigned int)di.data.j_cob.base);
		}
	case RL_JEc:
		DISASM("JEc %1x %02x", (unsigned int)di.data.j_mr.mask, (unsigned int)di.data.j_mr.relative);
	case RL_JENc:
		DISASM("JENc %1x %02x", (unsigned int)di.data.j_mr.mask, (unsigned int)di.data.j_mr.relative);
	case RL_JAc:
		DISASM("JAc %1x %02x", (unsigned int)di.data.j_mr.mask, (unsigned int)di.data.j_mr.relative);
	case RL_JANc:
		DISASM("JANc %1x %02x", (unsigned int)di.data.j_mr.mask, (unsigned int)di.data.j_mr.relative);
	case RL_IN:
		DISASM("IN %s %s", REGISTER(di.data.io_dp.destination), PORT(di.data.io_dp.port));
	case RL_OUT:
		DISASM("OUT %s %s", REGISTER(di.data.io_sp.source), PORT(di.data.io_sp.port));
	case RL_INM:
		DISASM("INM %s %s", REGISTER(di.data.io_dp.destination), PORT(di.data.io_dp.port));
	case RL_OUTM:
		DISASM("OUTM %s %s", REGISTER(di.data.io_sp.source), PORT(di.data.io_sp.port));
	case RL_ILLEGAL_INSTR:
		DISASM("<illegal instruction>");
#undef DISASM
#undef REGISTER
#undef COND
#undef PORT
	}
}

struct ui_board_cell
rl_render(struct node *n)
{
	struct computer_data *cd;
	struct ui_board_cell ui_c;

	cd = n->data;

	ui_c.style = UI_STYLE_DEFAULT;
	rl_disassemble_instr(ui_c.status, sizeof(ui_c.status), computer_load_word(cd, cd->pc));
	return ui_c;
}

void
rl_inspect(struct node *n)
{
	char buf[4096];
	char *cur;
	unsigned int len, printed, i;
	struct computer_data *cd;
	struct rl_data *rld;
	char instr[32];

	cd = n->data;
	rld = cd->data;
	cur = buf;
	len = sizeof(buf);

	printed = snprintf(cur, len, "RiscLabs (tm)\nGeneral Purpose Processor\nPC: %" PRIx16 "\n", (uint16_t)cd->pc);
	cur += printed;
	len -= printed;
	printed = snprintf(cur, len, "Registers:\nr0    | r1\n%02" PRIx8 " %02" PRIx8 " | %02" PRIx8 " %02" PRIx8 "\n", (uint8_t)(rld->regs[0]>>8), (uint8_t)rld->regs[0], (uint8_t)(rld->regs[1]>>8), (uint8_t)rld->regs[1]);
	cur += printed;
	len -= printed;
	printed = snprintf(cur, len, "r2    | r3\n%02" PRIx8 " %02" PRIx8 " | %02" PRIx8 " %02" PRIx8 "\n", (uint8_t)(rld->regs[2]>>8), (uint8_t)rld->regs[2], (uint8_t)(rld->regs[3]>>8), (uint8_t)rld->regs[3]);
	cur += printed;
	len -= printed;
	if(rld->caps & CAP_REGS) {
		printed = snprintf(cur, len, "r4    | r5\n%02" PRIx8 " %02" PRIx8 " | %02" PRIx8 " %02" PRIx8 "\n", (uint8_t)(rld->regs[4]>>8), (uint8_t)rld->regs[4], (uint8_t)(rld->regs[5]>>8), (uint8_t)rld->regs[5]);
		cur += printed;
		len -= printed;
	}
	printed = snprintf(cur, len, "status\n%02" PRIx8 " %02" PRIx8 "\n", (uint8_t)(rld->regs[6]>>8), (uint8_t)rld->regs[6]);
	cur += printed;
	len -= printed;

	printed = snprintf(cur, len, "Disassembly:\n");
	cur += printed;
	len -= printed;

	for(i = 0; i < 10; ++i) {
#define BACK_INSTRS 2
		rl_disassemble_instr(instr, sizeof(instr), computer_load_word(cd, cd->pc + 2*i - 2*BACK_INSTRS));
		printed = snprintf(cur, len, "%c %s\n", i == BACK_INSTRS ? '>' : ' ', instr);
#undef BACK_INSTRS
		cur += printed;
		len -= printed;
	}

	computer_inspect_text(buf);
}

