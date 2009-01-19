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
	struct r_core_t *core = (struct r_core_t *)data;
	if (input[0]==' ') {
		r_cons_printf("0x%08llx\n", r_num_get(&core->num, input+1));
	} else
		r_cons_printf("No help implemented yet. sorry :)\n");
	/* XXX */
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

static int cmd_print(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	int l, len = core->blocksize;
	u32 tbs = core->blocksize;

	if (input[0]) {
		l = (int) r_num_get(&core->num, input+1);
		if (l>0) len = l;
		if (l>tbs) r_core_block_size(core, l);
	}
	
	switch(input[0]) {
	case 'x':
        	r_print_hexdump(core->seek, core->block, len, 1, 78, 1);
		break;
	case '8':
		r_print_bytes(core->block, len, "%02x");
		break;
	default:
		fprintf(stderr, "Usage: p[8] [len]"
		" p8 [len]    8bit hexpair list of bytes\n");
		break;
	}
	if (tbs != core->blocksize)
		r_core_block_size(core, tbs);
	return 0;
}

static int cmd_flag(void *data, const char *input)
{
	//struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case ' ':
		//r_flag_add(core->flags);
		break;
	case '-':
		break;
	case '?':
		fprintf(stderr, "Usage: f[ ] [flagname]\n");
		break;
	}
	return 0;
}

static int cmd_visual(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	r_core_visual(core);
	return 0;
}

int r_core_cmd_init(struct r_core_t *core)
{
	r_cmd_init(&core->cmd);
	r_cmd_set_data(&core->cmd, core);
	r_cmd_add(&core->cmd, "x",    "alias for px", &cmd_hexdump);
	r_cmd_add(&core->cmd, "flag", "get/set flags", &cmd_flag);
	r_cmd_add(&core->cmd, "info", "get file info", &cmd_info);
	r_cmd_add(&core->cmd, "seek", "seek to an offset", &cmd_seek);
	r_cmd_add(&core->cmd, "bsize", "change block size", &cmd_bsize);
	r_cmd_add(&core->cmd, "print","print current block", &cmd_print);
	r_cmd_add(&core->cmd, "Visual","enter visual mode", &cmd_visual);
	r_cmd_add(&core->cmd, "?",    "help message", &cmd_help);
	r_cmd_add(&core->cmd, "quit", "exit program session", &cmd_quit);
	return 0;
}

int r_core_cmd_subst(struct r_core_t *core, char *cmd, int *rs)
{
	char *ptr;
	int len = strlen(cmd);

	/* quoted / raw command */
	if (cmd[0]=='"') {
		if (cmd[len-1]!='"') {
			fprintf(stderr, "Missing ending '\"': '%s'\n", cmd);
			return -1;
		}
		return 0;
	}
	ptr = strchr(cmd, '@');
	if (ptr) {
		ptr[0]='\0';
		*rs = 1;
		r_core_seek(core, r_num_math(&core->num, ptr+1));
	}
	return 0;
}

int r_core_cmd(struct r_core_t *core, const char *command, int log)
{
	int len;
	char *cmd;
	int ret = -1;
	
	u64 tmpseek = core->seek;
	int restoreseek = 0;

	if (command == NULL ) {
		cmd = (char *)command;
	} else {
		len = strlen(command)+1;
		cmd = alloca(len)+1024;
		memcpy(cmd, command, len);

		ret = r_core_cmd_subst(core, cmd, &restoreseek);
		if (ret == -1) {
			fprintf(stderr, "Invalid conversion: %s\n", command);
			return -1;
		}
	}
	
	ret = r_cmd_call(&core->cmd, cmd);
	if (ret == -1) {
		fprintf(stderr, "Invalid command: %s\n", command);
		ret = 1;
	} else
	if (log) r_line_hist_add(command);
	if (restoreseek)
		r_core_seek(core, tmpseek);

	return ret;
}

const char *r_core_cmd_str(struct r_core_t *core, const char *cmd)
{
	const char *retstr;
	r_cons_reset();
	if (r_core_cmd(core, cmd, 0) == -1) {
		fprintf(stderr, "Invalid command: %s\n", cmd);
		retstr = strdup("");
	} else {
		retstr = r_cons_get_buffer();
		if (retstr==NULL)
			retstr = strdup("");
		else retstr = strdup(retstr);
		r_cons_reset();
	}
	return retstr;
}
