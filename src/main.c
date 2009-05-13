/*
 * Copyright (C) 2006, 2007, 2008
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
#include <getopt.h>

int help_message(int line)
{
	if (line) 
		eprintf("radare [-dfhnuLvVwx] [-s #] [-b #] [-i f] [-P f] [-e k=v] [file]\n");
	else
	printf(
	"radare [options] [file]\n"
	"  -s [offset]      seek to the desired offset (cfg.seek)\n"
	"  -b [blocksize]   change the block size (512) (cfg.bsize)\n"
	"  -i [script]      interpret radare or ruby/python/perl/lua script\n"
	"  -p [project]     load metadata from project file\n"
	"  -l [plugin.so]   link against a plugin (.so or .dll)\n"
	"  -e [key=val]     evaluates a configuration string\n"
	"  -d [program|pid] debug a program. same as --args in gdb\n" /* TODO: fix this help */
	"  -f               set block size to fit file size\n"
	"  -L               list all available plugins\n"
	"  -w               open file in read-write mode\n"
	"  -x               dump block in hexa and exit\n"
	"  -n               do not load ~/.radarerc and ./radarerc\n"
	"  -v               same as -e cfg.verbose=false\n"
	"  -V               show version information\n"
	"  -u               unknown size (no seek limits)\n"
	"  -h               this help message\n");
	return 1;
}

int main(int argc, char **argv, char **envp)
{
	int pid, c;
	const char *prj;
	char buf[4096];
	char buf2[4096];
	int flag_d = 0;
	int __optind = 0;
	char *ptr;

	environ = envp;
	radare_init();

	while ((c = getopt(argc, argv, "l:fs:hb:wLvuVnxi:e:p:d")) != -1) {
		switch( c ) {
		case 'd':
			flag_d = 1;
			__optind = optind;
			break;
		case 'i':
			if (config.script)
				eprintf("Cannot interpret more than one -i file. Ignoring '%s'\n", optarg);
			else config.script = optarg;
			break;
		case 'f':
			config.block_size = 0;
			break;
		case 'n':
			config.noscript = 1;
			break;
		case 'p':
			config_set("file.project", optarg);
			break;
		case 'w':
			config_set("file.write", "true");
			break;
		case 's':
			config.seek = (u64)get_offset(optarg);
			if (config.seek < 0) config.seek = (u64)0;
			config_set("dbg.bep", optarg);
			break;
		case 'l':
			plugin_registry(optarg);
			break;
		case 'L':
			return plugin_list();
		case 'b':
			config.block_size = (size_t)get_offset(optarg);
			config.block = (unsigned char *)realloc(config.block, config.block_size);
			config_set_i("cfg.bsize", config.block_size);
			break;
		case 'V':
			printf("radare %s %dbit on %s%dbit "TARGET" %s%s\n", VERSION,
				sizeof(u64)*8, (LIL_ENDIAN)?"le":"be", sizeof(void *)*8, 
				(DEBUGGER)?   "dbg "   :"", (HAVE_GUI)? "gui"   :"");
			return 0;
		case 'u':
			config.unksize = 1;
			break;
		case 'x':
			config.mode = MODE_HEXDUMP;
			break;
		case 'e':
			config_eval(optarg);
			break;
		case 'h':
			help_message(0);
			return 0;
		case 'v':
			config_set("cfg.verbose", "false");
			break;
		default:
			return 1;
		}
	}

	if (optind < argc)
		config.file = argv[optind++];

	if (optind < argc)
		eprintf("warning: Only the first file has been opened.\n");

	/* TODO: we have to recheck this :) */
	if (flag_d) {
		optind--;
		config.debug = 1;
		prj = config_get("file.project");
		if (__optind==argc) {
			if (prj == NULL)
				return help_message(1);
			snprintf(buf2, 4095, "dbg://%s", project_get_file(prj) );
			config.file = strdup(buf2);
		} else {
			// XXX : overflowable, must use strcatdup or stgh like that
			// TODO: support hex pids too
			pid = atoi(argv[optind]);
			buf[0]='\0';

			/* by process-id */
			if (pid > 0) {
				sprintf(buf2, "pid://%d", pid);
				config.file = strdup(buf2);
			} else {
#if DEBUGGER
				/* by program path */
				for(c=optind;argv[c];c++) {
					ps.argv[c-optind] = argv[c];
					strcat(buf, argv[c]);
					if (argv[c+1])
						strcat(buf, " ");
				}
				ps.args = strdup(buf);
				if (strstr(buf, "://"))
					strcpy(buf2, buf);
				else sprintf(buf2, "dbg://%s", buf);
				config.file = strdup(buf2);
				ps.filename = strdup(buf2);
			//ptr = strchr(config.file, ' ');
			//if (ptr) *ptr = '\0';
			//config.file = strdup(buf2);
#else
				eprintf("TODO: Needs debugger\n");
#endif
			}
		}
	}

	plugin_load();

	return radare_go();
}
