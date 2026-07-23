# ideas

## not exact spec, this might change!

 - each computer would have 4-6 (depending on the price) general purpose 16-bit registers
 - the basic model would also have 256 bytes of memory (big endian, no alignment restrictions)
 - there would be a special readonly (writes ommited) "status" register, with each bit each bit having its own meaning:
	1. the LSB means the "zero" flag
	2. bit '2' means the "negative" flag
	3. bit '4' means the "carry" (unsigned) flag
	4. bit '8' means the "overflow" (signed) flag
	5. bits '16' through '128' are 0.
	6. bit '256' is reserved for more expensive computers as "UP output is staled"
	7. bit '512' is reserved for more expensive computers as "RIGHT output is staled"
	8. bit '1024' is reserved for more expensive computers as "DOWN output is staled"
	9. bit '2048' is reserved for more expensive computers as "LEFT output is staled"
	10. bit '4096' means "UP input is staled"
	11. bit '8192' means "RIGHT input is staled"
	12. bit '16384' means "DOWN input is staled"
	13. the MSB means "LEFT input is staled"
 - each computer would additionally have a "ZERO" register, where reads produce 0x0000 and writing to it is ommited
 - there would also be an 8-bit program counter, as an inaccessible register (outside of jumps)
 - for IO, there would be the following ports and pseudoports:
	1. the "UP" port
	2. the "RIGHT" port
	3. the "DOWN" port
	4. the "LEFT" port
	5. on more expensive computers, the "ANY" port (which saves itself in an inaccessible register)
	6. on more expensive computers, the "LAST" port (which reads from that inaccessible register)
	7. on even more expensive computers, the "OPPOSITE" port (which is the opposite of the port saved in that inaccessible register)
	8. on even more expensive computers, the "CLOCKWISE" port (which is 1 direction clockwise from the port saved in that inaccessible register)
 - reading from a non-ready port or writing to a stalled port idles until that port becomes ready or non-stalled
 - all computers would execute instructions ever N global ticks, with an 8Hz global clock, and most computers having N=2
 - there would be 4 instruction types across a 5 or 6 bit opcode (which also makes implementing this in C trivial as a bitmask for supported instructions and a LUT of function pointers per architecture):
	1. Register instructions:
		- MOV
		- arithmetic (ADD, SUB, as well as MUL, DIV and MOD on more expensive models)
		- logic (AND, NAND, OR, NOR, XOR, XNOR, some more specialized operations like XNOR locked behind better models)
		- bitshifts
		- probably NOP
		- XCH
		- TST (set flags based on a single register)
		- CMP (set flags based on a subtraction between 2 registers, in more expensive computers)
	2. Imm/Mem instruction (low byte / 9 bits taken from a register):
		- IMM (set register to zero-extended immediate)
		- IMMS (set register to sign-extended immediate)
		- LOADBR / LOADBI (register/immediate, set register to zero-extended byte in memory)
		- LOADSR / LOADSI (register/immediate, set register to sign-extended byte in memory)
		- LOADWR / LOADWI (register/immediate, set register to word in memory)
		- STOREBR / STOREBI (register/immediate, store the low byte of a register in memory)
		- STOREWR / STOREWI (register/immediate, store a register in memory)
		- some kind of MXCH which swaps two bytes in memory, on more expensive computers
	3. Jumps
		- JMP
		- Jc (jump if single flag)
		- JXc (jump if a bitmask of selected flags isn't zero, eg. JXzn means "jump if zero or negative", only on better computers)
		- JREL (jump to the lower 8 bits of a register, on more expensive computers)
		- JRELc (jump, if a single flag, to the lower 8 bits of a register, on REALLY expensive computers)
	4. IO instructions:
		- IN (read from port to register)
		- OUT (write from register to port)
		- IN and OUT directly to memory, on more expensive computers
 - PC wraps on overflow
 - some computers would have a 9-bit PC for a total of 512 bytes of memory
 - in later versions of the game, there might be "discount 100% totally working" computers that for example have a bit of a register stuck at 0, or have some other "hardware defect"
 - not sure, but probably an illegal opcode is treated like a no-op?

# ISA

`?` means any bit

## Instructions

| Bits                                  | Mnemonic | Result |
| ------------------------------------- | -------- | ------ |
| [0][0][000][DDD]   [SSS][???][00]   | MOV      | Copies the S register to D |
| [0][0][000][DDD]   [AAA][BBB][01]   | ADD      | Adds the A and B registers, and stores the sum in D. Sets ZNCV |
| [0][0][000][DDD]   [AAA][BBB][10]   | SUB      | Subtracts the B register from the A register, and stores the result in D. Sets ZNCV |
| [0][0][001][DDD]   [AAA][BBB][00]   | MUL      | Integer signed multiplies the A and B registers together, then stores the result in D. Sets ZN. Isn't available on all computers |
| [0][0][001][DDD]   [AAA][BBB][01]   | DIV      | Integer signed divides the A register by the B register, and stores the result in D. Division by 0 results in 0. Sets ZN. Isn't available on all computers |
| [0][0][001][DDD]   [AAA][BBB][10]   | MOD      | Stores the remainder of integer digned division of the A regster by the B register into D. Modulo 0 results in 0. Sets ZN. Isn't available on all computers |
| [0][0][010][DDD]   [AAA][BBB][?][0] | AND      | Bitands the A and B registers, then stores the result in D. Sets ZN |
| [0][0][010][DDD]   [AAA][BBB][?][1] | OR       | Bitors the A and B registers, then stores the result in D. Sets ZN |
| [0][0][011][DDD]   [AAA][BBB][00]   | NAND     | Bitwise nands the A and B registers, then stores the result in D. Sets ZN. Isn't available on all computers |
| [0][0][011][DDD]   [AAA][BBB][01]   | NOR      | Bitwise nors the A and B registers, then stores the result in D. Sets ZN. Isn't available on all computers |
| [0][0][011][DDD]   [AAA][BBB][10]   | XOR      | Bitwise xors the A and B registers, then stores the result in D. Sets ZN. Isn't available on all computers |
| [0][0][011][DDD]   [AAA][BBB][11]   | XNOR     | Bitwise xnors the A and B registers, then stores the result in D. Sets ZN. Isn't available on all computers |
| [0][0][100][DDD]   [SSS][0][NNNN]   | SHL      | Shifts the S register by N to the left, then stores the result in D. Sets ZNC |
| [0][0][100][DDD]   [SSS][1][NNNN]   | SHR      | Shifts the S register by N to the right, then stores the result in D. Sets ZNC |
| [0][0][101][DDD]   [AAA][BBB][?][0] | BSH      | If B is positive, shifts A right by the lowest nibble of B. Otherwise, shifts A left by the lowest nibble of abs(B). Stores the result in D. Sets ZNC. Isn't available on all computers |
| [0][0][101][DDD]   [AAA][BBB][?][1] | ABSH     | If B is positive, shifts all but the MSB of A right by the lowest nibble of B, shifting in bits equal to A's MSB. Otherwise, shifts A left by the lowest nibble of abs(B). Stores the result in D. Sets ZNC. Isn't available on all computers |
| [0][0][110][???]   [000][000][??]   | NOP      | Does nothing |
| [0][0][110][???]   [AAA][BBB][??]   | XCH      | Writes the value of the B register into the A register, and simultaniously writes the value of A into B |
| [0][0][111][???]   [RRR][????]  [0] | TST      | Sets some flags in the Status register based on the value of R. Sets ZN |
| [0][0][111][???]   [AAA][BBB][?][1] | CMP      | Sets some flags in the Status register based on the subtraction A - B. Sets ZN. Isn't available on all computers |
| [1][000][DDD][I    IIIIIIII]        | IMM      | Writes the zero-extended 9-bit immediate value I into the register D |
| [1][001][DDD][I    IIIIIIII]        | IMMS     | Writes the sign-extended 9-bit immediate value I into the register D |
| [1][111][000][?]   [DDD][SSS][??]   | LOADBR   | Sets the D register to the zero-extended byte pointed to by the lowest bits of the S register |
| [1][010][DDD][A    AAAAAAAA]        | LOADBI   | Sets the D register to the zero-extended byte pointed to by A |
| [1][111][001][?]   [DDD][SSS][??]   | LOADSR   | Sets the D register to the sign-extended byte pointed to by the lowest bits of the S register |
| [1][011][DDD][A    AAAAAAAA]        | LOADSI   | Sets the D register to the sign-extended byte pointed to by A |
| [1][111][010][?]   [DDD][SSS][??]   | LOADWR   | Sets the D register to the word (2 consecutive bytes) pointed to by the lowest bits of the S register |
| [1][100][DDD][A    AAAAAAAA]        | LOADWI   | Sets the D register to the word (2 consecutive bytes) pointed to by A |
| [1][111][011][?]   [DDD][SSS][??]   | STOREBR  | Sets the byte pointed to by the lowest bits of the D register to the lower byte of the S register |
| [1][101][SSS][A    AAAAAAAA]        | STOREBI  | Sets the byte pointed to by A to teh lower byte of the S register |
| [1][111][100][?]   [DDD][SSS][??]   | STOREWR  | Sets the word (2 consecutive bytes) pointed to by the lowest bits of the D register to the S register |
| [1][110][SSS][A    AAAAAAAA]        | STOREWI  | Sets the word (2 consecutive bytes) pointed to by A to the S register |
| [1][111][101][?]   [AAA][BBB][??]   | MXCH     | Sets the byte pointed to by the A register to the byte pointed to by the B register, and simultaniously sets the byte pointed to by B to the byte pointed to by A. Isn't available on all computers |
| [0][1][0][00][000] [RRRRRRRR]       | JMP      | Adds R (signed) to the program counter after incrementing it |
| [0][1][0][00][CCC] [RRRRRRRR]       | Jc       | Adds R (signed) to the program counter after incrementing it, iff. the condition C is true |
| [0][1][0][01][000] [OOO][BBBBB]     | JREL     | Sets the program counter to the lowest bits of the O register plus B (unsigned) instead of incrementing it. Isn't available on all computers |
| [0][1][0][01][CCC] [OOO][BBBBB]     | JRELc    | Sets the program counter to the lowest bits of the O register plus B (unsigned) instead of incrementing it, iff. the condition C is true. Isn't available on all computers |
| [0][1][0][10][MMM] [M][0][RRRRRR]   | JEc      | Adds R (signed) to the program counter after incrementing it, iff. the lowest 4 bits of the Status register bitanded with M gives a nonzero number. Isn't available on all computers |
| [0][1][0][10][MMM] [M][1][RRRRRR]   | JENc     | Adds R (signed) to the program counter after incrementing it, iff. the lowest 4 bits of the Status register bitanded with M gives zero. Isn't available on all computers |
| [0][1][0][11][MMM] [M][0][RRRRRR]   | JAc      | Adds R (signed) to the program counter after incrementing it, iff. the lowest 4 bits of the Status register bitanded with M gives M. Isn't available on all computers |
| [0][1][0][11][MMM] [M][1][RRRRRR]   | JANc     | Adds R (signed) to the program counter after incrementing it, iff. the lowest 4 bits of the Status register bitanded with M doesn't give M. Isn't available on all computers |
| [0][1][1][00][DDD] [PPP][?????]     | IN       | Reads the value of the P port into the D register |
| [0][1][1][01][SSS] [PPP][?????]     | OUT      | Writes the value of the S register to the P port |
| [0][1][1][10][DDD] [PPP][?????]     | INM      | Reads the value of the P port into the word (2 consecutive bytes) pointed to by the D register. Isn't available on all computers |
| [0][1][1][11][SSS] [PPP][?????]     | OUTM     | Writes the value of the word (2 consecutive bytes) pointed to by the S register to the P port. Isn't available on all computers |

## Registers

| Bits  | Name   | Comments |
| ----- | ------ | -------- |
| [000] | r0     | |
| [001] | r1     | |
| [010] | r2     | |
| [011] | r3     | |
| [100] | r4     | Isn't available on all computers |
| [101] | r5     | Isn't available on all computers |
| [110] | Status | Writes are ommited. Is set by some instructions or the hardware |
| [111] | Zero   | Writes are ommited. Reads always produce 0 |

## Ports

The `LAST_PORT` internal register is initialized to UP. Reads and writes to unavailable ports are ommited.

| Bits  | Name      | Comments |
| ----- | --------- | -------- |
| [000] | UP        | Directional connection |
| [001] | RIGHT     | Directional connection |
| [010] | DOWN      | Directional connection |
| [011] | LEFT      | Directional connection |
| [100] | ANY       | When read from, it looks for any port in the order UP, RIGHT, DOWN, LEFT. If one is found, sets the internal register `LAST_PORT`. When written to, behaves like LAST. Isn't available on all computers |
| [101] | LAST      | Reads or writes to the port in the internal register `LAST_PORT`. Isn't available on all computers |
| [110] | OPPOSITE  | Reads or writes to the port opposite of the one in the internal register `LAST_PORT`. Isn't available on all computers |
| [111] | CLOCKWISE | Reads or writes to the port clockwise from the one in the internal register `LAST_PORT`. Isn't available on all computers |

## Conditions

| Bits  | Name     | Comments |
| ----- | -------- | -------- |
| [000] | Always   | Is always true |
| [001] | Zero     | The Z flag is 1 |
| [010] | Neg      | The N flag is 1 |
| [011] | Carry    | The C flag is 1 |
| [100] | No Carry | The C flag is 0 |
| [101] | Overflow | The V flag is 1 |
| [110] | Nonzero  | The Z flag is 0 |
| [111] | Pos      | Both the Z and N flags are 0 |

## Status

| Bit number | Bit value | Name       | Comments |
| ---------- | --------- | ---------- | -------- |
| 1          | 1         | Z          | Condition flag |
| 2          | 2         | N          | Condition flag |
| 3          | 4         | C          | Condition flag. Isn't available on all computers, 0 when unsupported |
| 4          | 8         | V          | Condition flag. Isn't available on all computers, 0 when unsupported |
| 5          | 16        | Unused     | Always 0 |
| 6          | 32        | Unused     | Always 0 |
| 7          | 64        | Unused     | Always 0 |
| 8          | 128       | Unused     | Always 0 |
| 9          | 256       | UP\_OUT    | 1 iff. UP is not available for output. Isn't available on all computers, 0 when unsupported |
| 10         | 512       | RIGHT\_OUT | 1 iff. RIGHT is not available for output. Isn't available on all computers, 0 when unsupported |
| 11         | 1024      | DOWN\_OUT  | 1 iff. DOWN is not available for output. Isn't available on all computers, 0 when unsupported |
| 12         | 2048      | LEFT\_OUT  | 1 iff. LEFT is not available for output. Isn't available on all computers, 0 when unsupported |
| 13         | 4096      | UP\_IN     | 1 iff. UP is not available for input. Isn't available on all computers, 0 when unsupported |
| 14         | 8192      | RIGHT\_IN  | 1 iff. RIGHT is not available for input. Isn't available on all computers, 0 when unsupported |
| 15         | 16384     | DOWN\_IN   | 1 iff. DOWN is not available for input. Isn't available on all computers, 0 when unsupported |
| 16         | 32768     | LEFT\_IN   | 1 iff. LEFT is not available for input. Isn't available on all computers, 0 when unsupported |

