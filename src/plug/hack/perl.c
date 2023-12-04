/*
 * Copyright (C) 2007, 2008, 2009
 *       pancake <@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* THIS PLUGIS IS BROKEN! */
	#if 0
	case LANG_PERL:
		ptr = strdup(input);
		cmd[2] = ptr;
		eperl_init();
		perl_parse(my_perl, xs_init, 3, cmd, (char **)NULL);
		perl_run(my_perl);
		eperl_destroy();
		free(ptr);
		break;
	#endif

#include "plugin.h"
#undef _GNU_SOURCE
//#include "main.h"
#include <utils.h>
#undef _GNU_SOURCE
//#include <radare.h>
#include <EXTERN.h>
#include <XSUB.h>
#undef PL_madskills
#undef PL_xmlfp
#undef U32_MAX
#undef u8
#include <perl.h>

extern PerlInterpreter *my_perl;
extern void xs_init (pTHX);
static char *(*rs)(const char *cmd);
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
int perl_hack_cmd(const char *input);
void eperl_init();
void eperl_destroy();

PerlInterpreter *my_perl = NULL;

void radare_perl(pTHX_ CV* cv)
{
	FILE *fd;
	char *cmd;
	char file[64];
	char buf[1024];
	dXSARGS;

	char *str;
	cmd = sv_pv(ST(0));
	str = rs(cmd); //pipe_command_to_string(cmd);
	ST(0) = newSVpvn(str, strlen(str));
	free(str);
	XSRETURN(1);
#if 0
	if (!config.debug) {
		char *str;
		cmd = sv_pv(ST(0));
		str = pipe_command_to_string(cmd);
		ST(0) = newSVpvn(str, strlen(str));
		free(str);
		XSRETURN(1);
	} else {
		char str[4096];
		// XXX pipe_stdout_to_tmp_file is a br0ken idea
		str[0]='\0';
		cmd = sv_pv(ST(0));
		if (! pipe_stdout_to_tmp_file(file, cmd) ) {
			ST(0) = newSVpvn("", 0);
			return;
		}
		fd = fopen(file, "r");
		if (fd == NULL) {
			fprintf(stderr, "Cannot open tmpfile\n");
			unlink(file);
			ST(0) = newSVpvn("", 0);
			return;
		} else {
			while(!feof(fd)) {
				fgets(buf, 1023, fd);
				if (feof(fd)) break;
				if (strlen(buf)+strlen(str)> 4000) {
					fprintf(stderr, "Input line too large\n");
					break;
				}
				strcat(str, buf);
			}
			fclose(fd);
		}
		unlink(file);
	}
#endif
}

void xs_init(pTHX)
{
	newXS("r", radare_perl, __FILE__);
}

void eperl_init()
{
	my_perl = perl_alloc();
	perl_construct(my_perl);
}

void eperl_destroy()
{
	perl_destruct(my_perl);
	perl_free(my_perl);
	my_perl = NULL;
}

/* TODO: handle multi-line */
int perl_hack_cmd(const char *input)
{
	STRLEN n_a;
	char str[1025];
	char *ptr;
	char *ptrarr[3];
	static int perl_is_init=0;
	rs = radare_plugin.resolve("radare_cmd_str");

	if (rs==NULL) {
		printf("Cannot resolve radare_cmd_str symbol\n");
		return 0;
	}

	// Do not init all the frickin time?
	eperl_init();

	if (my_perl == NULL) {
		printf("Cannot init perl module\n");
		return 0;
	}
	perl_is_init = 1;

	/* prepare array */
	{
		char *perl_embed[] = { "", "-e", "0" };
		perl_parse(my_perl, xs_init, 3, perl_embed, (char **)NULL);
	}

		eval_pv("$|=1;", TRUE);
	if (input && input[0]!='\0') {
		char *perl_embed[3] = { "", (char *)input, NULL };
		perl_parse(my_perl, xs_init, 2, perl_embed, (char **)NULL);
		//call_argv(input, G_DISCARD | G_NOARGS, args);
		perl_run(my_perl);
		//printf("RET=%d\n", eval_pv(input, TRUE));
		return 0;
	}
	while(1) {
		char *args[] = { NULL };
		printf("perl> ");
		fflush(stdout);
		fgets(str, 1023, stdin);
		if (feof(stdin))
			break;
		str[strlen(str)-1]='\0';
		if (!strcmp(str, "q"))
			break;
		eval_pv(str, FALSE);
		if(SvTRUE(ERRSV))
			fprintf(stderr, "perl eval error: %s\n", SvPV(ERRSV,n_a));
	}
	eperl_destroy();
	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "perl",
	.desc = "perl plugin",
	.callback = &perl_hack_cmd
};
