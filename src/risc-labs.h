#ifndef RISC_LABS_H
#define RISC_LABS_H

#include "computer.h"

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

struct node create_rl_computer(unsigned int points);

#endif

