/*
 * Copyright (C) 2007, 2008
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

#include "plugin.h"
#include "main.h"
#include <radare.h>
#include <utils.h>
#include <EXTERN.h>
#include <XSUB.h>
#include <perl.h>

extern PerlInterpreter *my_perl;
extern void xs_init (pTHX);
static int (*r)(char *cmd, int log);
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
void perl_cmd(char *input);
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
	char str[1024];
	strcpy(str, "error");
		str[strlen(str)-1]='\0';

		ST(0) = newSVpvn(str, strlen(str));

		XSRETURN(1);
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

void perl_cmd(char *input)
{

	r = radare_plugin.resolve("radare_cmd");

	eperl_init();
 //               ptr = strdup(input);
//                cmd[2] = ptr;
                eperl_init();
                perl_parse(my_perl, xs_init, 3, input, (char **)NULL);
                perl_run(my_perl);
                eperl_destroy();
 //               free(ptr);


#if 0
	if (r != NULL) {
		r("b 20", 0);
		r("x", 0);
	} else	printf("Cannot resolve 'radare_cmd' symbol\n");
#endif
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "perl",
	.desc = "perl plugin",
	.callback = &perl_cmd
};

main() { printf("i'm a plugin!\n"); }
