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

/* TODO: support for nested plugins ?? here */

#if __UNIX__
#include <dlfcn.h>
#endif

#if __WINDOWS__ && !__CYGWIN__
#include <windows.h>
#endif


void *r_lib_open(const char *libname)
{
#if __WINDOWS__ && !__CYGWIN__
	return LoadLibrary(libname);
#else
	return dlopen(libname, RTLD_GLOBAL | RTLD_NOW);
#endif
	return NULL;
}

void *r_lib_sym(void *handle, const char *name)
{
#if __WINDOWS__ && !__CYGWIN__
	return GetProcAddress(handle, name);
#else
	return dlsym(handle, name);
#endif
}

int r_lib_close(void *handle)
{
#if __WINDOWS__ && !__CYGWIN__
	return CloseLibrary(handle, name);
#else
	return dlclose(handle);
#endif
}
