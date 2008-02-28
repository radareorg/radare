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

#define RADARE_MODULE
#include <plugin.h>
#include <main.h>
#include <lua.h>
#include <lualib.h>
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;
static char *(*rs)(const char *cmd) = NULL;

static int l_num (lua_State *L) {
	double d = lua_tonumber(L, 1);  /* get argument */
	lua_pushnumber(L, d+1);  /* push result */
	return 1;  /* number of results */
}

static int lua_cmd_str (lua_State *L) {
	char *str;
	char *s = lua_tostring(L, 1);  /* get argument */
	str = rs(s);
	lua_pushstring(L, str);  /* push result */
	free(str);
	return 1;  /* number of results */
}

static lua_State *L;

static int lua_hack_init()
{
	printf("Initializing LUA vm...\n");
	fflush(stdout);

 	L = (lua_State*)lua_open();
	if (L==NULL) {
		printf("Exit\n");	
		return 0;
	}

	lua_gc(L, LUA_GCSTOP, 0);
	luaL_openlibs(L);
	luaopen_base(L);
	luaopen_string(L);
	//luaopen_io(L); // PANIC!!
	lua_gc(L, LUA_GCRESTART, 0);

	//-- load template
	printf("Loading radare api... %s\n",
		slurp_lua(LIBDIR"/radare/radare.lua")?"ok":"error");
	fflush(stdout);

	lua_register(L, "radare_cmd_str", &lua_cmd_str);
	lua_pushcfunction(L, lua_cmd_str);
	lua_setglobal(L,"radare_cmd_str");

	// DEPRECATED: cmd = radare_cmd_str
	lua_register(L, "cmd", &lua_cmd_str);
	lua_pushcfunction(L,lua_cmd_str);
	lua_setglobal(L,"cmd");
}

static int lua_hack_cya()
{
	lua_close(L);
}

static char *slurp(char *str)
{
	char *ret;	
	u64 sz;
	FILE *fd = fopen(str, "r");
	if (fd == NULL)
		return NULL;
	fseek(fd, 0,SEEK_END);
	sz = ftell(fd);
	fseek(fd, 0,SEEK_SET);
	ret = (char *)malloc(sz+1);
	fread(ret, sz, 1, fd);
	ret[sz]='\0';
	fclose(fd);
	return ret;
}

static int slurp_lua(char *file)
{
	char *str = slurp(file);
	printf("slurp(%s)\n", file);
	if (str) {
		luaL_loadbuffer(L, str, strlen(str), "");
		lua_pcall(L,0,0,0);
		free(str);
		return 1;
	}
	return 0;
}

void lua_hack_cmd(char *input)
{
	if (rs == NULL)
		rs = radare_plugin.resolve("radare_cmd_str");

	if (rs == NULL) {
		printf("cannot find radare_cmd_str\n");
		return;
	}

	lua_hack_init();

	if (input && input[0]) {
		if (!slurp_lua(input)) {
			fprintf(stderr, "Cannot open '%s'\n", input);
			fflush(stdout);
		}
	} else {
		char str[1024];
		while(!feof(stdin)) {
			printf("lua> ");
			fflush(stdout);
			str[0]='\0';
			fgets(str,1000,stdin);
			if (str[0]=='.'||feof(stdin)||!memcmp(str,"exit",4)||!memcmp(str,"quit",4)||!strcmp(str,"q"))
				break;
			str[strlen(str)]='\0';
			luaL_loadbuffer(L, str, strlen(str), ""); // \n included
			if ( lua_pcall(L,0,0,0) != 0 )
				printf("Oops\n");
		}
	}
	lua_hack_cya();
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "lua",
	.desc = "lua plugin",
	.callback = &lua_hack_cmd
};
