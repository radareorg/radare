/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"

int r_core_init(struct r_core_t *core)
{
	core->seek = 0LL;
	core->blocksize = R_CORE_BLOCKSIZE;
	INIT_LIST_HEAD(&core->files);
	r_core_cmd_init(core);
	r_cons_init();
	r_io_init();
	core->file = NULL;
	core->block = (u8*)malloc(R_CORE_BLOCKSIZE);
	r_core_config_init(core);
	return 0;
}

int r_core_prompt(struct r_core_t *r)
{
	char prompt[32];
	char line[1024];
	int ret;

	sprintf(prompt, "[0x%08llx]> ", r->seek);
	r_line_prompt = prompt;
	ret = r_cons_fgets(line, sizeof(line), 0 , NULL);
	if (ret<0)
		return -1;
	ret = r_core_cmd(r, line, TRUE);
	r_cons_flush();
	return ret;
}

int r_core_block_size(struct r_core_t *core, u32 bsize)
{
	int ret = 0;
	if (bsize<1)
		bsize = 1;
	else if (bsize> R_CORE_BLOCKSIZE_MAX)
		bsize = R_CORE_BLOCKSIZE_MAX;
	else ret = 1;
	core->block = realloc(core->block, bsize);
	core->blocksize = bsize;
	r_core_block_read(core, 0);
	return ret;
}

int r_core_block_read(struct r_core_t *core, int next)
{
	r_io_lseek(core->file->fd, core->seek+(next)?core->blocksize:0, R_IO_SEEK_SET);
	return r_io_read(core->file->fd, core->block, core->blocksize);
}

int r_core_seek(struct r_core_t *core, u64 addr)
{
	core->seek = addr;
	r_io_lseek(core->file->fd, addr, R_IO_SEEK_SET);
	return r_core_block_read(core, 0);
}
