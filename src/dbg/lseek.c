/*
 * Copyright (C) 2007
 *       pancake <@youterm.com>
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
#include <unistd.h>
#include <sys/types.h>

off_t debug_lseek(int fildes, off_t offset, int whence)
{
	if (ps.opened && ps.fd == fildes)
		switch(whence) {
		case SEEK_SET:
			if (offset!=0)
				ps.offset = offset;
			return ps.offset;
		case SEEK_CUR:
			ps.offset = (off_t)((unsigned long long)ps.offset+(unsigned long long)offset);
			return ps.offset;
		case SEEK_END:
			return ps.offset = (off_t)((unsigned long long)(-1));
		default:
			return (off_t)(unsigned long long)-1;
		}

	return __lseek(fildes, offset, whence);
}
