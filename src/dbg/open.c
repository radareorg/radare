/*
 * Copyright (C) 2007-2009
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
 *
 * radare is part of the radare project
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "debug.h"
#include "../main.h"

void ps_construct_argv()
{
	int i = 0, idx, lidx=0;
	const char *ctmp;
	char *tmp, *tmp2, *eq;
	char buf[4096];

	free(ps.args);
	tmp2 = ps.args = strdup(ps.filename);

	/* XXX memory leaks everywhere!!1 */
	ctmp = config_get("file.dbg_arg");
	if (ctmp && *ctmp) {
		/* load program arguments from file */
		FILE *fd = fopen(ctmp, "r");
		while(!feof(fd)) {
			fgets(buf, 4095, fd);
			if (feof(fd))
				break;
			if (!buf[0]) continue;
			/* chop tail \n */
			buf[strlen(buf)-1]=0;
			/* ignore lines not starting with 'arg' */
			if (memcmp(buf, "arg", 3))
				continue;
			idx = atoi(buf+3);
			if (idx<0 || idx>254) {
				eprintf("arg%d: out of range\n", idx);
				continue;
			}
			eq = strchr(buf, '=');
			if (eq) {
				*eq = 0;
				if (eq[1]=='@') {
					char *contents = slurp(eq+2, NULL);
					ps.argv[idx] = contents;
				} else ps.argv[idx] = strdup(eq+1);
				if (idx == 0)
					ps.args = ps.argv[0];
			}
			if (idx>lidx)
				lidx = idx;
			ps.argv[lidx+1] = 0;
		}
		fclose(fd);
		return;
	}

	// parse argv
	//eprintf("commandline=\"%s\"\n", ps.args);
	for(tmp=ps.args;tmp[0];tmp=tmp+1) {
		if (tmp[0]==' '&&tmp!=ps.args) {
			if ((tmp[-1]=='\\') || (tmp[-1]=='/'))
				continue;
			tmp[0]='\0';
			ps.argv[i] = tmp2;
			tmp2 = tmp+1;
			if (++i>253) {
				printf("Too many arguments. truncated\n");
				break;
			}
		}
	}
	ps.argv[i] = tmp2;
	ps.argv[i+1] = 0;

	tmp = strchr(config.file, ' ');
	if (tmp) *tmp = '\0';
	//config.file = strdup("/bin/ls"); //ps.argv[0];
	//eprintf("ppa:A0(%s)\n", ps.argv[0]);
}

int debug_open(const char *pathname, int flags, mode_t mode)
{
	int flag = 0;
	char *file = (char *)pathname;

	if (file == NULL)
		return -1;

	/* TODO: advanced URIs for debugger :? */
	if (!strncmp("pid://", file, 6)) {
		file = file + 6;
		flag = 1;
	} else
	if ( (!strncmp("load://", file, 7))
	  || (!strncmp("dbg://", file, 6))) {
		file = file + ((file[0]=='d')?6:7);
		flag = 2;
	}

	config.debug = 1;

	// kill previous child here!
	if (flag && ps.opened>0) {
		eprintf("Process %d closed.", ps.pid);
		debug_close(ps.fd);
	}

	ps.filename = estrdup(ps.filename, resolve_path(file));
	config.file = (char *)malloc(strlen(ps.filename)+8);
	sprintf(config.file, "dbg://%s", ps.filename);

	ps_construct_argv();
	ps.is_file  = (atoi(ps.filename)?0:1);
	ps.opened = 1;

	if (debug_load() == -1 || ps.pid == 0)
		return -1;
	/* get entry point here */
	ps.ldentry    = arch_pc(ps.tid);
	ps.entrypoint = arch_get_entrypoint();

	/* initialize breakpoint list */
	ps.bps_n    =  0;
	memset(ps.bps, 0, sizeof(struct bp_t) * MAX_BPS);

	/* wait state */
	WS(event)   = UNKNOWN_EVENT;

	D {	if (ps.is_file)
			eprintf("Program '%s' loaded.\n", ps.filename);
		else eprintf("Attached to pid '%d'.\n", atoi(ps.filename));
	}

	eprintf("PID = %d\n", ps.pid);

#if __linux__
	system("if [ \"`cat /proc/sys/kernel/randomize_va_space`\" = 1 ]; then"
		" echo Warning: sysctl -w kernel.randomize_va_space=0; fi");
#endif
	return ps.fd;
}
