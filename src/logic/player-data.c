#include "logic/player-data.h"

#include "logic/board.h"
#include "logic/port.h"

struct player_data
create_player() {
	struct player_data pd;
	unsigned int i;

	init_board(&pd.board);
	pd.ppd = create_ppd();

	for(i = 0; i < BOARD_SZ; ++i) {
		pd.req_gens[i] = create_request_generator();
		pd.resp_valid[i] = create_response_validator();
	}

	pd.money = 0;

	return pd;
}

void
update_player(struct player_data *pd, struct global_protocol *gp) {
	unsigned int i;
	struct protocol_generator_output gen;

	update_board(&pd->board);

	for(i = 0; i < BOARD_SZ; ++i) {
		if(port_write_available(&pd->board.request_ports[i])) {
			gen = run_request_generator(gp, &pd->ppd, &pd->req_gens[i]);
			port_write(&pd->board.request_ports[i], gen.num);
		}
	}

	for(i = 0; i < BOARD_SZ; ++i) {
		if(port_read_available(&pd->board.response_ports[i])) {
			if(run_response_validator(&pd->ppd, &pd->resp_valid[i], port_read(&pd->board.response_ports[i]))) {
				++ pd->money;
			}
		}
	}
}

