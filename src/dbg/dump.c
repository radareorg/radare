/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
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
#include "arch/arch.h"
#include "../radare.h"
#include "../config.h"
#include "../print.h"
#include "../code.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#if __linux__
#include <sys/prctl.h>
#include <linux/ptrace.h>
#endif
#include <sys/stat.h>
#include "wp.h"
#include "mem.h"
#include "thread.h"
#include "signals.h"
#include "events.h"
#include "debug.h"

static int dump_num = 0;
static char dumpdir[128];

void debug_dumpcore()
{
#if __NetBSD__
	ptrace(PT_DUMPCORE, ps.tid, NULL, 0);
#else
	eprintf("Not supported for this platform\n");
#endif
}

/* hacky util for dumping pages without knowing anything about map pages */
int debug_dumpall(const char *ptr)
{
	char file[128];
	FILE *fd = NULL;
	int ret,i=0;
	char buf[4096];
	ut64 from=config.seek;
	ut64 to = config.limit;
	if (to <= 0)
		to = 0xffffffff;
	from &= 0xfffffff0; // align hack
	cons_printf("Dumping from 0x%08llx to 0x%08llx...\n", from, to);

	radare_controlc();
	while(!config.interrupted) {
		ret = debug_read_at(ps.pid, buf, 4096, from);
		from += 4096;
		if (0==(i++%30)) printf("0x%08llx: %d    \r", from, ret);
		if (ret<0) {
			if (fd != NULL) {
				fclose(fd);
				fd = NULL;
			}
			continue;
		}
		if (fd == NULL) {
			sprintf(file, "0x%08llx.dall", from);
			printf("\nNew section found at 0x%08llx\n", from);
			i = 0;
			fd = fopen(file, "wb");
			if (fd == NULL) {
				eprintf("Cannot create '%s'\n", file);
				break;
			}
		}
		fwrite(buf, ret, 1, fd);
	}
	if (config.interrupted)
		eprintf("Dump interrupted at 0x%08llx\n", from);
	if (fd != NULL)
		fclose(fd);
	radare_controlc_end();
	return 0;
}

int process_dump(const char *dir)
{
	int regs = PTRCAST (config_get("dump.regs"));
  	int user = PTRCAST (config_get("dump.user"));
  	int libs = PTRCAST (config_get("dump.libs"));
  	int fds  = PTRCAST (config_get("dump.fds"));

	if (strnull(dir)) {
		sprintf(dumpdir, "dump%d", dump_num);
		dir = dumpdir;
		dump_num++;
	} else
	if (dir&&*dir) {
		dir = dir + 1;
		if (strchr(dir,'/')) {
			eprintf("No '/' permitted here.\n");
			return;
		}
	}
#if __WINDOWS__ && !__CYGWIN__
	mkdir(dir);
#else
	mkdir(dir, 0755);
#endif
	if ( chdir(dir) == -1) {
		eprintf("No '/' permitted here.\n");
		return;
	}
	printf("Dump directory: %s\n", dir);

	/* --- */
	if (user||libs)
		page_dumper();
	if (regs)
		arch_dump_registers();
	if (fds)
		debug_fd_dump();
	/* --- */

	if (dir&&*dir)
		chdir("..");

	return 0;
}

int process_restore(const char *dir)
{
	int regs = PTRCAST (config_get("dump.regs"));
  	int user = PTRCAST (config_get("dump.user"));
  	int libs = PTRCAST (config_get("dump.libs"));
  	int fds  = PTRCAST (config_get("dump.fds"));

	if (strnull(dir)) {
		dump_num--;
		if (dump_num<0) {
			eprintf("No dumps to restore from. Sorry\n");
			return;
		}
		sprintf(dumpdir, "dump%d", dump_num);
		dir = dumpdir;
	} else
	if (dir&&*dir) {
		dir = dir + 1;
		if (strchr(dir,'/')) {
			eprintf("No '/' permitted here.\n");
			return;
		}
	}
	if ( chdir(dir) == -1 ) {
		eprintf("Cannot chdir to '%s'\n", dir);
		return;
	}

	printf("Restore directory: %s\n", dir);

	/* --- */
	if (user||libs)
		page_restore();
	if (regs)
		arch_restore_registers();
	if (fds)
		debug_fd_dump();
	/* --- */

	if (dir&&*dir)
		chdir("..");

	return 0;
}
