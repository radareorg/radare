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
#ifdef __APPLE__
#include "ruby/ruby.h"
#else
#include "ruby.h"
#endif
#include <plugin.h>
#include <main.h>
#include "../../main.h"
#include "../../dbg/debug.h"

 // XXX buggy ?!?

/* extern */
extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;

/* static stuff */
static char *(*rs_cmdstr)(const char *cmd) = NULL;
static int (*rs_cmd)(char *cmd, int log) = NULL;

static int slurp_ruby(char *file)
{
	rb_load_file(file);
	ruby_exec();
	return 0;
}

static VALUE radare_ruby_cmd(VALUE self, VALUE string)
{
	const char *retstr;

	Check_Type(string, T_STRING);

	retstr = rs_cmdstr ( RSTRING(string)->ptr );
	if (retstr == NULL || retstr[0]=='\0')
		return rb_str_new2("");
	return rb_str_new2(retstr);//"Hello message\n");
}

#define RUBYAPI  LIBDIR"/radare/radare.rb"
static int ruby_hack_init()
{
	int err;
	ruby_init();
	ruby_init_loadpath();
ruby_debug=0;
ruby_verbose=1;

	VALUE rb_RadareCmd = rb_define_class("Radare", rb_cObject);
	rb_define_method(rb_RadareCmd, "cmd", radare_ruby_cmd, 1);
	//rb_eval_string_protect("r = Radare.new()", NULL);

	printf("==> Loading radare ruby api... %s\n",
		slurp_ruby(RUBYAPI)? "error ( "RUBYAPI" )":"ok");
	fflush(stdout);


	return 0;
}

static int ruby_hack_cya()
{
	/* TODO */
	ruby_finalize();
	return 0;
}

int ruby_hack_cmd(char *input)
{
	int err, rb_state = 0;
	char str[1024];

	if (rs_cmd == NULL)
		rs_cmd = radare_plugin.resolve("radare_cmd");

	if (rs_cmdstr == NULL)
		rs_cmdstr = radare_plugin.resolve("radare_cmd_str");

	if (rs_cmd == NULL || rs_cmdstr == NULL) {
		printf("cannot find radare_cmd_str or radare_cmd\n");
		return 1;
	}

	ruby_hack_init();

	if (input && input[0]) {
		if (slurp_ruby(input)) {
			fprintf(stderr, "Cannot open '%s'\n", input);
			fflush(stderr);
		}
	} else {
		rb_eval_string_protect("require 'irb'; $r = Radare.new(); IRB.start();", &err);
		if (err != 0) 
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

			rb_eval_string_protect(str, &rb_state);
#if 0
			rb_eval_string_protect("rescue => e\n", NULL);
			rb_eval_string_protect(" puts e.exception()\n", NULL);
			rb_eval_string_protect("end\n", NULL);

#endif
			if (rb_state != 0) {
				printf("Fuck yeah: %d\n", rb_state);
#if 0
				rb_eval_string_protect("rescue => e\n", NULL);
				rb_eval_string_protect(" puts e.exception()\n", NULL);
				rb_eval_string_protect("end\n", NULL);
				rb_eval_string_protect(str, &rb_state);
#endif
			}
			//ruby_exec();
		}
		clearerr(stdin);
	}
	//ruby_hack_cya();
	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "ruby",
	.desc = "ruby plugin",
	//.init = &ruby_hack_init,
	.callback = &ruby_hack_cmd
};
