/*
 * Copyright (C) 2007, 2008
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

#include "main.h"
#include "plugin.h"
#include <dirent.h>
#include "plugin.h"
#if __UNIX__ || __CYGWIN__
#include <dlfcn.h>
#endif
#ifdef VALA
#include <gtk/gtk.h>
#endif

plugin_t plugins[MAXPLUGINS];
plugin_t plugins_backup[MAXPLUGINS];
int plugin_init_flag = 0;

struct core_t {
	void *ptr;
	char *name;
	char *args;
};

static struct core_t core[]={
	{ .ptr = &radare_cmd,      .name = "radare_cmd",      .args = "zi" },
	{ .ptr = &radare_cmd_str,  .name = "radare_cmd_str",  .args = "z"  },
	{ .ptr = &radare_exit,     .name = "radare_exit",     .args = ""   },
	{ .ptr = &radare_open,     .name = "radare_open",     .args = "i"  },
	{ .ptr = &radare_sync,     .name = "radare_sync",     .args = ""   },
	{ NULL, NULL, NULL }
};

static void *plugin_resolve(char *name)
{
	int i,j = (int)name;
	if (j>0 && j<255) {
		// get by index
		for(i=0;core[i].ptr&&i<j;i++)
			if (i == j)
				return core[i].ptr;
		return NULL;
	}
	for(i=0;core[i].ptr;i++)
		if (!strcmp(core[i].name, name))
			return core[i].ptr;
	return NULL;
}

int plugin_list()
{
	int i;
	for(i=0;i<MAXPLUGINS && plugins[i].name; i++)
		printf("%-10s  %s\n", plugins[i].name, plugins[i].desc);
	return i;
}

int is_plugin(const char *str)
{
// TODO: use SHARED_EXT
#if __WINDOWS__ && !__CYGWIN__
	if (strstr(str, ".dll"))
		return 1;
#endif
	if (strstr(str, ".so"))
		return 1;
// TODO __DARWIN__ ??
	if (strstr(str, ".dylib"))
		return 1;
	return 0;
}

plugin_t *plugin_registry(const char *file)
{
	int i;
	void *hd;
#ifdef VALA
	static int gtk_is_init = 0;
#endif
	char *ptr;
	plugin_t *p;
	const char *ip;
	char buf[4096];
#if __WINDOWS__
	HMODULE h;
#endif
	if (strlen(file)>254) {
		eprintf("Invalid plugin name\n");
		return NULL;
	}

	/* construct file name */
	ip = config_get("dir.plugins");
	if (ip) {
		strcpy(buf, ip);
	} else buf[0]='\0';

	strcat(buf, file);
	if (  (ptr = strstr(buf,".so"))
	   || (ptr = strstr(buf,".dll")))
		ptr[0]='\0';

#if __WINDOWS__ && !__CYGWIN__
	strcat(buf, ".dll");

	h = LoadLibrary(buf);
	if (h == NULL) {
		eprintf("Cannot open library (%s)\n", buf);
		return NULL;
	}

	p = GetProcAddress(h, "radare_plugin_type");
	if (p == NULL) {
		eprintf("cannot find 'radare_plugin_type' symbol.\n");
		return NULL;
	}
#else
	// TODO: support dynlib and so
	strcat(buf, ".so");
	/* open library */
	hd = (void *)dlopen(buf, RTLD_NOW); //LAZY);
	if (hd == NULL) {
		perror("dlopen");
		eprintf("Cannot open plugin '%s'.\n(%s)\n", buf, dlerror());
		return NULL;
	}

	dlerror(); // clear error buffer
	p = dlsym(hd, "radare_plugin_type");
	if (p == NULL) {
		eprintf("cannot find 'radare_plugin_type' symbol.\n(%s)\n", dlerror());
		dlclose(hd);
		return NULL;
	}
