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

// TODO: automatic bubble sort by addr - faster indexing
// TODO: support cursor indexing (index inside flag list)
// TODO: store data and show (flag.data) in last_print_format

#include "radare.h"
#include "flags.h"
#include "utils.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *nullstr = "";
struct list_head flags;

void flag_help()
{
	eprintf("Usage: f[?|d|-] [flag-name]\n"
	"  fortune      ; show fortune message! :D\n"
	"  fd           ; print flag delta offset\n"
	"  fn name      ; flag new name (ignores dupped names)\n"
	"  fr old new   ; rename a flag or more with '*'\n"
	"  f sym_main   ; flag current offset as sym_main\n"
	"  f foo @ 0x23 ; flag 0x23 offset as foo\n"
	"  f -sym_main  ; remove sym_main\n"
	"  f -*         ; remove all flags\n"
	"  f -sym_*     ; remove all flags starting with 'sym_'\n");
}

void flags_init()
{
	INIT_LIST_HEAD(&flags);
}

// TODO: implement bubble sort with cache?
flag_t *flag_get_i(int id)
{
	struct list_head *pos;
	int i = 0;

/*
	if ( id<0 || id>=nflags )
		return NULL;
*/
	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (i++ == id)
			return flag;
	}

	return NULL;
}

flag_t *flag_get(const char *name)
{
	struct list_head *pos;
	if (name==NULL || (name[0]>='0'&& name[0]<='9'))
		return NULL;
	list_for_each_prev(pos, &flags) {
		flag_t *flag = list_entry(pos, flag_t, list);
		if (!strcmp(name, flag->name))
			return flag;
	}
	return NULL;
}

u64 flag_get_addr(const char *name)
{
	flag_t *foo = flag_get(name);
	if (foo)
		return foo->offset;
	return 0;
}


static int flag_ptr = -1;

flag_t *flag_get_next(int delta)
{
	flag_t *flag = flag_get_i(flag_ptr+delta);

	flag_ptr += 1;
	if (flag_ptr < 0) flag_ptr = 0;
	if (flag == NULL)
		flag_ptr = 0;

	return flag;
}

flag_t *flag_get_reset()
{
	flag_ptr = 0;
	return flag_get_next(flag_ptr);
}

// TODO : use flag size
int flags_between(u64 from, u64 to)
{
	int n=0;
	struct list_head *pos;
	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (flag->offset >= from && flag->offset <= to)
			n++;
	}
	return n;
}

int strnstr(unsigned char *from, unsigned char *to, int size)
{
	int i;
	for(i=0;i<size;i++)
		if (from==NULL||to==NULL||from[i]!=to[i])
			break;
	return (size!=i);
}

int flag_rename(char *foo, char *bar)
{
	int n = 0;
	int ini = 0;
	int end = strlen(foo);
	int sz = end-ini;
	int glob_end = 0;
	struct list_head *pos;

	if (foo[0]=='*')
		ini = 1;
	if (foo[end-1]=='*') {
		glob_end = 1;
		foo[end]='\0';
		sz--;
		end --;
	}

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (ini) {
				char *str = strstr(flag->name, foo+ini);
				if (str) {
					str = strdup(str);
					sprintf(flag->name, "%s%s", bar, str);
					free(str);
				}
		} else
		if (!strnstr(foo+ini, flag->name, sz)) {
			if (glob_end) {
				char *str = strdup(flag->name+sz);
				sprintf(flag->name, "%s%s", bar, str);
				free(str);
			} else {
				if (flag->name[end]=='\0')
					strcpy(flag->name, bar);
			}
			n++;
		}
	}
	return n;
}

int flag_rename_str(char *text)
{
	char *arg = text?strchr(text, ' '):NULL;
	if (arg) {
		arg[0]='\0';
		cons_printf("Renameing %s\n", text);
		flag_rename(text, arg+1);
		arg[0]=' ';
	} else {
		cons_printf("Usage: fr old-name new-name\n");
		cons_printf("> fr hit0_* hit_search\n");
	}
}

/* deprecated ?!?! */
void flags_setenv()
{
	int i;
	char var[1024];
	char *ptr = environ[0];
	struct list_head *pos;

	for(i=0;(ptr = environ[i]);i++) {
		if (config.interrupted) break;
		if (!memcmp("flag_", environ[i], 5)) {
			int len = strchr(environ[i],'=') - environ[i];
			if (len>0) {
				memset(var, '\0', 1024);
				memcpy(var, environ[i], len);
				unsetenv(var);
			}
		}
	}

#if 0
	list_for_each(pos, &flags) {
		if (config.interrupted) break;
		flag_t *flag = list_entry(pos, flag_t, list);
		sprintf(var, "flag_%s", flag->name);
		sprintf(bar, OFF_FMT, flag->offset);
		setenv(var, bar, 1);
	}
#endif
}

