/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"
#include "r_flags.h"
#include "r_asm.h"

static int cmd_quit(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '?':
		fprintf(stderr,
		"Usage: q[!] [retvalue]\n"
		" q     ; quit program\n"
		" q!    ; force quit (no questions)\n"
		" q 1   ; quit with return value 1\n"
		" q a-b ; quit with return value a-b\n");
		break;
	case '\0':
	case ' ':
	case '!':
	default:
		exit(r_num_math(&core->num, input+1));
		break;
	}
	return 0;
}

static int cmd_seek(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;

	if (input[0]=='\0') {
		r_cons_printf("0x%llx\n", core->seek);
	} else {
		u64 off = r_num_math(&core->num, input+1);
		switch(input[0]) {
		case ' ':
			r_core_seek(core, off);
			break;
		case '+':
			r_core_seek(core, core->seek + off);
			break;
		case '-':
			r_core_seek(core, core->seek - off);
			break;
		case '?':
			fprintf(stderr,
			"Usage: s[+-] [addr]\n"
			" s 0x320   ; seek to this address\n"
			" s+ 512    ; seek 512 bytes forward\n"
			" s- 512    ; seek 512 bytes backward\n");
			break;
		}
	}
	return 0;
}

static int cmd_help(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	u64 n;

	switch(input[0]) {
	case ' ':
		n = r_num_math(&core->num, input+1);
		r_cons_printf("%lld 0x%llx\n", n,n);
		break;
	case '=':
		r_num_math(&core->num, input+1);
		break;
	case '+':
		if (input[1]) {
			if (core->num.value & U64_GT0)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '-':
		if (input[1]) {
			if (core->num.value & U64_LT0)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '!': // ??
		if (input[1]) {
			if (core->num.value != U64_MIN)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '?': // ??
		if (input[1]=='?') {
			fprintf(stderr,
			"Usage: ?[?[?]] expression\n"
			" ? eip-0x804800  ; calculate result for this math expr\n"
			" ?= eip-0x804800  ; same as above without user feedback\n"
			" ?? [cmd]    ; ? == 0  run command when math matches\n"
			" ?! [cmd]    ; ? != 0\n"
			" ?+ [cmd]    ; ? > 0\n"
			" ?- [cmd]    ; ? < 0\n"
			" ???             ; show this help\n");
		} else
		if (input[1]) {
			if (core->num.value == U64_MIN)
				r_core_cmd(core, input+1, 0);
		} else r_cons_printf("0x%llx\n", core->num.value);
		break;
	case '\0':
	default:
		fprintf(stderr,
		"Usage:\n"
		" b [bsz]   ; get or change block size\n"
		" e [a[=b]] ; list/get/set config evaluable vars\n"
		" f [name]  ; set flag at current address\n"
		" s [addr]  ; seek to address\n"
		" i [file]  ; get info about opened file\n"
		" p?[len]   ; print current block with format and length\n"
		" x [len]   ; alias for 'px' (print hexadecimal\n"
		" V         ; enter visual mode\n"
		" ? [expr]  ; evaluate math expression\n"
		" q [ret]   ; quit radare\n"
		"Append '?' to every char command to get detailed help\n"
		"");
		break;
	}
	return 0;
}

static int cmd_bsize(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '\0':
		r_cons_printf("0x%x\n", core->blocksize);
		break;
	default:
		//input = r_str_clean(input);
		r_core_block_size(core, r_num_math(NULL, input));
		break;
	}
	return 0;
}

static int cmd_hexdump(void *data, const char *input)
{
	return cmd_print(data, input-1);
}

static int cmd_info(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '?':
		break;
	case '*':
		break;
	default:
		r_cons_printf("URI: %s\n", core->file->uri);
		r_cons_printf("blocksize: 0x%x\n", core->blocksize);
	}
	return 0;
}

static int cmd_print(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	int l, len = core->blocksize;
	u32 tbs = core->blocksize;

	if (input[0] && input[1]) {
		l = (int) r_num_get(&core->num, input+2);
		if (l>0) len = l;
		if (l>tbs) {
			r_core_block_size(core, l);
			len = l;
		}
	}
	
	switch(input[0]) {
	case 'd':
		{
			/* XXX hardcoded */
			int ret, idx = 0; 
			char *buf = core->block;
			struct r_asm_t a;// TODO: move to core.h
			r_asm_init(&a);
			r_asm_set_pc(&a, core->seek);
			
			for(idx=0; idx < len; idx+=ret) {
				ret = r_asm_disasm_buf(&a, buf+idx, len-idx);
				r_cons_printf("0x%08llx  %12s  %s\n", core->seek+idx, a.buf_hex, a.buf_asm);
			}
		}
		break;
	case 'x':
        	r_print_hexdump(core->seek, core->block, len, 1, 78, !(input[1]=='-'));
		break;
	case '8':
		r_print_bytes(core->block, len, "%02x");
		break;
	default:
		fprintf(stderr, "Usage: p[8] [len]"
		" p8 [len]    8bit hexpair list of bytes\n"
		" px [len]    hexdump of N bytes\n"
		" pd [len]    disassemble N bytes\n");
		break;
	}
	if (tbs != core->blocksize)
		r_core_block_size(core, tbs);
	return 0;
}

static int cmd_flag(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '+':
		r_flag_set(&core->flags, input+1, core->seek, core->blocksize, 1);
		break;
	case ' ':
		r_flag_set(&core->flags, input+1, core->seek, core->blocksize, 0);
		break;
	case '-':
		r_flag_unset(&core->flags, input+1);
		break;
	case 's':
		/* spaces */
		break;
	case '*':
		r_flag_list(&core->flags, 1);
		break;
	case '\0':
		r_flag_list(&core->flags, 0);
		break;
	case '?':
		fprintf(stderr, "Usage: f[ ] [flagname]\n"
		" f name 12 @ 33   ; set flag 'name' with size 12 at 33\n"
		" f+name 12 @ 33   ; like above but creates new one if doesnt exist\n"
		" f-name           ; remove flag 'name'\n"
		" f                ; list flags\n"
		" f*               ; list flags in radare commands\n");
		break;
	}
	return 0;
}

