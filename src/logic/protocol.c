#include "logic/protocol.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static struct protocol_task generate_task(unsigned int points);
static void generate_request(struct global_protocol *gp, struct protocol_player_data *ppd, struct protocol_request_generator *prg);
static void save_task_answer(struct global_protocol *gp, struct protocol_player_data *ppd, union protocol_request_header hdr, uint16_t *buffer);
static void eval_task(struct global_protocol *gp, struct protocol_task tsk, uint8_t len, uint16_t *A, uint16_t *B, uint16_t *out_buf, uint8_t *out_len);
static uint16_t run_linear_task(struct protocol_task tsk, uint16_t A, uint16_t B);

struct global_protocol
init_protocol() {
	struct global_protocol gp;
	unsigned int i;

	gp.tasks[0] = (struct protocol_task){
		1,
		0,
		PR_TSK_MUT_NONE,
		PR_TSK_SRC_DATA,
		PR_TSK_DST_RESPONSE
	};

	for(i = 0; i < 256; ++i) {
		gp.tasks[i].enabled = 0;
	}

	return gp;
}

void
mutate_protocol(struct global_protocol *gp, unsigned int points) {
	unsigned int id;

	id = 1 + rand() / ((RAND_MAX+1u)/255);
	gp->tasks[id] = generate_task(points);
	gp->tasks[id].id = id;
}

struct protocol_player_data
create_ppd() {
	struct protocol_player_data ppd;
	unsigned int i;
	
	for(i = 0; i < 256; ++i) {
		ppd.saved_sequences[i].enabled = 0;
		ppd.expected_sequences[i].enabled = 0;
	}

	ppd.next_seq = 0;

	return ppd;
}

struct protocol_request_generator
create_request_generator() {
	struct protocol_request_generator prg;

	prg.length = prg.idx = 0;

	return prg;
}

struct protocol_generator_output
run_request_generator(struct global_protocol *gp, struct protocol_player_data *ppd, struct protocol_request_generator *prg) {
	struct protocol_generator_output out;

	if(prg->idx == prg->length) {
		prg->idx = 0;
		generate_request(gp, ppd, prg);
	}

	out.num = prg->buffer[prg->idx++];
	if(prg->idx == prg->length) out.eof = 1;
	else out.eof = 0;

	return out;
}

struct protocol_response_validator
create_response_validator() {
	struct protocol_response_validator prv;

	prv.idx = 0;

	return prv;
}

int
run_response_validator(struct protocol_player_data *ppd, struct protocol_response_validator *prv, uint16_t next_byte) {
	int ret_val;

	ret_val = 0;

	prv->buffer.words[prv->idx++] = next_byte;

	if(prv->idx >= 1 && prv->idx - 1 == prv->buffer.header.data.length) {
		if( ppd->expected_sequences[prv->buffer.header.data.id].enabled &&
			prv->buffer.header.data.length == ppd->expected_sequences[prv->buffer.header.data.id].len &&
			memcmp(&prv->buffer.words[1],
					ppd->expected_sequences[prv->buffer.header.data.id].words,
					prv->buffer.header.data.length) == 0 ) {
			ret_val = 1;
		}
		prv->idx = 0;
		ppd->expected_sequences[prv->buffer.header.data.id].enabled = 0;
		++ ppd->next_seq;
	}

	return ret_val;
}

static struct protocol_task
generate_task(unsigned int points) {
	struct protocol_task tsk;

	if(points == 0) return (struct protocol_task){1,0,PR_TSK_MUT_NONE,PR_TSK_SRC_DATA,PR_TSK_DST_RESPONSE};
	if(points >= NUM_PR_TSK_MUT + NUM_PR_TSK_SRC + NUM_PR_TSK_DST) {
		points = NUM_PR_TSK_MUT + NUM_PR_TSK_SRC + NUM_PR_TSK_DST - 1;
	}

	while(1) {
		tsk.mut = rand() / ((RAND_MAX+1u) / NUM_PR_TSK_MUT);
		tsk.src = rand() / ((RAND_MAX+1u) / NUM_PR_TSK_SRC);

		if(tsk.mut + tsk.src > points) continue;

		if(points - tsk.mut - tsk.src >= NUM_PR_TSK_DST) continue;

		tsk.dst = points - tsk.mut - tsk.src;
		return tsk;
	}
}

