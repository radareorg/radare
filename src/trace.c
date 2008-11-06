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

struct trace_t *trace_get(u64 addr)
{
	struct list_head *pos;
	list_for_each(pos, &traces) {
		struct trace_t *h= list_entry(pos, struct trace_t, list);
		if (h->addr == addr)
			return h;
	}
	return NULL;
}

// num of traces for an address
int trace_times(u64 addr)
{
	struct trace_t *t = trace_get(addr);
	return t?t->times:0;
}

int trace_count(u64 addr)
{
	struct trace_t *t = trace_get(addr);
	return t?t->count:0;
}

int trace_index(u64 addr)
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

int trace_set_times(u64 addr, int times)
{
	char bytes[16];
	struct trace_t *t;
	struct list_head *pos;

	if (arch_aop == NULL)
		return -1;
	/* update times counter */
	list_for_each(pos, &traces) {
		t = list_entry(pos, struct trace_t, list);
		if (t->addr == addr) {
			t->times = times;
			return 1;
		}
	}
	return 0;
}

int trace_add(u64 addr)
{
	char bytes[16];
	struct trace_t *t;
	struct list_head *pos;

	if (arch_aop == NULL)
		return -1;
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
	memset(t,'\0',sizeof(struct trace_t));
	t->addr = addr;
	t->times = 1;
	radare_read_at(addr, bytes, 16);
	t->opsize = arch_aop(addr, bytes, NULL);
	gettimeofday(&(t->tm), NULL);
	t->count = ++n_traces;
	list_add_tail(&(t->list), &traces);

	//eprintf("new trace (0x%08x)\n", (unsigned long)addr);
	return t->times;
}

u64 trace_range(u64 from)
{
	u64 last = from;
	u64 last2 = 0LL;
	struct trace_t *h;
	
	while(last != last2) {
		last2 = last;
		h = trace_get(last);
		if (h) last = last + h->opsize;
	}

	return last;
}

#if 1
u64 trace_next(u64 from)
{
        u64 next = 0xFFFFFFFFFFFFFFFFLL;
        struct list_head *pos;
        struct trace_t *h;

        list_for_each(pos, &traces) {
                h = list_entry(pos, struct trace_t, list);
                if (h->addr > from && h->addr < next)
                        next = h->addr;
        }

        if (next == 0xFFFFFFFFFFFFFFFFLL)
                return 0LL;
        return next;
}

#endif

#if 0
/* buggy version */
u64 trace_next(u64 from)
{
	u64 next;
	int next_init = 0;
	struct list_head *pos;
	struct trace_t *h;

	list_for_each(pos, &traces) {
		h = list_entry(pos, struct trace_t, list);
		if (!next_init) {
			if (h->addr > from && h->addr < next) {
				next = h->addr;
				next_init = 1;
			}
			continue;
		}
		if (h->addr > from && h->addr < next)
			next = h->addr;
	}

	if (next_init == 0)
		return NULL;

	return next;
}
#endif

void trace_show(int plain)
{
	u64 from = 0LL;
	u64 last;
	struct list_head *pos;
	struct trace_t *h;

	/* get the lower address */
	list_for_each(pos, &traces) {
		h = list_entry(pos, struct trace_t, list);
		if (from == 0LL)
			from = h->addr;
		if (h->addr < from)
			from = h->addr;
		if (plain)
			cons_printf("0x%08llx %d %d\n", h->addr, h->times, h->count);
	}
	if (plain)
		return;

	while(from) {
		last = trace_range(from);
		// TODO: show timestamps
		cons_printf("0x%08llx - 0x%08llx\n", from, last);
		from = trace_next(last);
		cons_flush();
	}
}

void trace_init()
{
	INIT_LIST_HEAD(&traces);
}

void trace_reset()
{
	struct list_head *pos;
	struct trace_t *h;
	list_for_each(pos, &traces) {
		h = list_entry(pos, struct trace_t, list);
		free(h);
	}
	INIT_LIST_HEAD(&traces);
}

int trace_get_between(u64 from, u64 to)
{
	int ctr = 0;
	struct list_head *pos;
	struct trace_t *h;

	/* get the lower address */
	list_for_each(pos, &traces) {
		h = list_entry(pos, struct trace_t, list);
		if (h->addr >= from && h->addr <=to)
			ctr++;
	}

	return ctr;
}
