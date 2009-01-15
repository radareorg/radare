#ifndef _INCLUDE_R_RADARE_
#define _INCLUDE_R_RADARE_

struct r_radare_t {
	struct r_core_t core;
};

int r_radare_init(struct r_radare_t *r);
int r_radare_cmd(struct r_radare_t *r, const char *cmd, int log);

#endif