char *flag_name_by_offset(u64 offset)
{
	struct list_head *pos;

	list_for_each(pos, &flags) {
		if (config.interrupted) break;
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (flag->offset == offset)
			return flag->name;
	}
	return nullstr;
}

int string_flag_offset(char *buf, u64 seek)
{
	int delta = (int)config_get_i("cfg.delta");
	flag_t *ref = NULL;
	struct list_head *pos;

	buf[0]='\0';

	list_for_each(pos, &flags) {
		if (config.interrupted) break;
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (flag->offset == seek) {
			ref = flag;
			break;
		}
		if (!flag->offset)
			continue;
		if (flag->offset && flag->offset <= seek
		&& (!ref || flag->offset>ref->offset))
			ref = flag;
	}

	if (ref) {
		long ul = (seek-ref->offset);
		if (ul == 0)
			sprintf(buf, "%s", ref->name);
		else
		if (ul >-delta && ul<delta)
			sprintf(buf, "%s+0x%lx", ref->name, ul);
		else
			return 0;
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


void flag_do_empty(flag_t *flag)
{
	if (flag == NULL)
		return;

	flag->name[0]='\0';
}

int flag_is_empty(flag_t *flag)
{
	if (flag == NULL || flag->name[0]=='\0')
		return 1;

	return 0;
}

void flag_list(char *arg)
{
	int i=0;
	struct list_head *pos;

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (config.interrupted) break;

		cons_printf("%03d 0x%08llx %3lld %s",
			i++, flag->offset, flag->length, flag->name);
		NEWLINE;
	// TODO: use flags[i]->format over flags[i]->data and flags[i]->length
	}
}

void flag_clear_by_addr(u64 seek)
{
	struct list_head *pos;

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (config.interrupted) break;
		if (flag->offset == seek) {
			list_del(&flag->list);
			pos = flags.next;
		}
	}
}

void flag_clear(const char *name)
{
	struct list_head *pos;
	char *str, *mask;
	int l;
	int found = 0;

	if (name == NULL || *name == '\0')
		return;

	str = strdup(name); // TODO: strdup only when no mask is used
	mask = strchr(str, '*');

	if (mask) {
		mask[0]='\0';
		l = strlen(str);
		__restart:
		list_for_each(pos, &flags) {
			flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
			if (config.interrupted) break;
			if (!memcmp(str, flag->name, l)) {
				list_del(&(flag->list));
				free(flag);
				found = 1;
				goto __restart;
			}
		}
	} else {
		__restart2:
		list_for_each(pos, &flags) {
			flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
			if (config.interrupted) break;
			if (strcmp(name, flag->name)) {
				list_del(&(flag->list));
				free(flag);
				found = 1;
				goto __restart2;
			}
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
	const char *ptr;
	flag_t *flag = NULL;
	struct list_head *pos;

	if (!dup) {
		/* show flags as radare commands */
		if (name[0]=='*' && name[1]=='\0') {
			list_for_each(pos, &flags) {
				flag_t *f = (flag_t *)list_entry(pos, flag_t, list);
				if (config.interrupted) break;
				cons_printf("f %s @ 0x"OFF_FMTx"\n", f->name, f->offset);
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

	list_for_each(pos, &flags) {
		flag_t *f = (flag_t *)list_entry(pos, flag_t, list);
		if (config.interrupted) break;
		if (!strcmp(f->name, name)) {
			if (dup) {
				/* ignore dupped name+offset */
				if (flag->offset == addr)
					return 1;
			} else {
				flag = f;
				f->offset = addr;
				f->length = config.block_size;
				f->format = last_print_format;
/*
				memcpy(f->data, config.block, 
					(config.block_size>sizeof(flags[i]->data))?
					sizeof(f->data):config.block_size);

*/
				return 1;
			}
		}
	}

	if (flag == NULL) {
		flag = malloc(sizeof(flag_t));
		list_add_tail(&(flag->list), &flags);
		if (flag==NULL)
			return 1;
	}

	strncpy(flag->name, name, FLAG_BSIZE);
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

	// TODO qsort(flags, nflags, sizeof(flag_t*), flag_qsort_compare);

	return 0;
}
