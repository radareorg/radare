/*
 * Copyright (C) 2008
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

#include "main.h"
#include "code.h"
#include "utils.h"
#include "print.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct list_head traces;
static unsigned int n_traces = 0;

// num of traces for an address
int trace_times(off_t addr)
{
	struct list_head *pos;
	list_for_each(pos, &traces) {
		struct trace_t *h= list_entry(pos, struct trace_t, list);
		if (h->addr == addr)
			return h->times;
	}
	return 0;
}

int trace_count(off_t addr)
{
	struct list_head *pos;
	list_for_each(pos, &traces) {
		struct trace_t *h= list_entry(pos, struct trace_t, list);
		if (h->addr == addr)
			return h->count;
	}
	return 0;
}

int trace_index(off_t addr)
{
	int idx = -1;
	struct list_head *pos;
	list_for_each(pos, &traces) {
		struct trace_t *h= list_entry(pos, struct trace_t, list);
		idx++;
		if (h->addr == addr)
			return idx;
	}
	return idx;
}

int trace_add(off_t addr)
{
	struct trace_t *t;
	struct list_head *pos;

	if (config_get("trace.dup")) {
		/* update times counter */
		list_for_each(pos, &traces) {
			t = list_entry(pos, struct trace_t, list);
			if (t->addr == addr)
				++(t->times);
		}
	} else {
		list_for_each(pos, &traces) {
			t = list_entry(pos, struct trace_t, list);
			if (t->addr == addr) {
				t->count = ++n_traces;
				gettimeofday(&(t->tm), NULL);
				return ++(t->times);
			}
		}
	}

	t = (struct trace_t *)malloc(sizeof(struct trace_t));
	t->addr = addr;
	t->times = 1;
	gettimeofday(&(t->tm), NULL);
	t->count = ++n_traces;
	list_add_tail(&(t->list), &traces);

eprintf("new trace (0x%08x)\n", (unsigned long)addr);
	return t->times;
}

void trace_init()
{
	INIT_LIST_HEAD(&traces);
}
