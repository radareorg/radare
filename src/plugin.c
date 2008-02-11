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
#include <dirent.h>
#include "plugin.h"
#if __UNIX__
#include <dlfcn.h>
#endif

plugin_t plugins[10];
plugin_t plugins_backup[10];
int plugin_init_flag = 0;
/* core functions */
struct core_t {
	void *ptr;
	char *name;
	char *args;
};

static struct core_t core[]={
	{ .ptr = &radare_cmd,      .name = "radare_cmd",      .args = "zi" },
	{ .ptr = &radare_cmd_str,  .name = "radare_cmd_str",  .args = "z" },
	{ .ptr = &radare_exit,     .name = "radare_exit",     .args = "" },
	{ .ptr = &radare_open, .name = "radare_open", .args = "i" },
	{ .ptr = &radare_sync, .name = "radare_sync", .args = "" },
	{ NULL,NULL,NULL}
};

void *radare_resolve(char *name)
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

int is_plugin(const char *str)
{
// TODO: use SHARED_EXT
#if __WINDOWS__
	if (strstr(str, ".dll"))
		return 1;
#endif
	if (strstr(str, ".so"))
		return 1;
	return 0;
}

plugin_t *plugin_registry(const char *file)
{
	int i;
	void *hd;
	char *ptr;
	plugin_t *p;
	int *ip;
	char buf[4096];
#if __WINDOWS__
	HMODULE h;
#endif

	if (strlen(file)>254) {
		eprintf("Invalid plugin name\n");
		return NULL;
	}

	/* find a place to store our child */
	for(i=0; i<10 && plugins[i].name; i++);i--;

	/* construct file name */
	ip = config_get("dir.plugins");
	if (ip) {
		strcpy(buf, ip);
	} else buf[0]='\0';
#if 0
	if (file[0]!='/') {
		buf[4000]='\0';
		getwd(buf);
		if (buf[4000]!='\0') {
			eprintf("Dont oflowme nauh!\n");
			exit(1);
		}
		strcat(buf,"/");
	}
#endif

	strcat(buf, file);
	if (  (ptr = strstr(buf,".so"))
	   || (ptr = strstr(buf,".dll")))
		ptr[0]='\0';

#if __WINDOWS__
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
		eprintf("Cannot open plugin '%s'.\n(%s)\n", buf,dlerror());
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
	ip = p;
	switch(((int)(*ip))) {
	case PLUGIN_TYPE_IO:
		p = (plugin_t *)malloc(sizeof(plugin_t));
		#if __WINDOWS__
		p = GetProcAddress(h, "radare_plugin");
		#else
		p = dlsym(hd, "radare_plugin");
		#endif
		break;
	case PLUGIN_TYPE_HACK: {
		#if __WINDOWS__
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
		list_add_tail(&(hack->list), &(hacks));
		pl->resolve = (void *)&radare_resolve;
		pl->config = &config;
#if DEBUGGER
		pl->ps = &ps;
#endif
		return NULL;
		} break;
	default:
		eprintf("Unknown plugin type '%d'\n", (int)p);
		return NULL;
	}

	sprintf(buf, "%s_plugin", file);
	plugins[i] = *p;
	plugins[i+1] = posix_plugin;
	printf("plugin registered??\n");

	return p;
}

// TODO: plugin_close() ?

int plugin_list()
{
	int i;
	for(i=0;i<10 && plugins[i].name; i++)
		printf("%-10s  %s\n", plugins[i].name, plugins[i].desc);
	return i;
}

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
		return 0;
	}
}

void plugin_init()
{
	int last = 0;

	if (plugin_init_flag)
		return;

	plugin_init_flag = 1;
	memset(&plugins,'\0', sizeof(plugin_t)*10);
	/* load libraries in current directory */
	/* load libraries in -l path */
	plugins[0] = haret_plugin;
#if __WINDOWS__
	plugins[1] = w32_plugin;
	plugins[2] = remote_plugin;
  #if DEBUGGER
	plugins[3] = debug_plugin;
	plugins[4] = posix_plugin;
	(debug_plugin.init)();
	last = 4;
  #else
	plugins[3] = posix_plugin;
	last = 3;
  #endif
#else
   #if DEBUGGER
	plugins[1] = debug_plugin;
	(debug_plugin.init)();
	//(ptrace_plugin.init)();
	plugins[2] = gdb_plugin;
	plugins[3] = remote_plugin;
	last = 3;
    #if SYSPROXY
	plugins[4] = sysproxy_plugin;
	plugins[5] = posix_plugin;
	last = 5;
    #else
	plugins[4] = posix_plugin;
	last = 4;
    #endif
  #else
	plugins[1] = posix_plugin;
	last = 1;
  #endif
#endif
#if HAVE_LIB_EWF
	plugins[last] = ewf_plugin;
	last += 1;
#endif
	plugins[last] = winedbg_plugin;
	plugins[last+1] = gxemul_plugin;
	plugins[last+2] = posix_plugin;
	last += 3;

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
		IF_HANDLED(fd, read)
			return plugins[i].lseek(fd, offset, whence);
	return -1;
}

ssize_t io_write(int fd, const void *buf, size_t count)
{
	if (!config_get("cfg.write")) {
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
		IF_HANDLED(fd, close)
			return (int)(plugins[i].debug);
	return 0;
}
