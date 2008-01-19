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

static void help_show_message()
{
	printf(
	"radare [options] [file]\n"
	"  -S [minlen]      extract strings inside file\n"
	"  -s [offset]      seek to the desired offset (cfg.seek)\n"
	"  -b [blocksize]   change the block size (512) (cfg.bsize)\n"
	"  -i [script]      interpret radare script\n"
	"  -P [project]     load metadata from project file\n"
//	"  -l [plugin.so]   link against a plugin (.so or .dll)\n"
	"  -e [key=val]     evaluates a configuration string\n"
	"  -f               set block size to fit file size\n"
	"  -L               list all available plugins\n"
	"  -c               same as -e cfg.color=true\n"
	"  -w               open file in read-write mode\n"
	"  -x               dump block in hexa and exit\n"
	"  -n               do not load ~/.radarerc and ./radarerc\n"
	"  -v               same as -e cfg.verbose=false\n"
	"  -V               show version information\n"
	"  -u               unknown size (no seek limits)\n"
	"  -h               this help message\n");
}

int main(int argc, char **argv, char **envp)
{
	int c, ret;
	char *ptr, ch;

	environ = envp;
	radare_init();
 
	while ((c = getopt(argc, argv, "fs:hb:wLvS:uVcnxi:e:P:")) != -1)
	{
		switch( c ) {
		case 'i':
			config.script = optarg;
			break;
		case 'f':
			config.block_size = 0;
			break;
		case 'c':
			config_set("cfg.color", "true");
			break;
		case 'n':
			config.noscript = 1;
			break;
		case 'P':
			config_set("file.project", optarg);
			break;
		case 'w':
			config_set("cfg.write", "true");
			break;
		case 's':
			config.seek = (off_t)get_offset(optarg);
			if (config.seek < 0) config.seek = (off_t)0;
			break;
#if 0
		case 'l':
			plugin_registry(optarg);
			break;
#endif
		case 'L':
			return plugin_list();
		case 'b':
			config.block_size = (size_t)get_offset(optarg);
			config.block = (unsigned char *)realloc(config.block, config.block_size);
			config_set_i("cfg.bsize", config.block_size);
			break;
		case 'V':
			printf("radare %s %dbit on %s%dbit "TARGET" %s%s%s%s\n", VERSION,
				sizeof(off_t)*8, (LIL_ENDIAN)?"le":"be", sizeof(void *)*8, 
				(HAVE_PERL)?  "perl "  :"",
				(HAVE_PYTHON)?"python ":"",
				(DEBUGGER)?   "dbg "   :"",
				(HAVE_VALAC)? "vala"   :"");
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
			help_show_message();
			return 0;
		case 'v':
			config_set("cfg.verbose", "false");
			break;
		case 'S':
			config.mode = MODE_STRINGS;
			config.ene  = atoi(optarg);
			break;
		default:
			return 1;
		}
	}

	if (optind < argc)
		config.file = argv[optind++];

	if (optind < argc)
		eprintf("warning: Only the first file has been opened.\n");

	if (config.mode == MODE_STRINGS)
		return stripstr_from_file(config.file, config.ene, (off_t)config.seek);

	return radare_go();
}
