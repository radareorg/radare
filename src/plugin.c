/*
 * Copyright (C) 2007, 2008, 2009
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
#if __UNIX__ || __CYGWIN__
#include <dlfcn.h>
#endif
#if HAVE_GUI
#include <gtk/gtk.h>
#endif

static plugin_t plugins[MAXPLUGINS];
static int plugin_init_flag = 0;

#include "io.c"

/* 
 * z = zero terminated string
 * i = integer
 * p = pointer
 * a = 64bit address
 */

static struct core_t core[]={
	{ .ptr = &radare_cmd,      .name = "radare_cmd",      .args = "zi" },
	{ .ptr = &radare_cmdf,     .name = "radare_cmdf",     .args = "z*" },
	{ .ptr = &radare_cmd_str,  .name = "radare_cmd_str",  .args = "z"  },
	{ .ptr = &radare_exit,     .name = "radare_exit",     .args = ""   },
	{ .ptr = &radare_open,     .name = "radare_open",     .args = "i"  },
	{ .ptr = &radare_sync,     .name = "radare_sync",     .args = ""   },
	{ .ptr = &radare_block,    .name = "radare_block",    .args = ""   },
	{ .ptr = &radare_read_at,  .name = "radare_read_at",  .args = "api"},
	{ NULL, NULL, NULL }
};

static void *plugin_resolve(const char *name)
{
	int i,j = (int)(size_t)name;
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
#elif __DARWIN__
	if (strstr(str, ".dylib"))
		return 1;
#else
	if (strstr(str, ".so"))
		return 1;
#endif
	return 0;
}

plugin_t *plugin_registry(const char *file)
{
	int i;
	void *hd;
	char *ptr;
	plugin_t *p;
	const char *ip;
	char buf[4096];
#if HAVE_GUI
	static int gtk_is_init = 0;
#endif
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

	p = (plugin_t *)GetProcAddress(h, "radare_plugin_type");
	if (p == NULL) {
		eprintf("cannot find 'radare_plugin_type' symbol.\n");
		return NULL;
	}
#else
	// TODO: support dynlib and so
	strcat(buf, ".so");
	/* open library */
	hd = (void *)dlopen(buf, RTLD_GLOBAL | RTLD_NOW); //LAZY);
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
		p = (plugin_t *)GetProcAddress(h, "radare_plugin");
		#else
		p = (plugin_t *)dlsym(hd, "radare_plugin");
		#endif
#if HAVE_GUI
	case PLUGIN_TYPE_GUI:
		/* initialize gtk before */
		if (!gtk_is_init) {
			if ( ! gtk_init_check(NULL, NULL) ) {
				//fprintf(stderr, "Oops. Cannot initialize gui\n");
				return 0;
			}
		gtk_is_init = 1;
		}
#endif
	case PLUGIN_TYPE_HACK: {
		#if __WINDOWS__ && !__CYGWIN__
		struct plugin_hack_t *pl = (struct plugin_hack_t *)GetProcAddress(h, "radare_plugin");
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

		pl->resolve = plugin_resolve;
		pl->config = &config;
#if DEBUGGER
		pl->ps = &ps;
#endif
		//return NULL;
		} break;
	default:
		//eprintf("Unknown plugin type '%d'\n", (int)p);
		if (((int)(size_t)p) == PLUGIN_TYPE_GUI)
			eprintf("You need GUI to run this plugin. Sorry\n");
		return NULL;
	}

	sprintf(buf, "%s_plugin", file);
	/* find a place to store our child */
	for(i=0; i<MAXPLUGINS && plugins[i].name; i++);
	plugins[i] = *p;
	plugins[i+1] = posix_plugin;
	return p;

}

// TODO: plugin_close() ?

/* load plugins from dir.plugins */
void plugin_load()
{
	const char *str = config_get("dir.plugins");
	// add hack plugins if dir.plugins defined
	if (!strnull(str)) {
		DIR *fd = opendir(str);
		struct dirent *de;
		if (fd == NULL) {
			eprintf("Cannot open dir.plugins '%s'\n", str);
			return;
		}
		while(de = (struct dirent *)readdir(fd)) {
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

	/* r_io_init() */
	io_map_init();
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
  #endif
#endif

#if __UNIX__
	plugins[last++] = shm_plugin;
	plugins[last++] = mmap_plugin;
	plugins[last++] = serial_plugin;
   #if DEBUGGER
	plugins[last++] = debug_plugin;
	(debug_plugin.init)();
    #if SYSPROXY
	plugins[last++] = sysproxy_plugin;
    #endif
  #endif
#endif

#if HAVE_LIB_EWF
	plugins[last++] = ewf_plugin;
#endif

#if SYSPROXY
	plugins[last++] = sysproxy_plugin;
#endif
	plugins[last++] = malloc_plugin;
	plugins[last++] = remote_plugin;
	plugins[last++] = winedbg_plugin;
	plugins[last++] = windbg_plugin;
	plugins[last++] = socket_plugin;
	plugins[last++] = gxemul_plugin;
	plugins[last++] = bfdbg_plugin;
	plugins[last++] = gdbwrap_plugin;
	plugins[last++] = gdb_plugin;
	plugins[last++] = gdbx_plugin;

	/* must be dupped or will die */
	plugins[last++] = posix_plugin;

	//plugins[last++] = winegdb_plugin;

	radare_hack_init();
}