static int cmd_eval(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	switch(input[0]) {
	case '\0':
		r_config_list(&core->config, NULL, 0);
		break;
	case '-':
		r_config_init(&core->config);
		break;
	case '*':
		r_config_list(&core->config, NULL, 1);
		break;
	case '?':
		fprintf(stderr,
		"Usage: e[?] [var[=value]]\n"
		"  e     ; list config vars\n"
		"  e-    ; reset config vars\n"
		"  e*    ; dump config vars in radare commands\n"
		"  e a   ; get value of var 'a'\n"
		"  e a=b ; set var 'a' the 'b' value\n");
		//r_cmd_help(&core->cmd, "e");
		break;
	default:
		r_config_eval(&core->config, input);
	}
	return 0;
}

static int cmd_hash(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	fprintf(stderr, "TODO\n");
	return 0;
}

static int cmd_visual(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	r_core_visual(core);
	return 0;
}

static int cmd_system(void *data, const char *input)
{
	struct r_core_t *core = (struct r_core_t *)data;
	return r_io_system(core->file->fd, input);
}

int r_core_cmd_subst(struct r_core_t *core, char *cmd, int *rs, int *times)
{
	char *ptr, *ptr2, *str;
	int i, len = strlen(cmd);

	len = atoi(cmd);
	if (len>0) {
		for(i=0;cmd[i]>='0'&&cmd[i]<='9'; i++);
		if (i>0) strcpy(cmd, cmd+i);
		*times = len;
	}

	/* quoted / raw command */
	if (cmd[0]=='"') {
		if (cmd[len-1]!='"') {
			fprintf(stderr, "parse: Missing ending '\"': '%s'\n", cmd);
			return 0;
		}
		return 0;
	}
	ptr = strchr(cmd, ';');
	if (ptr)
		ptr[0]='\0';
	ptr = strchr(cmd, '@');
	if (ptr) {
		ptr[0]='\0';
		*rs = 1;
		if (ptr[1]=='@') {
			fprintf(stderr, "TODO: foreach @ loop\n");
		}
		r_core_seek(core, r_num_math(&core->num, ptr+1));
	}
	ptr = strchr(cmd, '~');
	if (ptr) {
		ptr[0]='\0';
		r_cons_grep(ptr+1);
	} else r_cons_grep(NULL);
	while(ptr = strchr(cmd, '`')) {
		ptr2 = strchr(ptr+1, '`');
		if (ptr2==NULL) {
			fprintf(stderr, "parse: Missing '`' in expression.");
			return 0;
		}
		ptr2[0]='\0';
		str = r_core_cmd_str(core, ptr+1);
		for(i=0;str[i];i++) if (str[i]=='\n') str[i]=' ';
		r_str_inject(ptr, ptr2+1, str, 1024);
		free(str);
	}
	return 0;
}

int r_core_cmd(struct r_core_t *core, const char *command, int log)
{
	int i, len;
	char *cmd;
	int ret = -1;
	int times = 1;
	
	u64 tmpseek = core->seek;
	int restoreseek = 0;

	if (command == NULL ) {
		cmd = (char *)command;
	} else {
		len = strlen(command)+1;
		cmd = alloca(len)+1024;
		memcpy(cmd, command, len);

		ret = r_core_cmd_subst(core, cmd, &restoreseek, &times);
		if (ret == -1) {
			fprintf(stderr, "Invalid conversion: %s\n", command);
			return -1;
		}
	}
	
	for(i=0;i<times;i++) {
		ret = r_cmd_call(&core->cmd, cmd);
		if (ret == -1) // stop on error?
			break;
	}
	if (ret == -1){
		if (command[0])
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

int r_core_cmd_init(struct r_core_t *core)
{
	r_cmd_init(&core->cmd);
	r_cmd_set_data(&core->cmd, core);
	r_cmd_add(&core->cmd, "x",     "alias for px", &cmd_hexdump);
	r_cmd_add(&core->cmd, "flag",  "get/set flags", &cmd_flag);
	r_cmd_add(&core->cmd, "info",  "get file info", &cmd_info);
	r_cmd_add(&core->cmd, "seek",  "seek to an offset", &cmd_seek);
	r_cmd_add(&core->cmd, "bsize", "change block size", &cmd_bsize);
	r_cmd_add(&core->cmd, "eval",  "evaluate configuration variable", &cmd_eval);
	r_cmd_add(&core->cmd, "print", "print current block", &cmd_print);
	r_cmd_add(&core->cmd, "Visual","enter visual mode", &cmd_visual);
	r_cmd_add(&core->cmd, "!",     "run system command", &cmd_system);
	r_cmd_add(&core->cmd, "#",     "calculate hash", &cmd_hash);
	r_cmd_add(&core->cmd, "?",     "help message", &cmd_help);
	r_cmd_add(&core->cmd, "quit",  "exit program session", &cmd_quit);

	return 0;
}
