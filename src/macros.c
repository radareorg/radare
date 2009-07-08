/*
 * Copyright (C) 2008
 *			 pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <stdio.h>
#include "main.h"
#include "macros.h"

static ut64 _macro_break_value = 0;
ut64 *macro_break_value = NULL;
static int macro_break;
int macro_counter = 0;
static struct list_head macros;

void radare_macro_init()
{
	INIT_LIST_HEAD(&macros);
}

// XXX add support single line function definitions
// XXX add support for single name multiple nargs macros
int radare_macro_add(const char *oname)
{
	struct list_head *pos;
	struct macro_t *macro;
	char buf[1024];
	char *bufp;
	char *pbody;
	char *ptr;
	int lidx;
	int macro_update;
	char *name;

	if (oname[0]=='\0')
		return radare_macro_list();

	name = alloca(strlen(oname)+1);
	strcpy(name, oname);

	pbody = strchr(name, ',');
	if (pbody) {
		pbody[0]='\0';
		pbody = pbody + 1;
	}

	if (name[strlen(name)-1]==')') {
		//eprintf("No body?\n");
		return -1;
	}

	macro = NULL;
	macro_update = 0;
	list_for_each_prev(pos, &macros) {
		struct macro_t *mac = list_entry(pos, struct macro_t, list);
		if (!strcmp(name, mac->name)) {
			macro = mac;
			free(macro->name);
			free(macro->code);
			macro_update = 1;
			break;
		}
	}
	if (macro == NULL)
		macro = (struct macro_t *)malloc(sizeof(struct macro_t));
	macro->name = strdup(name);
	if (pbody) macro->code = (char *)malloc(strlen(pbody)+2);
	else macro->code = (char *)malloc(1024);
	macro->code[0]='\0';
	macro->nargs = 0;
	ptr = strchr(macro->name, ' ');

	if (ptr != NULL) {
		*ptr='\0';
		macro->nargs = set0word(ptr+1);
	}

	if (pbody) {
		for(lidx=0;pbody[lidx];lidx++) {
			if (pbody[lidx]==',')
				pbody[lidx]='\n';
			else
			if (pbody[lidx]==')' && pbody[lidx-1]=='\n')
				pbody[lidx]='\0';
		}
		strcpy(macro->code, pbody);
	} else {
		while(1) {
			if (stdin == cons_stdin_fd) {
				printf(".. ");
				fflush(stdout);
			}
			fgets(buf, 1023, cons_stdin_fd);
			if (buf[0]==')')
				break;
			for(bufp=buf;*bufp==' '||*bufp=='\t';bufp=bufp+1);
			lidx = strlen(buf)-2;
			if (buf[lidx]==')' && buf[lidx-1]!='(') {
				buf[lidx]='\0';
				strcat(macro->code, bufp);
				break;
			}
			if (buf[0] != '\n')
				strcat(macro->code, bufp);
		}
	}
	if (macro_update == 0)
		list_add_tail(&(macro->list), &(macros));
	
	return 0;
}

int radare_macro_rm(const char *_name)
{
	char *name = alloca(strlen(_name));
	struct list_head *pos;
	char *ptr = strchr(name, ')');
	if (ptr) *ptr='\0';
	list_for_each_prev(pos, &macros) {
		struct macro_t *mac = list_entry(pos, struct macro_t, list);
		if (!strcmp(mac->name, name)) {
			free(mac->name);
			free(mac->code);
			list_del(&(mac->list));
			free(mac);
			eprintf("Macro '%s' removed.\n", name);
			return 1;
		}
	}
	return 0;
}

int radare_macro_list()
{
	int j, idx = 0;
	struct list_head *pos;
	list_for_each_prev(pos, &macros) {
		struct macro_t *mac = list_entry(pos, struct macro_t, list);
		cons_printf("%d (%s, ", idx, mac->name);
		for(j=0;mac->code[j];j++) {
			if (mac->code[j]=='\n')
				cons_printf(", ");
			else cons_printf("%c", mac->code[j]);
		}
		cons_strcat(")\n");
		idx++;
	}
	return 0;
}
#if 0
(define name value
  f $0 @ $1)

(define loop cmd
  loop:
  
  ? $0 == 0
  ?? .loop:
  )

.(define patata 3)
#endif

int radare_cmd_args(const char *ptr, const char *args, int nargs)
{
	int i,j;
	char *cmd = alloca(strlen(ptr)+1024);
	cmd[0]='\0';

//	eprintf("call(%s)\n", ptr);
	for(i=j=0;ptr[j];i++,j++) {
		if (ptr[j]=='$') {
			if (ptr[j+1]>='0' && ptr[j+1]<='9') {
				const char *word = get0word(args, ptr[j+1]-'0');
				strcat(cmd, word);
				j++;
				i = strlen(cmd)-1;
			} else
			if (ptr[j+1]=='@') {
				char off[32];
				sprintf(off, "%d", macro_counter);
				strcat(cmd, off);
				j++;
				i = strlen(cmd)-1;
			} else {
				cmd[i]=ptr[j];
				cmd[i+1]='\0';
			}
		} else {
			cmd[i]=ptr[j];
			cmd[i+1]='\0';
		}
	}
	while(*cmd==' '||*cmd=='\t')
		cmd = cmd + 1;
	if (*cmd==')')
		return 0;
	//eprintf("cmd(%s)\n", cmd);
	return radare_cmd(cmd, 0);
}

char *macro_label_process(struct macro_label_t *labels, int *labels_n, char *ptr)
{
	int i;
	for(;ptr[0]==' ';ptr=ptr+1);

	if (ptr[strlen(ptr)-1]==':') {
		/* label detected */
		if (ptr[0]=='.') {
		//	eprintf("---> GOTO '%s'\n", ptr+1);
			/* goto */
			for(i=0;i<*labels_n;i++) {
		//	eprintf("---| chk '%s'\n", labels[i].name);
				if (!strcmp(ptr+1, labels[i].name))
					return labels[i].ptr;
			}
			return NULL;
		} else
		/* conditional goto */
		if (ptr[0]=='?' && ptr[1]=='!' && ptr[2] != '?') {
			if (config.last_cmp != 0) {
				char *label = ptr + 3;
				for(;label[0]==' '||label[0]=='.';label=label+1);
		//		eprintf("===> GOTO %s\n", label);
				/* goto label ptr+3 */
				for(i=0;i<*labels_n;i++) {
					if (!strcmp(label, labels[i].name))
						return labels[i].ptr;
				}
				return NULL;
			}
		} else
		/* conditional goto */
		if (ptr[0]=='?' && ptr[1]=='?' && ptr[2] != '?') {
			if (config.last_cmp == 0) {
				char *label = ptr + 3;
				for(;label[0]==' '||label[0]=='.';label=label+1);
		//		eprintf("===> GOTO %s\n", label);
				/* goto label ptr+3 */
				for(i=0;i<*labels_n;i++) {
					if (!strcmp(label, labels[i].name))
						return labels[i].ptr;
				}
				return NULL;
			}
		} else {
			for(i=0;i<*labels_n;i++) {
		//	eprintf("---| chk '%s'\n", labels[i].name);
				if (!strcmp(ptr+1, labels[i].name)) {
					i = 0;
					break;
				}
			}
			/* Add label */
		//	eprintf("===> ADD LABEL(%s)\n", ptr);
			if (i == 0) {
				strncpy(labels[*labels_n].name, ptr, 64);
				labels[*labels_n].ptr = ptr+strlen(ptr)+1;
				*labels_n = *labels_n + 1;
			}
		}
		return ptr + strlen(ptr)+1;
	}

	return ptr;
}

