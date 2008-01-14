/*
 * Copyright (C) 2007
 *       pancake <@youterm.com>
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
#include "radare.h"

#define UNDOS 64
static off_t undos[UNDOS];
static int undos_idx = 0;
static int undos_lim = 0;

void undo_seek()
{
	if (--undos_idx<0)
		undos_idx = 0;
	else
		config.seek = undos[undos_idx-1];
}

void undo_redo()
{
	if (undos_idx<undos_lim) {
		undos_idx+=2;
		undo_seek();
	}
}

void undo_push()
{
	int i;

	if (undos[undos_idx-1] == config.seek)
		return;

	undos[undos_idx] = config.seek;
	if (undos_idx==UNDOS-1) {
		for(i=1;i<UNDOS;i++)
			undos[i-1] = undos[i];
	} else
		undos_idx++;

	if (undos_lim<undos_idx)
		undos_lim = undos_idx;
}

void undo_reset()
{
	undos_idx = 0;
}

void undo_list()
{
	int i;
	if (undos_idx<1)
		printf("-no seeks yet-\n");
	else
	for(i=undos_idx-1;i!=0;i--) {
		printf(OFF_FMT" ; %lld", undos[i-1], undos[i-1]);
		NEWLINE;
	}
}
