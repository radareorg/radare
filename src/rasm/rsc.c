/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
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

#include "rasm.h"

int rasm_rsc(ut64 offset, const char *str, unsigned char *data)
{
	int i;
	FILE *fd;
	char buf[1024];
	// XXX hacky solution
	sprintf(buf, "SYNTAX=intel rsc asm '%s' > /tmp/.rscasm", str);
	system(buf);
	fd = fopen("/tmp/.rscasm", "r");
	for(i=0;1;i++) {
		int b;
		fscanf(fd, "%02x", &b);
		if (feof(fd)) break;
		data[i]=(unsigned char)b;
	}
	fclose(fd);
	unlink(fd);
	return i;
}