static void
generate_request(struct global_protocol *gp, struct protocol_player_data *ppd, struct protocol_request_generator *prg) {
	uint8_t possible_task_idcs[256];
	unsigned int num_possible_tasks;
	unsigned int i, j;
	uint8_t chosen_task;
	union protocol_request_header head;
	uint8_t possible_indirection_ids[256];
	unsigned int num_possible_indirections;
	uint8_t indirection_id;

	possible_task_idcs[0] = 0;
	num_possible_tasks = 1;
	for(i = 1; i < 256; ++i) {
		if(gp->tasks[i].enabled) {
			if(gp->tasks[i].src != PR_TSK_SRC_INDIRECT) {
				possible_task_idcs[num_possible_tasks++] = i;
			} else {
				for(j = 0; j < 256; ++j) {
					if(j != ppd->next_seq && ppd->saved_sequences[j].enabled) {
						possible_task_idcs[num_possible_tasks++] = i;
						break;
					}
				}
			}
		}
	}

	chosen_task = possible_task_idcs[rand() / ((RAND_MAX+1u) / num_possible_tasks)];
	head.data.type = chosen_task;
	head.data.id = ppd->next_seq;
	if(gp->tasks[chosen_task].src != PR_TSK_SRC_INDIRECT) {
		head.data.data = rand() / ((RAND_MAX+1u) / 256);
		head.data.length = 8 + rand() / ((RAND_MAX+1u) / 240); /* TODO in the future I should probably add request length scaling with time */
	} else {
		num_possible_indirections = 0;
		for(i = 0; i < 256; ++i) {
			if(i != ppd->next_seq && ppd->saved_sequences[i].enabled) {
				possible_indirection_ids[num_possible_indirections++] = i;
			}
		}
		indirection_id = rand() / ((RAND_MAX+1u) / num_possible_indirections);
		head.data.data = possible_indirection_ids[indirection_id];
		head.data.length = ppd->saved_sequences[possible_indirection_ids[indirection_id]].len;
		ppd->saved_sequences[possible_indirection_ids[indirection_id]].enabled = 0;
	}
	prg->buffer[0] = head.words[0];
	prg->buffer[1] = head.words[1];

	for(i = 0; i < head.data.length; ++i) {
		prg->buffer[2 + i] = rand() / ((RAND_MAX+1u) / 65536);
	}
	
	save_task_answer(gp, ppd, head, &prg->buffer[2]);

	prg->length = 2 + head.data.length;
}

static void
save_task_answer(struct global_protocol *gp, struct protocol_player_data *ppd, union protocol_request_header hdr, uint16_t *buffer) {
	unsigned int i;
	struct sequence *out;
	uint16_t b_buf[256];

	if(gp->tasks[hdr.data.type].dst == PR_TSK_DST_RESPONSE) {
		out = &ppd->expected_sequences[hdr.data.id];
	} else {
		out = &ppd->saved_sequences[hdr.data.id];
	}

	for(i = 0; i < hdr.data.length; ++i) {
		if(gp->tasks[hdr.data.type].src == PR_TSK_SRC_DATA) {
			b_buf[i] = hdr.data.data * 0x101;
		} else {
			b_buf[i] = ppd->saved_sequences[hdr.data.data].words[i];
		}
	}

	out->enabled = 1;
	eval_task(gp, gp->tasks[hdr.data.type], hdr.data.length, buffer, b_buf, out->words, &out->len);
}

