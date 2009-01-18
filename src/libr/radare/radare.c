/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include <r_radare.h>

int r_radare_init(struct r_radare_t *r)
{
	r_core_init(&r->core);
	INIT_LIST_HEAD(&r->files);
	return 0;
}

int r_radare_prompt(struct r_radare_t *r)
{
	char line[1024];
	int ret;

	ret = r_cons_fgets(line, sizeof(line), 0 , NULL);
	if (ret<0)
		return 0;
	return r_radare_cmd(r, line, TRUE);
}

int r_radare_cmd(struct r_radare_t *r, const char *cmd, int log)
{
	printf("run(%s)\n", cmd);
	return 0;
}

int r_radare_seek(struct r_radare_t *r, u64 addr)
{
	r->core.seek = addr;
	r_io_lseek(r->file->fd, addr, R_IO_SEEK_SET);
	//r_core_seek(&r->core, addr);
}
