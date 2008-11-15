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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "r_io.h"
#include "list.h"

#define IO_MAP_N 10
struct io_maps_t {
        int fd;
        char file[128];
        u64 from;
        u64 to;
        struct list_head list;
};

static int maps_n = 0;
static int maps[10];

static struct list_head io_maps;

void r_io_map_init()
{
	INIT_LIST_HEAD(&io_maps);
}

int r_io_map_rm(int fd)
{
	struct list_head *pos;
	list_for_each_prev(pos, &io_maps) {
		struct io_maps_t *im = list_entry(pos, struct io_maps_t, list);
		if (im->fd == fd) {
			/* FREE THIS */
			r_io_handle_close(fd, r_io_handle_resolve_fd(fd));
			fprintf(stderr, "r_io_map_rm: TODO\n");
			return 0;
		}
	}
	fprintf(stderr, "Not found\n");
	return 0;
}

int r_io_map_list()
{
	int n = 0;
	struct list_head *pos;
	list_for_each_prev(pos, &io_maps) {
		struct io_maps_t *im = list_entry(pos, struct io_maps_t, list);
		if (im->file[0] != '\0') {
			printf("0x%08llx 0x%08llx %s\n",
				im->from, im->to, im->file);
			n++;
		}
	}
	return n;
}

int r_io_map(const char *file, u64 offset)
{
	struct io_maps_t *im;
	int fd = r_io_open(file, R_IO_READ, 0644);
	if (fd == -1)
		return -1;
	im = malloc_struct(struct io_maps_t);
//(struct io_maps_t*)malloc(sizeof(struct io_maps_t));
	if (im == NULL) {
		r_io_close(fd);
		return -1;
	}
	im->fd     = fd;
	strncpy(im->file, file, 127);
	im->from = offset;
	im->to   = offset+lseek(fd, 0, SEEK_END);
	list_add_tail(&(im->list), &(io_maps));
	return fd;
}

int r_io_map_read_at(u64 off, u8 *buf, u64 len)
{
	struct list_head *pos;

	list_for_each_prev(pos, &io_maps) {
		struct io_maps_t *im = list_entry(pos, struct io_maps_t, list);
		if (im->file[0] != '\0' && off >= im->from && off < im->to) {
			r_io_lseek(im->fd, off-im->from, SEEK_SET);
			return r_io_read(im->fd, buf, len);
		}
	}
	return 0;
}

int r_io_map_write_at(u64 off, u8 *buf, u64 len)
{
	struct list_head *pos;

	list_for_each_prev(pos, &io_maps) {
		struct io_maps_t *im = list_entry(pos, struct io_maps_t, list);
		if (im->file[0] != '\0' && off >= im->from && off < im->to) {
			r_io_lseek(im->fd, off-im->from, SEEK_SET);
			return r_io_write(im->fd, buf, len);
		}
	}
	return 0;
}

int r_io_map_read_rest(u64 off, u8 *buf, u64 len)
{
	struct list_head *pos;

	list_for_each_prev(pos, &io_maps) {
		struct io_maps_t *im = list_entry(pos, struct io_maps_t, list);
		if (im->file[0] != '\0' && off+len >= im->from && off < im->to) {
			lseek(im->fd, 0, SEEK_SET);
			return read(im->fd, buf+(im->from-(off+len)), len);
		}
	}
	return 0;
}
