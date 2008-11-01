/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
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

#include "libps2fd.h"
#if __UNIX__
#include <sys/wait.h>
#include <signal.h>
#endif
#include <stdio.h>
#include "signal.h"

extern struct sig signals[];

void print_sigah()
{
	int i;
	for(i=0;signals[i].name;i++)
		debug_print_sigh(signals[i].name, \
			   (unsigned long)arch_get_sighandler(signals[i].sig));
}

int name_to_sig(char *signame)
{
	int i;
	for(i=0;signals[i].name;i++)
		if (!strcasecmp(signame, signals[i].name))
			return signals[i].sig;
	for(i=0;signals[i].name;i++)
		if (!strstr(signame, signals[i].name)) // XXX strcasestr!! not for w32
			return signals[i].sig;
	return get_offset(signame);
	//return -1;
}

const char *sig_to_name(int sig)
{
	int i;
	for(i=0;signals[i].name;i++)
		if (sig == signals[i].sig)
			return signals[i].name;
	return nullstr;
}

char *sig_to_desc(int sig)
{
	int i;
	for(i=0;signals[i].name;i++)
		if (sig == signals[i].sig)
			return signals[i].string;
	return NULL;
}

int debug_signal(const char *args)
{
	int signum;
	const char *signame;
	char *arg;
	u64 address;

	if (!ps.opened) {
		eprintf(":signal No program loaded.\n");
		return 1;
	}

	if (!args||args[0]=='\0') {
		print_sigah();
		return 0;
	}

	if(strchr(args,'?')) {
		cons_printf("Usage: !signal <SIGNUM> <HANDLER-ADDR>\n");
		cons_printf(" HANDLER=0 means ignore signal\n");
		return 0;
	}
	signame = args + 1;
	if ((arg = strchr(signame, ' '))) {
		arg[0]='\0'; arg=arg+1;
		signum = name_to_sig(signame);
		address = get_math(arg);
		//signal_set(signum, address);
		arch_set_sighandler(signum, address);
	} else {
		signum = name_to_sig(signame);	

		if (signum == -1) {
			eprintf(":signal Invalid signal name %s.\n", signame);
			return 1;
		}

		debug_print_sigh(signame, (unsigned long)arch_get_sighandler(signum));
	}

	return 0;
}