/* TODO: add support for spaced arguments */
int radare_macro_call(const char *name)
{
	char *args;
	int nargs = 0;
	char *str, *ptr, *ptr2;
	struct list_head *pos;
	static int macro_level = 0;
	/* labels */
	int labels_n = 0;
	struct macro_label_t labels[MAX_LABELS];

	str = alloca(strlen(name)+1);
	strcpy(str, name);
	ptr = strchr(str, ')');
	if (ptr) *ptr='\0';
	args = strchr(str, ' ');
	if (args) {
		*args='\0';
		args = args +1;
		nargs = set0word(args);
	}

	macro_level ++;
	if (macro_level > MACRO_LIMIT) {
		eprintf("Maximum macro recursivity reached.\n");
		macro_level --;
		return 0;
	}

	if (ptr != NULL) *ptr='\0';
	list_for_each_prev(pos, &macros) {
		struct macro_t *mac = list_entry(pos, struct macro_t, list);

		if (!strcmp(str, mac->name)) {
			char *ptr = mac->code;
			char *end = strchr(ptr, '\n');

			if (mac->nargs != 0 && nargs != mac->nargs) {
				eprintf("Macro '%s' expects %d args. (not %d) (%s).\n",
					mac->name, mac->nargs, nargs, name);
				macro_level --;
				return 0;
			}

			macro_break = 0;
			do {
				if (end) *end='\0';

				/* Label handling */
				ptr2 = macro_label_process(&(labels[0]), &labels_n, ptr);
				if (ptr2 == NULL) {
					eprintf("Oops. invalid label name\n");
					break;
				} else
				if (ptr != ptr2 && end) {
					*end='\n';
					ptr = ptr2;
					end = strchr(ptr, '\n');
					continue;
				}

				/* Command execution */
				if (*ptr) {
					radare_cmd_args(ptr, args, nargs);
				}
				if (end) {
					*end='\n';
					ptr = end + 1;
				} else {
					macro_level --;
					return 1;
				}

				/* Fetch next command */
				end = strchr(ptr, '\n');
			} while(!macro_break);

			if (macro_break) {
				macro_level --;
				return 0;
			}
		}
	}
	eprintf("No macro named '%s'\n", str);
	macro_level --;
	return 0;
}

int radare_macro_break(const char *value)
{
	macro_break = 1;
	macro_break_value = NULL;
	_macro_break_value = 0LL;
	_macro_break_value = (ut64)get_math(value);
	if (value && *value) {
		macro_break_value = &_macro_break_value;
	} 
}
