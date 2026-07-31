#ifndef CORE_PROTOCOL_H
#define CORE_PROTOCOL_H

#include <stdint.h>

enum protocol_task_mutation {
	PR_TSK_MUT_NONE, /* ignore B */
	PR_TSK_MUT_ADD,
	PR_TSK_MUT_SUB,
	PR_TSK_MUT_AND,
	PR_TSK_MUT_OR,
	PR_TSK_MUT_MUL,
	PR_TSK_MUT_DIV,
	PR_TSK_MUT_MOD,
	PR_TSK_MUT_NAND,
	PR_TSK_MUT_NOR,
	PR_TSK_MUT_XOR,
	PR_TSK_MUT_XNOR,
	PR_TSK_MUT_IMPL, /* for each bit, if either A is 0 or B is 1, output is 1; otherwise 0 */
	PR_TSK_MUT_MAX, /* for each word, take the unsigned maximum of A and B */
	PR_TSK_MUT_MIN, /* for each word, take the unsigned minimum of A and B */
	PR_TSK_MUT_VERIFY_NE, /* if each word of A is not equal to the corresponding word of B, output A. Otherwise, output 1 word, the index of the first word of A equal to that of B */
	PR_TSK_MUT_VERIFY_GREATER, /* if each word of A is greater (signed greater) than the corresponding word of B, output A. Otherwise, output 1 word, the index of the first word of A less or equal to that of B */
	PR_TSK_MUT_VERIFY_LESS, /* if each word of A is less (signed less) than the corresponding word of B, output A. Otherwise, output 1 word, the index of the first word of A greater or equal to that of B */
	PR_TSK_MUT_VERIFY_ABOVE, /* if each word of A is above (unsigned greater) than the corresponding word of B, output A. Otherwise, output 1 word, the index of the first word of A less or equal to that of B */
	PR_TSK_MUT_VERIFY_BELOW, /* if each word of A is below (unsigned less) than the corresponding word of B, output A. Otherwise, output 1 word, the index of the first word of A above or equal to that of B */
	PR_TSK_MUT_STRLEN, /* let strlen(number) be that number, and strlen(sequence) be min(sequence.length, first_index_of_a_0x0000_word). If strlen(A) <= strlen(B), output one word: strlen(A). Otherwise output one word: strlen(B) - strlen(A) */
	PR_TSK_MUT_COMPRESSED, /* for each word of A, extract the high and low bytes of A. If the high byte isn't a defined request ID or its mutation is also COMPRESSED, output a word with the high byte equal to B's high byte and low byte equal to A's low byte. If it is, perform that request on a 1-length sequence A' equal to A's low byte and 1-length sequence B' equal to B's low byte, then output that */
	NUM_PR_TSK_MUT
};

enum protocol_task_source {
	PR_TSK_SRC_DATA, /* B is a sequence with length equal to A, with each word being the request data times 0x101 */
	PR_TSK_SRC_INDIRECT, /* both A and B's length must match. B is sequence saved earlier with the index equal to the request data */
	NUM_PR_TSK_SRC
};

enum protocol_task_destination {
	PR_TSK_DST_RESPONSE, /* output to one of the response ports */
	PR_TSK_DST_SAVE, /* save this request to be used later */
	NUM_PR_TSK_DST
};

struct protocol_task {
	uint8_t enabled;
	uint8_t id;
	enum protocol_task_mutation mut;
	enum protocol_task_source src;
	enum protocol_task_destination dst;
};

struct global_protocol {
	struct protocol_task tasks[256];
};

#endif

