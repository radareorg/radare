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
#include "radare.h"
#include "flags.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *nullstr = "";
rad_flag_t **flags  = NULL;
unsigned int nflags = 0;
unsigned int eflags = 0;

void flag_help()
{
	eprintf("Usage: f[?|d|-] [flag-name]\n"
	"  fd           ; print flag delta offset\n"
	"  fn name      ; flag new name (ignores dupped names)\n"
	"  f sym_main   ; flag current offset as sym_main\n"
	"  f foo @ 0x23 ; flag 0x23 offset as foo\n"
	"  f -sym_main  ; remove sym_main\n"
	"  f -*         ; remove all flags\n"
	"  f -sym_*     ; remove all flags starting with 'sym_'\n");
}

// TODO: implement bubble sort with cache
rad_flag_t *flag_get_i(int id)
{
	int i,j = 0;

	if ( id<0 || id>=nflags )
		return NULL;

	for(i=0;i<nflags;i++) {
		if (flags[i]->name[0]=='\0')
			continue;
		if (j == id)
			return flags[id];
		j++;
	}

	return NULL;
}

rad_flag_t *flag_get(const char *name)
{
	register int i;

	if (name == NULL)
		return NULL;

	if (name[0]>='0'&& name[0]<='9')
		return NULL;

	for(i=0;i<nflags;i++)
		if (!strcmp(name, flags[i]->name))
			return flags[i];

	return NULL;
}

static int flag_ptr = -1;

rad_flag_t *flag_get_next(int delta)
{
	rad_flag_t *flag = flag_get_i(flag_ptr+delta);

	flag_ptr += delta;
	if (flag_ptr < 0) flag_ptr = 0;
	if (flag_ptr > nflags ) flag_ptr = nflags;

	return flag;
}

rad_flag_t *flag_get_reset()
{
	flag_ptr = 0;
	return flag_get_next(flag_ptr);
}

int flags_between(u64 from, u64 to)
{
	int i,n=0;
	for(i=0;i<nflags;i++) {
		if (flags[i]->offset > from && flags[i]->offset < to)
			n++;
	}
	return n;
}

/* deprecated ?!?! */
void flags_setenv()
{
	int i;
	char var[1024];
	char bar[1024];
	char *ptr = environ[0];

	for(i=0;(ptr = environ[i]);i++) {
		if (!memcmp("flag_", environ[i], 5)) {
			int len = strchr(environ[i],'=') - environ[i];
			if (len>0) {
				memset(var, '\0', 1024);
				memcpy(var, environ[i], len);
				unsetenv(var);
			}
		}
	}
	for(i=0; i<nflags; i++) {
		if (flag_is_empty(flags[i]))
			continue;
		sprintf(var, "flag_%s", flags[i]->name);
		sprintf(bar, OFF_FMT, flags[i]->offset);
		setenv(var, bar, 1);
	}
}

char *flag_name_by_offset(u64 offset)
{
	int i;
	rad_flag_t *flag;

	for(i=0; i<nflags; i++) {
		if (flags[i]->name[0]=='\0')
			continue;
		flag = flags[i];
		if (flag->offset == offset)
			return flag->name;
	}

	return nullstr;
}

int string_flag_offset(char *buf, unsigned long long seek)
{
	unsigned int i;
	int delta = (int)config_get_i("cfg.delta");
	rad_flag_t *ref = NULL;

	buf[0]='\0';

	for(i=0; i<nflags; i++) {
		if (config.interrupted) break;
		if (flags[i]->offset == seek) {
			ref = flags[i];
			break;
		}
		if (flag_is_empty(flags[i]))
			continue;
		if (!flags[i]->offset)
			continue;
		if (flags[i]->offset && flags[i]->offset <= seek && (!ref || flags[i]->offset>ref->offset)) {
			ref = flags[i];
		}
	}

	if (ref) {
		long ul = (seek-ref->offset);
		if (ul == 0)
			sprintf(buf, "%s", ref->name);
		else
		if (ul >-delta && ul<delta)
			sprintf(buf, "%s+0x%lx", ref->name, ul);
		return 1;
	}

	return 0;
}

void print_flag_offset(u64 seek)
{
	char buf[1024];

	if (string_flag_offset(buf, seek) )
		cons_printf("%s", buf);
}


void flag_do_empty(rad_flag_t *flag)
{
	if (flag == NULL)
		return;

	eflags--;
	flag->name[0]='\0';
}

int flag_is_empty(rad_flag_t *flag)
{
	if (flag == NULL || flag->name[0]=='\0')
		return 1;

	return 0;
}

