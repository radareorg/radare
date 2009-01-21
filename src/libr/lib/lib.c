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

#include "r_types.h"
#include "r_lib.h"
#include <stdio.h>
#include <dirent.h>

/* TODO: support for nested plugins ?? here */

#if __UNIX__
#include <dlfcn.h>
  #define DLOPEN(x)  dlopen(x, RTLD_GLOBAL | RTLD_NOW)
  #define DLSYM(x,y) dlsym(x,y)
  #define DLCLOSE(x) dlclose(x)
#elif __WINDOWS__ && !__CYGWIN__
#include <windows.h>
  #define DLOPEN(x)  LoadLibrary(x)
  #define DLSYM(x,y) GetProcAddress(x,y)
  #define DLCLOSE(x) CloseLibrary(x)
#else
  #define DLOPEN(x)  NULL
  #define DLSYM(x,y) NULL
  #define DLCLOSE(x) NULL
#endif

void *r_lib_dl_open(const char *libname)
{
	return DLOPEN(libname);
}

void *r_lib_dl_sym(void *handle, const char *name)
{
	return DLSYM(handle, name);
}

int r_lib_dl_close(void *handle)
{
	return DLCLOSE(handle);
}

/* ---- */

int r_lib_init(struct r_lib_t *lib, const char *symname)
{
	INIT_LIST_HEAD(&lib->handlers);
	INIT_LIST_HEAD(&lib->plugins);
	strncpy(lib->symname, symname, sizeof(lib->symname)-1);
	return 0;
}

struct r_lib_t *r_lib_new(const char *symname)
{
	struct r_lib_t *lib = MALLOC_STRUCT(struct r_lib_t);
	r_lib_init(lib, symname);
	return lib;
}

int r_lib_free(struct r_lib_t *lib)
{
	/* TODO: iterate over libraries and free them all */
	/* TODO: iterate over handlers and free them all */
	free (lib);
	return 0;
}

/* THIS IS WRONG */
int r_lib_dl_check_filename(const char *file)
{
	/* skip hidden files */
	if (file[0]=='.')
		return 0;
	/* per SO dylib filename extensions */
	if (strstr(file, ".so"))
		return 1;
	if (strstr(file, ".dll"))
		return 1;
	if (strstr(file, ".dylib"))
		return 1;
	return 0;
}

/* high level api */

int r_lib_run_handler(struct r_lib_t *lib, struct r_lib_struct_t *plugin)
{
	struct list_head *pos;

	list_for_each_prev(pos, &lib->handlers) {
		struct r_lib_handler_t *h = list_entry(pos, struct r_lib_handler_t, list);
		if (h->type == plugin->type)
			return h->callback(h->user, plugin->data);
	}
	return 0;
}

int r_lib_open(struct r_lib_t *lib, const char *file)
{
	struct r_lib_struct_t *plugin;
	void * handler;
	int ret;

	/* ignored by filename */
	if (r_lib_dl_check_filename(file)) {
		fprintf(stderr, "Invalid library extension: %s\n", file);
		return -1;
	}

	handler = r_lib_dl_open(file);
	if (handler == NULL) {
		fprintf(stderr, "Cannot open library: %s\n", file);
		return -1;
	}
	plugin = (struct r_lib_struct_t *) r_lib_dl_sym(handler, lib->symname);
	if (plugin == NULL) {
		fprintf(stderr, "No root symbol '%s' found in library '%s'\n", lib->symname, file);
		return -1;
	}
	ret = r_lib_run_handler(lib, plugin);
	if (ret == -1) {
		fprintf(stderr, "Library handler returned -1 for '%s'\n", file);
	} else {
		/* append plugin */
		struct r_lib_plugin_t *p = MALLOC_STRUCT(struct r_lib_plugin_t);
		p->type = plugin->type;
		p->file = strdup(file);
		p->libhandler = handler;
		list_add(&p->list, &lib->handlers);
	}
	return ret;
}

int r_lib_opendir(struct r_lib_t *lib, const char *path)
{
	struct dirent *de;
	DIR *dh = opendir(path);
	if (dh == NULL) {
		fprintf(stderr, "Cannot open dir: %s\n", path);
		return -1;
	}
	while((de = (struct dirent *)readdir(dh))) {
		r_lib_open(lib, de->d_name);
	}
	closedir(dh);
	return 0;
}

int r_lib_list(struct r_lib_t *lib)
{
	struct list_head *pos;
	printf("Plugin Handlers:\n");
	list_for_each_prev(pos, &lib->handlers) {
		struct r_lib_handler_t *h = list_entry(pos, struct r_lib_handler_t, list);
		printf(" - %d: %s", h->type, h->desc);
	}
	printf("Loaded plugins:\n");
	list_for_each_prev(pos, &lib->plugins) {
		struct r_lib_plugin_t *p = list_entry(pos, struct r_lib_plugin_t, list);
		printf(" - %d : %s", p->type, p->file);
	}
	return 0;
}

int r_lib_add_handler(struct r_lib_t *lib, int type, const char *desc, int (*cb)(void *, void *), void *user )
{
	struct list_head *pos;
	struct r_lib_handler_t *handler = NULL;
	list_for_each_prev(pos, &lib->handlers) {
		struct r_lib_handler_t *h = list_entry(pos, struct r_lib_handler_t, list);
		if (type == h->type) {
			fprintf(stderr, "Redefining library handler callback for %d\n", type);
			handler = h;
			break;
		}
	}
	if (handler == NULL) {
		handler = MALLOC_STRUCT(struct r_lib_handler_t);
		handler->type = type;
		list_add(&handler->list, &lib->handlers);
	}
	strncpy(handler->desc, desc, sizeof(handler->desc));
	handler->user = user;
	handler->callback = cb;
	return 0;
}

#if 0
// TODO

int r_lib_del_handler(struct r_lib_t *lib)
{
	return 0;
}
int r_lib_close(struct r_lib_t *lib, const char *file)
{
	return 0;
}

typedef int r_lib_delegate_t (void *user, void *ptr);

#endif
