#ifndef UTIL_H
#define UTIL_H

#define _CONCAT2(A,B) A##B
#define CONCAT2(A,B) _CONCAT2(A,B)

#define BOARD_SZ 8

struct assembly_result {
	int success;
	unsigned int line; /* 0-indexed */
	unsigned int column; /* 0-indexed */
	char reason[64];
};

#endif

