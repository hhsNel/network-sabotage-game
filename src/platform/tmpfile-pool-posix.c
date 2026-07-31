#include "tmpfile.h"

#include <stdio.h>
#include <stdlib.h>

enum tmpfile_status { UNUSED, USED, FREED };
struct tracked_tmpfile {
	char filename[256];
	enum tmpfile_status status;
};

struct tracked_tmpfile tmpfile_pool[512];

void tmpfile_init() {
	unsigned int i;

	for(i = 0; i < sizeof(tmpfile_pool)/sizeof(*tmpfile_pool); ++i) {
		snprintf(tmpfile_pool[i].filename, sizeof(tmpfile_pool[i].filename), "/tmp/network-sabotage-%u", i);
		tmpfile_pool[i].status = UNUSED;
	}
}

void tmpfile_shutdown() {
	unsigned int i;

	for(i = 0; i < sizeof(tmpfile_pool)/sizeof(*tmpfile_pool); ++i) {
		if(tmpfile_pool[i].status != UNUSED) {
			remove(tmpfile_pool[i].filename);
		}
	}
}

tmpfile_id tmpfile_request() {
	unsigned int i;

	for(i = 0; i < sizeof(tmpfile_pool)/sizeof(*tmpfile_pool); ++i) {
		if(tmpfile_pool[i].status != USED) {
			tmpfile_pool[i].status = USED;
			return i;
		}
	}

	fprintf(stderr, "ran out of tmpfiles\n");
	exit(1);
}

char *tmpfile_get_name(tmpfile_id id) {
	if(id >= sizeof(tmpfile_pool)/sizeof(*tmpfile_pool)) return "";
	return tmpfile_pool[id].filename;
}

void tmpfile_free(tmpfile_id id) {
	if(id >= sizeof(tmpfile_pool)/sizeof(*tmpfile_pool)) return;
	tmpfile_pool[id].status = FREED;
}

