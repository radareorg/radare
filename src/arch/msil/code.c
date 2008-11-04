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

/* code analysis functions */

#include "../../main.h"
#include "../../code.h"
#include <stdio.h>
#include <string.h>
#include "demsil.h"

// NOTE: bytes should be at least 16 bytes?
int arch_msil_aop(u64 addr, const u8 *bytes, struct aop_t *aop)
{
	int n;

	DISASMSIL_OFFSET CodeBase = addr;
	ILOPCODE_STRUCT ilopar[8]; // XXX only uses 1
	DisasMSIL(bytes, 16,CodeBase, ilopar, 8, &n);

	if (aop == NULL)
		return ilopar[0].Size;

	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;
	aop->length = ilopar[0].Size;

	return ilopar[0].Size;
}
