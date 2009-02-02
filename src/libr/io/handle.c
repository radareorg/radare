/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

/* TODO: write li->fds setter/getter helpers */

#include "r_io.h"
#include "list.h"
#include <stdio.h>

struct io_list_t {
	struct r_io_handle_t *plugin;
	struct list_head list;
};

static struct list_head io_list;

int r_io_handle_init()
{
	INIT_LIST_HEAD(&io_list);
	/* load default IO plugins here */
	return 0;
}

int r_io_handle_add(struct r_io_handle_t *plugin)
{
	int i;
	struct io_list_t *li;
	li = MALLOC_STRUCT(struct io_list_t);
	if (li == NULL)
		return -1;
	li->plugin = plugin;
	for(i=0;i<R_IO_NFDS;i++)
		li->plugin->fds[i] = -1;
	list_add_tail(&(li->list), &(io_list));
	return 0;
}

struct r_io_handle_t *r_io_handle_resolve(const char *filename)
{
	struct list_head *pos;
	list_for_each_prev(pos, &io_list) {
		struct io_list_t *il = list_entry(pos, struct io_list_t, list);
		printf("resolving (%s)\n", filename);
		if (il->plugin->handle_open(filename))
			return il->plugin;
	}
		printf("cannot resolve\n");
	return NULL;
}

struct r_io_handle_t *r_io_handle_resolve_fd(int fd)
{
	int i;
	struct list_head *pos;
	list_for_each_prev(pos, &io_list) {
		struct io_list_t *il = list_entry(pos, struct io_list_t, list);
		for(i=0;i<R_IO_NFDS;i++) {
			if (il->plugin->fds[i] == fd)
				return il->plugin;
		}
	}
	return NULL;
}

int r_io_handle_generate()
{
	return (random()%666)+1024;
}

int r_io_handle_open(int fd, struct r_io_handle_t *plugin)
{
	int i=0;
	struct list_head *pos;
	list_for_each_prev(pos, &io_list) {
		struct io_list_t *il = list_entry(pos, struct io_list_t, list);
		if (plugin == il->plugin) {
			for(i=0;i<R_IO_NFDS;i++) {
				if (il->plugin->fds[i] == -1) {
					il->plugin->fds[i] = fd;
					return 0;
				}
			}
			return -1;
		}
	}
	return -1;
}

int r_io_handle_close(int fd, struct r_io_handle_t *plugin)
{
	int i=0;
	struct list_head *pos;
	list_for_each_prev(pos, &io_list) {
		struct io_list_t *il = list_entry(pos, struct io_list_t, list);
		if (plugin == il->plugin) {
			for(i=0;i<R_IO_NFDS;i++) {
				if (il->plugin->fds[i] == fd) {
					il->plugin->fds[i] = -1;
					return 0;
				}
			}
			return -1;
		}
	}
	return -1;
}

int r_io_handle_list()
{
	int n = 0;
	struct list_head *pos;
	list_for_each_prev(pos, &io_list) {
		struct io_list_t *il = list_entry(pos, struct io_list_t, list);
		printf(" - %s\n", il->plugin->name);
		n++;
	}
	return n;
}
