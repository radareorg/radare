/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_radare.h"

int r_radare_init(struct r_radare_t *r)
{
	r_core_init(&r->core);
	return 0;
}

int r_radare_cmd(struct r_radare_t *r, const char *cmd, int log)
{
	
}

int r_radare_seek(struct r_radare_t *r, u64 addr)
{
	r->core.seek = addr;
	r_io_lseek(r->core.io, 
	//r_core_seek(&r->core, addr);
}
