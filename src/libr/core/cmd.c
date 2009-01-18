/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"

static int cmd_quit(void *data, const char *input)
{
	/* XXX */
	exit(1);
	return 0;
}

static int cmd_seek(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	u64 off = r_num_math(NULL, input);
	r_core_seek(core, off);
	return 0;
}

static int cmd_help(void *data, const char *input)
{
	/* XXX */
	r_cons_printf("No help implemented yet. sorry :)\n");
	return 0;
}

static int cmd_bsize(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	//input = r_str_clean(input);
	r_core_block_size(core, r_num_math(NULL, input));
	return 0;
}

static int cmd_hexdump(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
        r_print_hexdump(core->seek, core->block, core->blocksize, 1, 78, 1);
	return 0;
}

static int cmd_info(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	r_cons_printf("URI: %s\n", core->file->uri);
	r_cons_printf("blocksize: 0x%x\n", core->blocksize);
	return 0;
}

int r_core_cmd_init(struct r_core_t *core)
{
	r_cmd_init(&core->cmd);
	r_cmd_set_data(&core->cmd, core);
	r_cmd_add(&core->cmd, "x",    "alias for px", &cmd_hexdump);
	r_cmd_add(&core->cmd, "info", "get file info", &cmd_info);
	r_cmd_add(&core->cmd, "seek", "seek to an offset", &cmd_seek);
	r_cmd_add(&core->cmd, "bsize", "change block size", &cmd_bsize);
	r_cmd_add(&core->cmd, "print","print current block", &cmd_hexdump);
	r_cmd_add(&core->cmd, "?",    "help message", &cmd_help);
	r_cmd_add(&core->cmd, "quit", "exit program session", &cmd_quit);
}

int r_core_cmd(struct r_core_t *core, const char *cmd, int log)
{
	int ret;
	ret = r_cmd_call(&core->cmd, cmd);
	if (ret == -1) {
		fprintf(stderr, "Invalid command: %s\n", cmd);
		ret = 1;
	} else
	if (log) {
		//r_line_add()
	}

	return ret;
}
