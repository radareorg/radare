/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"
#include "r_flags.h"

static u64 num_callback(void *userptr, const char *str, int *ok)
{
	struct r_core_t *core = userptr;
	struct r_flag_item_t *flag;
	
	if (str[0]=='$') {
		switch(str[1]) {
		case '$': return core->seek;
		case 'b': return core->blocksize;
		//case '?': return core->blocksize; // HELP
		}
	}

	flag = r_flag_get(&(core->flags), str);
	if (flag != NULL) {
		*ok = 1;
		return flag->offset;
	}
	*ok = 0;
	return 0LL;
}

struct r_core_t *r_core_new()
{
	struct r_core_t *c = MALLOC_STRUCT(struct r_core_t);
	r_core_init(c);
	return c;
}

int r_core_init(struct r_core_t *core)
{
	core->num.callback = &num_callback;
	core->num.userptr = core;
	r_cons_init();
	r_macro_init(&core->macro);
	core->macro.num = &core->num;
	core->macro.user = core;
	core->macro.cmd = r_core_cmd0;
	r_io_init();
	core->file = NULL;
	INIT_LIST_HEAD(&core->files);
	core->seek = 0LL;
	core->blocksize = R_CORE_BLOCKSIZE;
	core->block = (u8*)malloc(R_CORE_BLOCKSIZE);
	r_core_cmd_init(core);
	r_core_config_init(core);
	r_flag_init(&core->flags);
	return 0;
}

struct r_core_t *r_core_free(struct r_core_t *c)
{
	free(c);
	return NULL;
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
	ret = r_core_cmd(r, line, R_TRUE);
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
	r_io_lseek(core->file->fd, core->seek+((next)?core->blocksize:0), R_IO_SEEK_SET);
	return r_io_read(core->file->fd, core->block, core->blocksize);
}

int r_core_seek(struct r_core_t *core, u64 addr)
{
	u64 tmp = core->seek;
	int ret;
	core->seek = addr;
	ret = r_core_block_read(core, 0);
	if (ret == -1)
		core->seek = tmp;
	return ret;
}
