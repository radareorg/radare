/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
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

#include "main.h"
#include "radare.h"
#include "utils.h"
#include "eperl.h"

#if HAVE_PERL
PerlInterpreter *my_perl = NULL;

void radare_perl(pTHX_ CV* cv)
{
	FILE *fd;
	char *cmd;
	char file[64];
	char buf[1024];
	dXSARGS;

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
		str[strlen(str)-1]='\0';

		ST(0) = newSVpvn(str, strlen(str));

		XSRETURN(1);
	}
}

void xs_init(pTHX)
{
	newXS("r", radare_perl, __FILE__);
}
#endif

void eperl_init()
{
#if HAVE_PERL
	my_perl = perl_alloc();
	perl_construct(my_perl);
#endif
}

void eperl_destroy()
{
#if HAVE_PERL
	perl_destruct(my_perl);
	perl_free(my_perl);
	my_perl = NULL;
#endif
}
