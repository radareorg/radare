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

#define USE_OLLY 1
#if USE_OLLY
#define STRICT
#define MAINPROG


#if RADARE_CORE
#include "arch/x86/ollyasm/disasm.h"
#else
#include "../arch/x86/ollyasm/disasm.h"
#endif
#endif

int rasm_olly_x86(u64 offset, const char *str, u8 *data)
{
	char errtext[TEXTLEN];
	t_asmmodel am;
	int i,j;
	// am.code = data ?
	j = Assemble(str,(unsigned long)offset,&am,0,4,errtext);
	memcpy(data, am.code,j);
	return j;
}