#endif
	ip = (const char *)p;
	switch(((int)(*ip))) {
	case PLUGIN_TYPE_IO:
		p = (plugin_t *)malloc(sizeof(plugin_t));
		#if __WINDOWS__ && !__CYGWIN__
		p = GetProcAddress(h, "radare_plugin");
		#else
		p = dlsym(hd, "radare_plugin");
		#endif
#ifdef VALA
	case PLUGIN_TYPE_GUI:
		/* initialize gtk before */
		if (!gtk_is_init) {
			if ( ! gtk_init_check(NULL, NULL) ) {
				fprintf(stderr, "Oops. Cannot initialize gui\n");
				return;
			}
		gtk_is_init = 1;
		}
#endif
	case PLUGIN_TYPE_HACK: {
		#if __WINDOWS__ && !__CYGWIN__
		struct plugin_hack_t *pl = GetProcAddress(h, "radare_plugin");
		#else
		struct plugin_hack_t *pl = dlsym(hd, "radare_plugin");
		#endif
		struct hack_t *hack;
		if (pl == NULL) {
			eprintf("error: Cannot find symbol 'radare_plugin' in %s\n", buf);
			return NULL;
		}
		hack = radare_hack_new(pl->name, pl->desc, pl->callback);
		hack->type = ((int)(*ip));
		hack->widget = pl->widget;

		list_add_tail(&(hack->list), &(hacks));

		pl->resolve = (void *)&plugin_resolve;
		pl->config = &config;
#if DEBUGGER
		pl->ps = &ps;
#endif
		//return NULL;
		} break;
	default:
		//eprintf("Unknown plugin type '%d'\n", (int)p);
		if (((int)p) == PLUGIN_TYPE_GUI)
			eprintf("You need GUI to run this plugin. Sorry\n");
		return NULL;
	}

	sprintf(buf, "%s_plugin", file);
	/* find a place to store our child */
	for(i=0; i<MAXPLUGINS && plugins[i].name; i++);i--;
	plugins[i] = *p;
	plugins[i+1] = posix_plugin;
	return p;

}

// TODO: plugin_close() ?


/* load plugins from dir.plugins */
void plugin_load()
{
	char *str = config_get("dir.plugins");
	// add hack plugins if dir.plugins defined
	if (!strnull(str)) {
		DIR *fd = opendir(str);
		struct dirent *de;
		if (fd == NULL) {
			eprintf("Cannot open dir.plugins '%s'\n", str);
			return;
		}
		while(de = readdir(fd)) {
			if (de->d_name[0] && de->d_name[0]!='.' && is_plugin(de->d_name)) {
				plugin_registry(de->d_name);
			}
		}
		closedir(fd);
	}
}

void plugin_init()
{
	int last = 0;

	if (plugin_init_flag)
		return;

	plugin_init_flag = 1;
	memset(&plugins,'\0', sizeof(plugin_t)*MAXPLUGINS);
	/* load libraries in current directory */
	/* load libraries in -l path */
	plugins[last++] = haret_plugin;
#if __WINDOWS__
	extern plugin_t w32_plugin;
	plugins[last++] = w32_plugin;
  #if DEBUGGER
	plugins[last++] = debug_plugin;
	plugins[last++] = posix_plugin;
	(debug_plugin.init)();
  #else
	plugins[last++] = posix_plugin;
  #endif
#else
   #if DEBUGGER
	plugins[last++] = debug_plugin;
	(debug_plugin.init)();
	plugins[last++] = gdb_plugin;
	plugins[last++] = gdbx_plugin;
    #if SYSPROXY
	plugins[last++] = sysproxy_plugin;
    #endif
  #endif
#endif
#if HAVE_LIB_EWF
	plugins[last++] = ewf_plugin;
#endif
	plugins[last++] = remote_plugin;
	plugins[last++] = winedbg_plugin;
	plugins[last++] = socket_plugin;
	plugins[last++] = gxemul_plugin;
	plugins[last++] = posix_plugin;
	plugins[last++] = posix_plugin;

	//plugins[last++] = winegdb_plugin;

	radare_hack_init();
}

int io_system(const char *command)
{
	FIND_FD(config.fd)
		IF_HANDLED( config.fd, system )
			return (plugins[i].system)(command);
	
	if (!memcmp(command, "help", 4)) {
		eprintf("Not in debugger.\n");
		return 0;
	}
	return system(command);
}

/* io wrappers */
int io_open(const char *pathname, int flags, mode_t mode)
{
	FIND_OPEN(pathname)
		IF_HANDLED(0, open)
			return plugins[i].open(pathname, flags, mode);
	return -1;
}

ssize_t io_read(int fd, void *buf, size_t count)
{
	FIND_FD(fd)
		IF_HANDLED(fd, read)
			return plugins[i].read(fd, buf, count);
	return -1;
}

u64 io_lseek(int fd, u64 offset, int whence)
{
	FIND_FD(fd)
		IF_HANDLED(fd, lseek)
			return plugins[i].lseek(fd, offset, whence);
	return -1;
}

ssize_t io_write(int fd, const void *buf, size_t count)
{
	if (!config_get("file.write")) {
		eprintf("Not in write mode\n");
		return -1;
	}
	FIND_FD(fd)
		IF_HANDLED(fd, write)
			return plugins[i].write(fd, buf, count);
	return -1;
}

int io_close(int fd)
{
	FIND_FD(fd)
		IF_HANDLED(fd, close)
			return plugins[i].close(fd);
	return -1;
}

int io_isdbg(int fd)
{
	FIND_FD(fd)
		IF_HANDLED(fd, open)
			return (int)(plugins[i].debug);
	return 0;
}
