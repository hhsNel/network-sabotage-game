#ifndef CORE_RISC_LABS_H
#define CORE_RISC_LABS_H

#include "core/computer.h"
#include "core/util.h"

enum rl_reg {
	RL_REG_R0 = 0,
	RL_REG_R1 = 1,
	RL_REG_R2 = 2,
	RL_REG_R3 = 3,
	RL_REG_R4 = 4,
	RL_REG_R5 = 5,
	RL_REG_STATUS = 6,
	RL_REG_ZERO = 7,
};

enum rl_port {
	RL_PORT_UP = 0,
	RL_PORT_RIGHT = 1,
	RL_PORT_DOWN = 2,
	RL_PORT_LEFT = 3,
	RL_PORT_ANY = 4,
	RL_PORT_LAST = 5,
	RL_PORT_OPPOSITE = 6,
	RL_PORT_CLOCKWISE = 7,
};

enum rl_condition {
	RL_COND_ALWAYS = 0,
	RL_COND_ZERO = 1,
	RL_COND_NEG = 2,
	RL_COND_CARRY = 3,
	RL_COND_NO_CARRY = 4,
	RL_COND_OVERFLOW = 5,
	RL_COND_NONZERO = 6,
	RL_COND_POS = 7,
};

#define RL_STATUS_Z 0x0001
#define RL_STATUS_N 0x0002
#define RL_STATUS_C 0x0004
#define RL_STATUS_V 0x0008
#define RL_STATUS_UP_OUT 0x0100
#define RL_STATUS_RIGHT_OUT 0x0200
#define RL_STATUS_DOWN_OUT 0x0400
#define RL_STATUS_LEFT_OUT 0x0800
#define RL_STATUS_UP_IN 0x1000
#define RL_STATUS_RIGHT_IN 0x2000
#define RL_STATUS_DOWN_IN 0x4000
#define RL_STATUS_LEFT_IN 0x8000

enum rl_opcode {
	RL_MOV     = 0,
	RL_ADD     = 1,
	RL_SUB     = 2,
	RL_MUL     = 3,
	RL_DIV     = 4,
	RL_MOD     = 5,
	RL_AND     = 6,
	RL_OR      = 7,
	RL_NAND    = 8,
	RL_NOR     = 9,
	RL_XOR     = 10,
	RL_XNOR    = 11,
	RL_SHL     = 12,
	RL_SHR     = 13,
	RL_BSH     = 14,
	RL_ABSH    = 15,
	RL_XCH     = 16,
	RL_TST     = 17,
	RL_CMP     = 18,
	RL_IMM     = 19,
	RL_IMMS    = 20,
	RL_LOADBR  = 21,
	RL_LOADBI  = 22,
	RL_LOADSR  = 23,
	RL_LOADSI  = 24,
	RL_LOADWR  = 25,
	RL_LOADWI  = 26,
	RL_STOREBR = 27,
	RL_STOREBI = 28,
	RL_STOREWR = 29,
	RL_STOREWI = 30,
	RL_MXCH    = 31,
	RL_Jc      = 32,
	RL_JRELc   = 33,
	RL_JEc     = 34,
	RL_JENc    = 35,
	RL_JAc     = 36,
	RL_JANc    = 37,
	RL_IN      = 38,
	RL_OUT     = 39,
	RL_INM     = 40,
	RL_OUTM    = 41,
	RL_ILLEGAL_INSTR = -1,
};

typedef uint64_t rl_capabilities;

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

struct rl_data {
	rl_capabilities caps;
	uint16_t regs[7]; /* plus the ZERO register */
	uint8_t last_port : 2;
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

struct assembly_result rl_asm(uint8_t *out_buf, size_t out_size, char *in_str);
struct decoded_instr decode(uint16_t raw_instr);

#endif

