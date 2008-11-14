/*
 * Copyright (C) 2007, 2008
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

#include "r_cons.h"
#include "r_print.h"

static int flags =
	R_PRINT_FLAGS_COLOR |
	R_PRINT_FLAGS_ADDRMOD;

void r_print_set_flags(int _flags)
{
	flags = _flags;
}

void r_print_addr(u64 addr)
{
	//config_get_i("cfg.addrmod");
	int mod = flags & R_PRINT_FLAGS_ADDRMOD;
	char ch = (0==(addr%(mod?mod:1)))?',':' ';

	if (flags & R_PRINT_FLAGS_COLOR) {
		cons_printf("%s0x%08llx"C_RESET"%c ",
			cons_palette[PAL_ADDRESS], addr, ch);
	} else {
		cons_printf(OFF_FMT"%c ", addr, ch);
	}
}
