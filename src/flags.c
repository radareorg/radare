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
// TODO: visual editor for flags

#include "main.h"
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
static int flag_ptr = -1;

static struct flags_spaces_t {
	const char *name;
} flag_spaces[255];

int flag_space_idx = -1;
int flag_space_idx2 = -1;
#define FLAG_SPACES 255

void flag_help()
{
	eprintf("Usage: f[?|d|-] [flag-name]\n"
	"  fortune      ; show fortune message! :D\n"
	"  fd           ; print flag delta offset\n"
	"  fc cmd       ; set command to be executed on flag at current seek\n"
	"  fg text      ; grep for all flags (like multiline f | grep foo)\n"
	"  fn name      ; flag new name (ignores dupped names)\n"
	"  fs spacename ; create/list/switch flag spaces\n"
	"  fr old new   ; rename a flag or more with '*'\n"
	"  f sym_main   ; flag current offset as sym_main\n"
	"  f foo @ 0x23 ; flag 0x23 offset as foo\n"
	"  f -sym_main  ; remove sym_main\n"
	"  f -*         ; remove all flags\n"
	"  f -sym_*     ; remove all flags starting with 'sym_'\n");
}

void flag_init()
{
	INIT_LIST_HEAD(&flags);
}

void flag_cmd(const char *text)
{
	flag_t *flag = flag_by_offset(config.seek);
	if (text == NULL || text[0] == '\0' || text[0]=='?') {
		cons_printf("Usage: fc <cmd> @ <offset>\n");
		cons_printf("   > fc pd 20 @ 0x8049104\n");
	} else
	if (flag != NULL) {
		free((void *)flag->cmd);
		flag->cmd = strdup(text);
		cons_printf("flag_cmd(%s) = '%s'\n", flag->name, text);
	}
}

// TODO: implement bubble sort with cache?
flag_t *flag_get_i(int id)
{
	struct list_head *pos;
	int i = 0;

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (i++ == id)
			return flag;
	}

	return NULL;
}

// TODO: USE GLOB OR SO...
void flag_grep(const char *grep) // TODO: add u64 arg to grep only certain address
{
	int i=0;
	struct list_head *pos;
	//const char *cmd_flag = config_get("cmd.flag");

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (config.interrupted) break;
		if (strstr(flag->name, grep)) {
			cons_printf("%03d 0x%08llx %3lld %s\n",
				i++, flag->offset, flag->length, flag->name);
			if (config_get("cmd.flag")) {
				u64 seek = config.seek;
				radare_seek(flag->offset, SEEK_SET);
				radare_cmd_raw(flag->cmd, 0);
				radare_seek(seek, SEEK_SET);
				NEWLINE;
			}
		}

	// TODO: use flags[i]->format over flags[i]->data and flags[i]->length
	}
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

flag_t *flag_get_next(int delta)
{
	flag_t *nice = NULL;
	struct list_head *pos;

	if (delta == 1) {
		list_for_each(pos, &flags) {
			flag_t *flag = list_entry(pos, flag_t, list);
			if (flag->offset > config.seek)  {
				if (nice) {
					if (flag->offset < nice->offset)
						nice = flag;
				} else {
					nice = flag;
				}
			}
		}
	} else { //if (delta == -1) {
		list_for_each(pos, &flags) {
			flag_t *flag = list_entry(pos, flag_t, list);
			if (flag->offset < config.seek)  {
				if (nice) {
					if (flag->offset > nice->offset)
						nice = flag;
				} else {
					nice = flag;
				}
			}
		}
	}
	return nice;
}
/*

	if (flag_ptr < 0) flag_ptr = 0;
	if (flag == NULL)
		flag_ptr = 0;
	flag_ptr += delta;

	return flag;
}
*/

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
		if (!_strnstr(foo+ini, flag->name, sz)) {
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
	int n = 0;
	char *arg = text?strchr(text, ' '):NULL;
	if (arg) {
		arg[0]='\0';
		n = flag_rename(text, arg+1);
		cons_printf("%d flags renamed\n", n);
		arg[0]=' ';
	} else {
		cons_printf("Usage: fr old-name new-name\n");
		cons_printf("> fr hit0_* hit_search\n");
	}
	return n;
}

/* deprecated ?!?! */
void flags_setenv()
{
	int i;
	char var[1024];
	char *ptr = environ[0];

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
	{
		struct list_head *pos;
		list_for_each(pos, &flags) {
			if (config.interrupted) break;
			flag_t *flag = list_entry(pos, flag_t, list);
			sprintf(var, "flag_%s", flag->name);
			sprintf(bar, OFF_FMT, flag->offset);
			setenv(var, bar, 1);
		}
	}
#endif
}

