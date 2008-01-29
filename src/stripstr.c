/*
 * Copyright (C) 2006, 2007, 2008
 *       pancake <@youterm.com>
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
#include "flags.h"
#include "radare.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include "utils.h"
#if __UNIX__
#include <sys/mman.h>
#endif

enum {
	ENCODING_ASCII = 0,
	ENCODING_CP850 = 1
};

static char *encodings[3] = { "ascii", "cp850", NULL };
static int encoding = ENCODING_ASCII; // default
static int working = 0;
static int min;

static int resolve_encoding(const char *name)
{
	int i;

	if (name != NULL)
		for(i=0;encodings[i];i++)
			if (!strcasecmp(name, encodings[i]))
				return i;

	return ENCODING_ASCII;
}

static int is_encoded(unsigned char c)
{
	switch(encoding) {
	case ENCODING_ASCII:
		break;
	case ENCODING_CP850:
		switch(c) {
		// CP850
		case 128: // cedilla
		case 133: // a grave
		case 135: // minicedilla
		case 160: // a acute
		case 161: // i acute
		case 129: // u dieresi
		case 130: // e acute
		case 139: // i dieresi
		case 162: // o acute
		case 163: // u acute
		case 164: // enye
		case 165: // enyemay
		case 181: // A acute
		case 144: // E acute
		case 214: // I acute
		case 224: // O acute
		case 233: // U acute
			return 1;
		}
		break;
	}
	return 0;
}

int stripstr_iterate(const unsigned char *buf, int i, off_t offset, char *match)
{
	rad_flag_t *flag;
	static int unicode = 0;
	static int matches = 0;
	char str[4096];

	if (match&&match[0]=='\0')
		match=NULL;

	if (is_printable(buf[i]) || (config.color&&is_encoded(buf[i]))) {
		if (matches == 0)
			offset += i;
		str[matches] = buf[i];
		if (matches < sizeof(str))
			matches++;
	} else {
		/* wide char check \x??\x00\x??\x00 */
		if (matches && buf[i+2]=='\0' && buf[i]=='\0' && buf[i+1]!='\0') {
			unicode = 1;
			return 1; // unicode
		}
		/* check if the length fits on our request */
		if (matches >= min) {
			str[matches] = '\0';
			// XXX Support for 32 and 64 bits here
			if (match && match[0]=='*' && match[1]=='\0') {
				int i,len = strlen(str);
				char msg[32];
				if (len>20) len = 20;
				strcpy(msg, "str_");
				memcpy(msg+4, str, len);
				str[4+len]='\0';
				for(i=4;i<len+4;i++) {
					switch(msg[i]) {
					case ' ':
					case '@':
					case '%':
					case '#':
					case '!':
					case ':':
					case '"':
					case '&':
					case '>':
					case '<':
					case ';':
					case '`':
					case '\'':
						msg[i]='_';
					}
				}

				// XXX THIS IS UGLY AS SHIT
				do {
					flag = flag_get(msg);
					if (flag && flag->offset != (offset-matches)) {
						strcat(msg, "0");
					} else break;
				} while(1);

				cons_printf("f %s @ 0x%08x\n", msg, (unsigned int)offset-matches);
			} else
			if ((!match) || (match && strstr(str, match)) ){
				D cons_printf("0x%08x %c %s\n", (unsigned int)offset-matches, (unicode)?'U':'A', str);
				else cons_printf("%s\n", str);
				cons_flush();
			}
		}
		matches = 0;
		unicode = 0;
	}
	return 0;
}

int radare_strsearch(char *str)
{
	off_t i;
	int j, ret;
	off_t seek = config.seek;
	off_t size = config.size;

	min = 5;

	if (str) str=str+1;
	if (size <=0)
		size=0xbfffffff;

	radare_controlc();
	for(i = (size_t)seek; working && config.seek < size; i++) {
		ret = radare_read(1);
		if (ret == -1) break;
		for(j=0;j<config.block_size;j++)
			stripstr_iterate(config.block, j, config.seek+j, str);
	}
	config.seek = seek;
	radare_controlc_end();
	return 0;
}

int stripstr_from_file(const char *filename, int min, off_t seek)
{
	int i, fd = open(filename, O_RDONLY);
	unsigned char *buf;
	size_t len;

	if (fd == -1) {
		eprintf("Cannot open target file.\n");
		return 1;
	}

	len = lseek(fd, 0, SEEK_END);

	encoding = resolve_encoding(config_get("cfg.encoding"));

#if __UNIX__
	buf = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
	if (((int)buf) == -1 ) {
		perror("mmap");
		return 1;
	}
	if (min == 0)
		min = 5;

	for(i = (size_t)seek; i < len; i++)
		stripstr_iterate(buf, i, i, NULL);
	
	munmap(buf, len); 
#endif
#if __WINDOWS__
	eprintf("Not yet implemented\n");
#endif
	close(fd);

	return 0;
}
