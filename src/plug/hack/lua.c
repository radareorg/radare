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

#include <plugin.h>
#include <main.h>
#include <lua.h>
#include <lualib.h>

static int l_num (lua_State *L) {
	double d = lua_tonumber(L, 1);  /* get argument */
	lua_pushnumber(L, d+1);  /* push result */
	return 1;  /* number of results */
}

static int l_cmd (lua_State *L) {
	char *str;
	char *s = lua_tostring(L, 1);  /* get argument */
	str = radare_cmd_str(s);
	lua_pusstring(L, str);  /* push result */
	return 1;  /* number of results */
}


int main()
{
	lua_State *L = (lua_State*)lua_open();
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

	lua_register(L, "num", &l_num);
	lua_pushcfunction(L,l_num);
	lua_setglobal(L,"num");

	lua_register(L, "cmd", &l_cmd);
	lua_pushcfunction(L,l_cmd);
	lua_setglobal(L,"cmd");

	{
		char *cmd = "print \"pop\"";
		luaL_loadbuffer(L, cmd, strlen(cmd), "");
		lua_pcall(L,0,0,0);
	}

	lua_close(L);
}

void lua_cmd(char *input)
{
	if (rs == NULL)
		rs = radare_plugin.resolve("radare_cmd_str");

	if (rs == NULL) {
		printf("cannot find radare_cmd_str\n");
		return;
	}
	lua_hack_init();
	//Py_Initialize();
	//init_radare_module();
	//PyRun_SimpleString("import r");

	if (input && input[0]) {
		FILE *fd  = fopen(input, "r");
		if (fd == NULL) {
			fprintf(stderr, "Cannot open '%s'\n", input);
			fflush(stdout);
		} else {
		//	PyRun_SimpleFile(fd, input);
			fclose(fd);
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
	//		PyRun_SimpleString(str);
		}
	}
	//elua_destroy();
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "lua",
	.desc = "lua plugin",
	.callback = &lua_cmd
};
