/*
 * Copyright (C) 2007, 2008
 *       th0rpe <nopcode.org>
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
#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

void add_th(TH_INFO *th)
{
	list_add_tail(&(th->list), &(ps.th_list));
}

void del_th(TH_INFO *th)
{		
	list_del(&(th->list));
	free(th);
}

#if 0
int test_add_th(TH_INFO *th)
{
	if(get_th(th->tid) == NULL)  {
		add_th(th);
		return 1;
	}

	return 0;
}
#endif

int th_list()
{
	struct list_head *pos;
	int n = 0;

	list_for_each_prev(pos, &ps.th_list) {
		TH_INFO		*th = list_entry(pos, TH_INFO, list);
		printf(" %c %d: 0x%08lx state: 0x%x\n", (ps.th_active == th)?'*':' ', th->tid, th->addr, th->status);
		n++;
	}
	return n;
}

TH_INFO *init_th(pid_t tid, int status)
{
	TH_INFO *th;

	if(!(th = (TH_INFO *)malloc(sizeof(*th))))
		return NULL;

	th->tid = tid;
	th->status = status;
	th->addr = 0;

	return th;
}

TH_INFO	*get_th(int tid)
{
	struct list_head *pos;

	list_for_each_prev(pos, &ps.th_list) {
		TH_INFO	*th = list_entry(pos, TH_INFO, list);

		if(tid == th->tid)
			return th;
	}

	return NULL;
}

void free_th()
{
	struct list_head *p, *aux;

	p = (&ps.th_list)->next;

	while(p && p != &(ps.th_list)) {
		TH_INFO		*th = list_entry(p, TH_INFO, list);

		aux = p->next;
		list_del(&(th->list));
		free(th);
		p = aux;
	}
}

