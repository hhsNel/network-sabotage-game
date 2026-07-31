#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include "core/board.h"
#include "core/protocol.h"
#include "logic/protocol.h"

struct player_data {
	struct board board;
	struct protocol_player_data ppd;
	struct protocol_request_generator req_gens[BOARD_SZ];
	struct protocol_response_validator resp_valid[BOARD_SZ];
	unsigned int money;
};

struct player_data create_player();
void update_player(struct player_data *pd, struct global_protocol *gp);

#endif

