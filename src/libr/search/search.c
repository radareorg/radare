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

#include "r_search.h"

struct r_search_t *r_search_new(int fd)
{
	struct r_search_t *r = malloc(sizeof(struct r_search_t));
	memset(r,'\0', sizeof(struct r_search_t));
	r->fd = fd;
	INIT_LIST_HEAD(&(r->kw));
	return r;
}

struct r_search_tr_search_free(struct r_search_t *s)
{
	free(s);

	return NULL;
}