flag_t *flag_by_offset(u64 offset)
{
	struct list_head *pos;

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (flag->offset == offset)
			return flag;
		if (config.interrupted) break;
	}

	return NULL;
}

const char *flag_name_by_offset(u64 offset)
{
	struct list_head *pos;

	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (flag->offset == offset)
			return flag->name;
		if (config.interrupted) break;
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

		/* filter per flag spaces */
		if ((flag_space_idx != -1) && (flag->space != flag_space_idx))
			continue;

		cons_printf("%03d 0x%08llx %4lld %s",
			i++, flag->offset, flag->length, flag->name);
		NEWLINE;
	// TODO: use flags[i]->format over flags[i]->data and flags[i]->length
	}
}

void flag_clear_by_addr(u64 seek)
{
	struct list_head *pos;

	_polla:
	list_for_each(pos, &flags) {
		flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
		if (config.interrupted) break;
		if (flag->offset == seek) {
			list_del(&flag->list);
			free(flag);
			pos = flags.prev;
			goto _polla;
		}
	}
}

void flag_space_set(const char *name)
{
	int i;
	for(i=0;i<FLAG_SPACES;i++) {
		if (flag_spaces[i].name != NULL)
		if (!strcmp(name, flag_spaces[i].name)) {
			flag_space_idx = i;
			return;
		}
	}
	/* not found */
	for(i=0;i<FLAG_SPACES;i++) {
		if (flag_spaces[i].name == NULL) {
			flag_spaces[i].name = strdup(name);
			flag_space_idx = i;
			break;
		}
	}
}

void flag_space_cleanup()
{
	/* TODO: remove all flag spaces with 0 flags */
}

void flag_space_remove(const char *name)
{
	struct list_head *pos;
	int i;

	for(i=0;i<FLAG_SPACES;i++) {
		if (flag_spaces[i].name && ((!strcmp(name, flag_spaces[i].name)) || (name[0]=='*'))) {
			free((void *)flag_spaces[i].name);
			flag_spaces[i].name = NULL;
			if (i == flag_space_idx) {
				flag_space_idx = -1;
			}

			/* unlink related flags */
			list_for_each(pos, &flags) {
				flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
				if (config.interrupted) break;
				if (flag->space == i)
					flag->space = -1;
			}
			break;
		}
	}
}

void flag_space_list()
{
	int i,j = 0;
	for(i=0;i<FLAG_SPACES;i++) {
		if (flag_spaces[i].name) {
			printf("%02d %c %s\n", j++, (i==flag_space_idx)?'*':' ', flag_spaces[i].name);
		}
	}
	
}

void flag_space_init()
{
	static int init = 0;
	int i;
	if (init)
		return;
	init = 1;
	for(i=0;i<FLAG_SPACES;i++) {
		flag_spaces[i].name = NULL;
	}
}

