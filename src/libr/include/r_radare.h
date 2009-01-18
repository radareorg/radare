#ifndef _INCLUDE_R_RADARE_
#define _INCLUDE_R_RADARE_

#include "r_core.h"


struct r_radare_file_t {
	char *uri;
	char *filename;
	u64 seek;
	u64 size;
	int rwx;
	int fd;
	struct list_head list;
};

struct r_radare_t {
	struct r_core_t core;
	struct r_radare_file_t *file;
	struct list_head files;
};

int r_radare_init(struct r_radare_t *r);
int r_radare_cmd(struct r_radare_t *r, const char *cmd, int log);

#endif
