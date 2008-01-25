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
		if (!strcmp(signame, signals[i].name))
			return signals[i].sig;
	return -1;
}

static char *nullstr="";
char *sig_to_name(int sig)
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