void flag_list(char *arg)
{
	int i;
#if 0
	register int i, j;
	int maxlen = config.width/10;
	char *tmp;
#endif

	for(i=0; i<nflags; i++) {
		if (config.interrupted) break;
		if (flag_is_empty(flags[i]))
			continue;

		cons_printf("%03d 0x%08llx %3lld %s",
			i, flags[i]->offset, flags[i]->length, flags[i]->name);
		NEWLINE;

#if 0
		switch(flags[i]->format) {
		case FMT_OCT:
			cons_printf("o ");
			for(j=0;j<maxlen && j<flags[i]->length;j++)
				cons_printf("0%02o ", flags[i]->data[j]);
			if (flags[i]->length>6)
				cons_printf("..");

			cons_printf("\n");
			break;
		case FMT_INT:
			cons_printf("i ");
			for(j=0;j<maxlen && j<flags[i]->length;j++)
				cons_printf("%d ", flags[i]->data[j]);
			if (flags[i]->length>6)
				cons_printf("..");

			cons_printf("\n");
			break;
		case FMT_ASC:
			tmp = (char *)malloc(flags[i]->length+1);
			j = (config.width-55)/4;
			if (maxlen>j)
				maxlen=j;

			memcpy(tmp, flags[i]->data, flags[i]->length);
			for(j=0;j<maxlen&&j<flags[i]->length;j++) {
				if ( !is_printable(flags[i]->data[j]) )
					cons_printf("\\x%x", flags[i]->data[j]);
				else	cons_printf("%c", flags[i]->data[j]);
			}
			cons_printf("\n");
			free(tmp);
			break;
		case FMT_RAW:
			cons_printf("s ");
			fwrite(flags[i]->data, 1, flags[i]->length, stdout);
			cons_printf("\n");
			break;
		default:
			j = (config.width-55)/3;
			if (maxlen>j)
				maxlen=j;
			for(j=0;j<maxlen && j<flags[i]->length;j++)
				cons_printf(" %02x", flags[i]->data[j]);
			cons_printf("..\n");
		}
#endif
	}
}

void flag_clear_by_addr(u64 seek)
{
	register int i;

	for(i=0;i<nflags;i++)
		if (!flag_is_empty(flags[i])
		&& flags[i]->offset == seek) {
			flag_do_empty(flags[i]);
			break;
		}
}

void flag_clear(const char *name)
{
	char *str, *mask;
	register int i, l;
	int found = 0;

	if (name == NULL || *name == '\0')
		return;

	str = strdup(name); // TODO: strdup only when no mask is used
	mask = strchr(str, '*');

	if (mask) {
		mask[0]='\0';
		l = strlen(str);
		for(i = 0; i < nflags; i++)
			if(!flag_is_empty(flags[i])
			&& !memcmp(str, flags[i]->name, l)) {
				flag_do_empty(flags[i]);
				found = 1;
			}
	} else {
		for(i=0;i<nflags;i++)
			if (!flag_is_empty(flags[i])
			&& !strcmp(name, flags[i]->name)) {
				flag_do_empty(flags[i]);
				found = 1;
				break;
			}
	}

	if (!found)
		flag_clear_by_addr(get_math(str));

	free(str);
}

int flag_qsort_compare(const void *a, const void *b)
{
	if (a == NULL)
		return -1;
	if (b == NULL)
		return 1;
	return strcmp(b, a);
}

int flag_valid_name(const char *name)
{
	if (strchr(name, '*')  ||  strchr(name, '/') ||  strchr(name, '+')
	||  strchr(name, '-')  ||  strchr(name, ' ') ||  strchr(name, '\n')
	||  strchr(name, '\t') ||  ((name[0] >= '0') &&  (name[0] <= '9'))
	||  !is_printable(name[0]))
		return 0;
	return 1;
}

int flag_set(const char *name, u64 addr, int dup)
{
	register int i;
	const char *ptr;
	rad_flag_t *flag = NULL;

	if (!dup) {
		if (name[0]=='*' && name[1]=='\0') {
			for(i=0; i<nflags; i++) {
				if (flag_is_empty(flags[i]))
					continue;
			//	cons_printf("b 0x"OFF_FMTx"\n", flags[i]->length);
				cons_printf("f %s @ 0x"OFF_FMTx"\n",
					flags[i]->name, flags[i]->offset);
			}
			return 2;
		} else {
			if (!flag_valid_name(name)) {
				eprintf("invalid flag name '%s'.\n", name);
				return 2;
			}

			for (ptr = name + 1; *ptr != '\0'; ptr = ptr +1) {
				if (!is_printable(*ptr)) {
					eprintf("invalid flag name\n");
					return 2;
				}
			}
		}
	}

	for(i=0; i<nflags; i++) {
		if (flag_is_empty(flags[i])) {
			if (flag == NULL)
				flag = flags[i];
			continue;
		}
		if (!strcmp(flags[i]->name, name)) {
			if (dup) {
				/* ignore dupped name+offset */
				if (flags[i]->offset == addr)
					return 1;
			} else {
				flags[i]->offset = addr;
				flags[i]->length = config.block_size;
				flags[i]->format = last_print_format;
				memcpy(flags[i]->data, config.block, 
					(config.block_size>sizeof(flags[i]->data))?
					sizeof(flags[i]->data):config.block_size);
				return 1;
			}
		}
	}

	if (flag == NULL) {
		i = nflags++;
		flags = (rad_flag_t**)realloc(flags, nflags*sizeof(rad_flag_t*));
		flags[i] = malloc(sizeof(rad_flag_t));
		if (flags[i]==-1)
			return 1;
		flag = flags[i];
	}

	strncpy(flag->name, name, FLAG_BSIZE);
	eflags++;
	flag->offset = addr;
	flag->length = config.block_size;
	flag->format = last_print_format;
#if 0
	// XXX store data in flags is ugly!
	if (radare_read(0)!=-1)
	memcpy(flag->data, config.block, 
		(config.block_size>sizeof(flag->data))?
		sizeof(flag->data):config.block_size);
#endif

	// TODO qsort(flags, nflags, sizeof(rad_flag_t*), flag_qsort_compare);

	return 0;
}
