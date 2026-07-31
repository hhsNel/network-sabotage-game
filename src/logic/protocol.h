#ifndef LOGIC_PROTOCOL_H
#define LOGIC_PROTOCOL_H

#include "core/protocol.h"

union protocol_request_header {
	uint16_t words[2];
	struct {
		uint8_t length;
		uint8_t type;
		uint8_t id;
		uint8_t data;
	} data;
};

union protocol_response_header {
	uint16_t word;
	struct {
		uint8_t id;
		uint8_t length;
	} data;
};

struct protocol_generator_output { uint16_t num; int eof; };
struct protocol_request_generator {
	uint16_t buffer[sizeof(union protocol_request_header)+256];
	unsigned int length;
	unsigned int idx;
};

struct protocol_response_validator {
	union {
		union protocol_response_header header;
		uint16_t words[sizeof(union protocol_response_header)+256];
	} buffer;
	unsigned int idx;
};

struct sequence { uint8_t enabled; uint8_t len; uint16_t words[256]; };
struct protocol_player_data {
	struct sequence saved_sequences[256];
	struct sequence expected_sequences[256];
	uint8_t next_seq;
};

struct protocol_player_data create_ppd();

struct global_protocol init_protocol();
void mutate_protocol(struct global_protocol *gp, unsigned int points);
struct protocol_request_generator create_request_generator();
struct protocol_generator_output run_request_generator(struct global_protocol *gp, struct protocol_player_data *ppd, struct protocol_request_generator *prg);
struct protocol_response_validator create_response_validator();
int run_response_validator(struct protocol_player_data *ppd, struct protocol_response_validator *prv, uint16_t next_byte);

#endif

