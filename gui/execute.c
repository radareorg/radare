/*
 * Copyright (C) 2007
 *       pancake <pancake@phreaker.net>
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
#include <string.h>

/*
 * TODO: use pthreads and mutexify the gtk main loop here
 */

int execute_command(char *cmd)
{
	FILE *fd;
	char buffer[1024];

	fd = popen(cmd, "r");
	if (!fd) {
	perror("popen");
		fprintf(stderr, "Cannot execute %s\n", cmd);
		return 0;
	}
	while(!feof(fd)) {
		fgets(buffer, 1024, fd);
		if (feof(fd)) break;
		vte_terminal_feed_child(VTE_TERMINAL(term), buffer, strlen(buffer));
	}

	fclose(fd);
	return 1;
}
