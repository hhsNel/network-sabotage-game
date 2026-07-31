#ifndef TMPFILE_H
#define TMPFILE_H

typedef unsigned int tmpfile_id;

void tmpfile_init();
void tmpfile_shutdown();
tmpfile_id tmpfile_request();
char *tmpfile_get_name(tmpfile_id id);
void tmpfile_free(tmpfile_id id);

#endif

