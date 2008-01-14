// DEPRECATED // MAINTAINED JUST FOR FUN //
/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
 *       th0rpe <th0rpe@nopcode.org>
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
#include "list.h"
#include "watchpoint.h"
#include <stdlib.h>

void watchpoint_add(struct watchpoint_t *wp)
{
	struct watchpoint_t *w = (struct watchpoint_t*)
		malloc(sizeof(struct watchpoint_t*));
	memcpy(w, wp, sizeof(struct watchpoint_t*));

	list_add_tail(&(w->list), &(ps.wps));
}

void watchpoint_free(struct watchpoint_t *wp)
{
	free(wp->value);
	wp->value = NULL;
}

/* TODO: watk foreach watchpoint checking its condition */
/* This function is called from ptrace_wait() */

void watchpoint_check()
{
	struct list_head *pos = NULL;

	list_for_each_prev(pos, &ps.wps) {
		struct watchpoint_t *wp = (struct watchpoint_t *) pos +
			sizeof(struct list_head) -
			sizeof(struct watchpoint_t);
		// TODO: CHECK CONDITIONS FOR THE WATCHPOINT
	}
}

int watchpoint_parse(char *str, struct watchpoint_t *wp)
{
	if ((!strcmp(str, "eax"))
	||  (!strcmp(str, "ebx"))
	||  (!strcmp(str, "ecx"))
	||  (!strcmp(str, "edx"))
	||  (!strcmp(str, "esi"))
	||  (!strcmp(str, "edi"))
	||  (!strcmp(str, "ebp"))
	||  (!strcmp(str, "esp"))) {
		wp->type    = WP_REGISTER;
		wp->address = arch_pc(); // XXX must be the value or R_ENX
		wp->value   = strdup(str);
		return 1;
	} else {
		off_t addr  = get_math(str);
		if (addr!= 0) {
			wp->value   = strdup(str);
			wp->address = addr;
			wp->type    = WP_MEMORY;
			return 1;
		}
	}

	return 0;
}

void watchpoint_show(struct watchpoint_t *wp)
{
	switch(wp->type) {
	case WP_REGISTER:
		fprintf(stderr, "Type: REGISTER\n");
		fprintf(stderr, "Value: %s\n", wp->value);
		break;
	case WP_MEMORY:
		break;
	}
}

void watchpoint_list()
{
	struct list_head *pos = NULL;

	list_for_each_prev(pos, &ps.wps) {
		struct watchpoint_t *wp = (struct watchpoint_t *) pos +
			sizeof(struct list_head) -
			sizeof(struct watchpoint_t);
		printf(" * watchpoint %s %s %x %s\n",
			(wp->type==WP_REGISTER)?"reg":"mem", 
			(wp->action==ACTION_MONITOR)?"monitor":"stop",
			wp->address, wp->value);
	}
}