void flag_space(const char *name)
{
	flag_space_init();

	switch(name[0]) {
	case '\0':
		flag_space_list();
		break;
	case '*':
		flag_space_idx = -1;
		break;
	case '-':
		flag_space_remove(name+1);
		break;
	case '?':
		cons_printf("Usage: fs [name]\n");
		cons_printf("  > fs regs     - create/switch to 'regs' flagspace\n");
		cons_printf("  > fs -regs    - remove 'regs' space\n");
		cons_printf("  > fs *        - select all spaces\n");
		cons_printf("  > fs          - list all flag spaces\n");
		break;
	default:
		flag_space_set(name);
		break;
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
		list_for_each(pos, &flags) {
			flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
			if (config.interrupted) break;
			if (!memcmp(str, flag->name, l)) {
				list_del(&(flag->list));
				free(flag);
				pos = flags.next;
				continue;
			}
		}
	} else {
		__restart2:
		list_for_each(pos, &flags) {
			flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
			if (config.interrupted) break;
			if (!strcmp(name, flag->name)) {
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

int flag_is_valid_name(const char *name)
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
			if (!flag_is_valid_name(name)) {
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
				if (f->offset == addr)
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
		memset(flag,'\0', sizeof(flag_t));
		list_add_tail(&(flag->list), &flags);
		if (flag==NULL)
			return 1;
	}

	strncpy(flag->name, name, FLAG_BSIZE);
	flag->name[FLAG_BSIZE-1]='\0';
	flag->offset = addr;
	flag->space = flag_space_idx;
	flag->length = config.block_size;
	flag->format = last_print_format;
	flag->cmd = NULL;
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

void flags_visual_menu()
{
	char cmd[1024];
	struct list_head *pos;
#define MAX_FORMAT 2
	int format = 0;
	char *fs = NULL;
	char *fs2 = NULL;
	int option = 0;
	int _option = 0;
	int delta = 7;
	int menu = 0;
	int i,j, ch;
	int hit;

	while(1) {
		cons_gotoxy(0,0);
		cons_clear();
		switch(menu) {
		case 0: // flag space
			cons_printf("\n Flag spaces:\n");
			hit = 0;
			for(j=i=0;i<FLAG_SPACES;i++) {
				if (flag_spaces[i].name) {
					if (option==i) {
						fs = flag_spaces[i].name;
						hit = 1;
					}
					cons_printf(" %c %02d %c %s\n", (option==i)?'>':' ', j++, (i==flag_space_idx)?'*':' ', flag_spaces[i].name);
				}
			}
			if (!hit) {
				option = j-1;
				continue;
			}
			break;
		case 1: // flag selection
			cons_printf("\n Flags in flagspace '%s'\n", fs);
			hit = 0;
			i = j = 0;
			list_for_each(pos, &flags) {
				flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
				/* filter per flag spaces */
				if ((flag_space_idx != -1) && (flag->space != flag_space_idx))
					continue;
				if (option==i) {
					fs2 = flag->name;
					hit = 1;
				}
				if( (i >=option-delta) && ((i<option+delta)||((option<delta)&&(i<(delta<<1))))) {
					cons_printf(" %c  %03d 0x%08llx %4lld %s\n",
						(option==i)?'>':' ',
						i, flag->offset, flag->length, flag->name);
					j++;
				}
				i++;
			}
			if (!hit) {
				option = i-1;
				continue;
			}
			cons_printf("\n Selected: %s\n", fs2);

			switch(format) {
			case 0: sprintf(cmd, "px @ %s", fs2); break;
			case 1: sprintf(cmd, "pd @ %s", fs2); break;
			case 2: sprintf(cmd, "pz @ %s", fs2); break;
			default: format = 0; continue;
			}
#if 0
			/* TODO: auto seek + print + disasm + string ...analyze stuff and proper print */
			cmd[0]='\0';
			if (strstr(fs2, "str_")) {
				sprintf(cmd, "pz @ %s", fs2);
			} else
			if (strstr(fs2, "sym_")) {
				sprintf(cmd, "pd @ %s", fs2);
			} else
				sprintf(cmd, "px @ %s", fs2);
#endif
			if (cmd[0])
				radare_cmd_raw(cmd, 0);
		}
		cons_flush();
		ch = cons_readchar();
		switch(ch) {
		case 'j':
			option++;
			break;
		case 'k':
			if (--option<0)
				option = 0;
			break;
		case 'h':
		case 'b': // back
			menu = 0;
			option = _option;
			break;
		case 'n':
			switch(menu) {
			case 0: // new flag space
				break;
			case 1: // new flag
				break;
			}
			break;
		case 'q':
			return;
		case '*':
		case '+':
			radare_set_block_size_i(config.block_size+1);
			break;
		case '/':
		case '-':
			radare_set_block_size_i(config.block_size-1);
			break;
		case 'P':
			if (--format<0)
				format = MAX_FORMAT;
			break;
		case 'p':
			format++;
			break;
		case 'l':
		case ' ':
		case '\n': // never happens
			if (menu == 1) {
				sprintf(cmd, "s %s", fs2);
				radare_cmd_raw(cmd, 0);
				return;
			}
			flag_space_set(fs);
			menu = 1;
			_option = option;
			option = 0;
			break;
		case '?':
			cons_printf(" j/k   - down/up keys\n");
			cons_printf(" h/b   - go back\n");
			cons_printf(" l/' ' - accept current selection\n");
			cons_printf(" n/d   - new/destroy flagspace or flag\n");
			cons_printf(" +/-   - increase/decrease block size\n");
			cons_printf(" p/P   - rotate print format\n");
			cons_printf(" :     - enter command\n");
			cons_flush();
			press_any_key();
			break;
		case ':':
			cons_set_raw(0);
#if HAVE_LIB_READLINE
			char *ptr = readline(VISUAL_PROMPT);
			if (ptr) {
				strncpy(cmd, ptr, sizeof(cmd));
				radare_cmd(cmd, 1);
				//commands_parse(line);
				free(ptr);
			}
#else
			cmd[0]='\0';
			dl_prompt = ":> ";
			if (cons_fgets(cmd, 1000, 0, NULL) <0)
				cmd[0]='\0';
			//line[strlen(line)-1]='\0';
			radare_cmd(cmd, 1);
#endif
			cons_set_raw(1);
			if (cmd[0])
				press_any_key();
			cons_gotoxy(0,0);
			cons_clear();
			continue;
		}
	}
}
