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
#include "ruby.h"
#include <plugin.h>
#include <main.h>
#include "../../main.h"
#include "../../dbg/debug.h"

/* extern */
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;

/* static stuff */
static char *(*rs_cmdstr)(const char *cmd) = NULL;
static int (*rs_cmd)(char *cmd, int log) = NULL;

static int slurp_ruby(char *file)
{
	rb_load_file(file);
	ruby_run();
	return 0;
}

static VALUE radare_ruby_cmd(VALUE self, VALUE string)
{
	const char *retstr;

	Check_Type(string, T_STRING);

	retstr = rs_cmdstr ( RSTRING(string)->ptr );
	if (retstr == NULL || retstr[0]=='\0')
		return rb_str_new2("");
		//return Qnil;

	return rb_str_new2("Hello message\n");
}

#define RUBYAPI  LIBDIR"/radare/radare.rb"
static int ruby_hack_init()
{
	ruby_init();

	VALUE rb_RadareCmd = rb_define_class("Radare", rb_cObject);
	rb_define_method(rb_RadareCmd, "cmd", radare_ruby_cmd, 1);
	rb_eval_string("r = Radare.new()");

	printf("Loading radare ruby api... %s\n",
		slurp_ruby(RUBYAPI)? "error ( "RUBYAPI" )":"ok");
	fflush(stdout);

	return 0;
}

static int ruby_hack_cya()
{
	/* TODO */
	return 0;
}

void ruby_hack_cmd(char *input)
{
	if (rs_cmdstr == NULL)
		rs_cmdstr = radare_plugin.resolve("radare_cmd_str");

	if (rs_cmd == NULL)
		rs_cmd = radare_plugin.resolve("radare_cmd");

	if (rs_cmd == NULL || rs_cmdstr == NULL) {
		printf("cannot find radare_cmd_str or radare_cmd\n");
		return;
	}

	ruby_hack_init();

	if (input && input[0]) {
		if (slurp_ruby(input)) {
			fprintf(stderr, "Cannot open '%s'\n", input);
			fflush(stdout);
		}
	} else {
		char str[1024];
		while(!feof(stdin)) {
			printf("ruby> ");
			fflush(stdout);
			str[0]='\0';
			fgets(str,1000,stdin);
			if (str[0])
				str[strlen(str)-1]='\0';
			if (	str[0]=='.'
			||	feof(stdin)
			||	!memcmp(str,"exit",4)
			||	!memcmp(str,"quit",4)
			||	!strcmp(str,"q"))
				break;
			str[strlen(str)]='\0';
			rb_eval_string(str);
		}
	}
	ruby_hack_cya();
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "ruby",
	.desc = "ruby plugin",
	.callback = &ruby_hack_cmd
};