static void
eval_task(struct global_protocol *gp, struct protocol_task tsk, uint8_t len, uint16_t *A, uint16_t *B, uint16_t *out_buf, uint8_t *out_len) {
	unsigned int i;
	unsigned int strlen_a, strlen_b;
	uint8_t decompressed_out_len;

	switch(tsk.mut) {
#define VERIFY(MACRO) \
		for(i = 0; i < len; ++i) { \
			if(! (MACRO(A[i], B[i]))) break; \
		} \
		if(i == len) { \
			*out_len = len; \
			for(i = 0; i < len; ++i) { \
				out_buf[i] = A[i]; \
			} \
		} else { \
			*out_len = 1; \
			out_buf[0] = i; \
		}
	case PR_TSK_MUT_VERIFY_NE:
#define COND(A,B) ((A) != (B))
		VERIFY(COND);
#undef COND
		break;
	case PR_TSK_MUT_VERIFY_GREATER:
#define COND(A,B) ((uint16_t)(A) > (uint16_t)(B))
		VERIFY(COND);
#undef COND
		break;
	case PR_TSK_MUT_VERIFY_LESS:
#define COND(A,B) ((uint16_t)(A) < (uint16_t)(B))
		VERIFY(COND);
#undef COND
		break;
	case PR_TSK_MUT_VERIFY_ABOVE:
#define COND(A,B) ((int16_t)(A) > (int16_t)(B))
		VERIFY(COND);
#undef COND
		break;
	case PR_TSK_MUT_VERIFY_BELOW:
#define COND(A,B) ((int16_t)(A) < (int16_t)(B))
		VERIFY(COND);
#undef COND
		break;
#undef VERIFY
	case PR_TSK_MUT_STRLEN:
		for(strlen_a = 0; strlen_a < len && A[strlen_a] != 0; ++strlen_a);
		if(tsk.src == PR_TSK_SRC_DATA) {
			strlen_b = B[0] & 0x00FF;
		} else {
			for(strlen_b = 0; strlen_b < len && B[strlen_b] != 0; ++strlen_b);
		}
		*out_len = 1;
		if(strlen_a <= strlen_b) out_buf[0] = strlen_a;
		else out_buf[0] = strlen_b - strlen_a;
		break;
	case PR_TSK_MUT_COMPRESSED:
		*out_len = len;
		for(i = 0; i < len; ++i) {
			if(! gp->tasks[A[i]>>8].enabled || gp->tasks[A[i]>>8].mut == PR_TSK_MUT_COMPRESSED) {
				out_buf[i] = (B[i] & 0xFF00) | (A[i] & 0x00FF);
			} else {
				eval_task(gp, gp->tasks[A[i]>>8], 1, A+i, B+i, out_buf+i, &decompressed_out_len);
				if(decompressed_out_len != 1) {
					fprintf(stderr, "something went very wrong: decompressed task wrote more than 1 word\n");
					exit(1);
				}
			}
		}
		break;
	default:
		*out_len = len;
		for(i = 0; i < len; ++i) {
			out_buf[i] = run_linear_task(tsk, A[i], B[i]);
		}
		break;
	}
}

static uint16_t
run_linear_task(struct protocol_task tsk, uint16_t A, uint16_t B) {
	switch(tsk.mut) {
	case PR_TSK_MUT_NONE:
		return A;
	case PR_TSK_MUT_ADD:
		return A + B;
	case PR_TSK_MUT_SUB:
		return A - B;
	case PR_TSK_MUT_AND:
		return A & B;
	case PR_TSK_MUT_OR:
		return A | B;
	case PR_TSK_MUT_MUL:
		return A * B;
	case PR_TSK_MUT_DIV:
		if(B == 0) return 0;
		return A / B;
	case PR_TSK_MUT_MOD:
		if(B == 0) return 0;
		return A % B;
	case PR_TSK_MUT_NAND:
		return ~(A & B);
	case PR_TSK_MUT_NOR:
		return ~(A | B);
	case PR_TSK_MUT_XOR:
		return A ^ B;
	case PR_TSK_MUT_XNOR:
		return ~(A ^ B);
	case PR_TSK_MUT_IMPL:
		return (~A) | B;
	case PR_TSK_MUT_MAX:
		if(A > B) return A;
		return B;
	case PR_TSK_MUT_MIN:
		if(A < B) return A;
		return B;
	default:
		fprintf(stderr, "something went very wrong: bad mutation in run_linear_task\n");
		exit(1);
	}
}

