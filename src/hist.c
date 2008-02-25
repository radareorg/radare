/*
 * Copyright (C) 2007
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
#include "search.h"
#include "plugin.h"
#include "utils.h"
#include "cmds.h"
#include "readline.h"
#include "flags.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK 4096

static unsigned int size = 0;
static char *hist = NULL;
static int lsize = 0;
static char *labels = NULL;
static int emulating = 0;
static int cpu_cmp = 0;
void hist_dump(char *file);
void hist_load(char *file);

int is_label(char *str)
{
	if (str[0]==';'&&str[strlen(str)-1]==':')
		return 1;
	return 0;
}

int label_get(char *name)
{
	int i, n;
	for(i=0;i<size;i++) {
		if (!strcmp(name, labels+i+4)) {
			memcpy(&n, labels+i, 4);
			return n;
		}
		i+=strlen(labels+i+4)+4;
	}
	return -1;
}

void label_show()
{
	int i, p, n=0;
	for(i=0;i<lsize;i++,n++) {
		memcpy(&p, labels+i, 4);
		printf("  %03d  %s\n", p, labels+i+4);
		i+=strlen(labels+i+4)+4;
	}
}

void hist_reset()
{
	lsize = 0;
	free(labels);
	labels = NULL;
	size = 0;
	free(hist);
	hist = NULL;
}

int hist_cmp(char *label)
{
	char *buf= strdup(label);
	char *ptr = strchr(buf, ',');
	if (ptr==NULL) {
		eprintf("Usage: ;cmp eax, eip\n");
		return 0;
	}
	ptr[0]='\0';
	ptr = ptr + 1;
	
	cpu_cmp = get_cmp(buf, ptr);
	
	//cons_printf("%d\n", cpu_cmp);
	
	return 1;
}

int hist_set(char *label)
{
	char *buf= strdup(label);
	char *ptr = strchr(buf, ',');
	if (ptr==NULL) {
		eprintf("Usage: ;cmp eax, eip\n");
		return 0;
	}
	ptr[0]='\0';
	ptr = ptr + 1;
	
	flag_set(buf, get_math(ptr), 0);
	
	return 1;
}

int hist_get(char *label)
{
	flag_t *flag = flag_get(label);
	if (flag) {
		cons_printf("0x%x\n", flag->offset);
	} else  cons_printf("(uh?)\n");
	return 0;
}

int hist_goto(char *label)
{
	int n, i = label_get(label);
	if (i == -1) {
		eprintf("Label %s not found\n", label);
		return 0;
	}

	emulating = 1;
	for(;i<size;i++,n++) {
		if (!strstr(hist+i, "core.break"))
			break;
		radare_cmd(hist+i, 0);
		i+=strlen(hist+i);
	}
	emulating = 0;

	return 1;
}

int hist_cgoto(char *label, int type)
{
	int i = label_get(label);
	if (i == -1) {
		eprintf("Label %s not found\n", label);
		return 0;
	}

	switch(type) {
	case OP_JE: if (cpu_cmp != 0) return 0; break;
	case OP_JNE: if (cpu_cmp == 0) return 0; break;
	case OP_JA: if (cpu_cmp <= 0) return 0; break;
	case OP_JB: if (cpu_cmp >= 0) return 0; break;
	default: eprintf("unknown conditional opcode\n"); break;
	}

	hist_goto(label);

	return 1;
}

int hist_loop(char *label)
{
	char *cmd;
	int i, times = atoi(label);

	cmd = strchr(label, ',');
	if (cmd == NULL) {
		eprintf("Usage: ;loop 3 label\n");
		return 0;
	}

	i = label_get(++cmd);
	if (i == -1) {
		eprintf("Label %s not found\n", cmd);
		return 0;
	}
	for(i=0;i<times;i++)
		hist_goto(cmd);

	return 1;
}

void comment_help()
{
	cons_printf(
	" ;set eax, 0x33 - sets a value for a flag\n"
	" ;get eax       - sets a value for a flag\n");
}

void hist_add_label(char *str) {
	int len = 2;
	memset(labels+lsize+4, '\0', BLOCK-((lsize+len+4)%BLOCK));
	memcpy(labels+lsize, &size, 4);
	memcpy(labels+lsize+4, str+1, len-2);
	lsize+=len+1+2;
}

void hist_add(char *str, int log)
{
	int len, found;


	if (str == NULL || str[0]=='\0')
		return;
#if HAVE_LIB_READLINE
	if (log)
		add_history(str);
#endif
	/* disabled */
	return;

	if (hist==NULL) {
		hist = malloc(BLOCK);
		hist[0]='\0';
	}
	str = strclean(str);
	len = strlen(str)+1;

	if (labels==NULL)
		labels = malloc(BLOCK);
	else if (((lsize+len)%BLOCK)==(BLOCK-1))
		labels = realloc(labels, BLOCK+lsize);

	/* closures to avoid execution */
	if (str[1] == '{') { config.skip = 1; return; }
	if (str[1] == '}') { config.skip = 0; return; }

	if (!memcmp(str,";get ", 5))  // ==
		hist_get(str+5);
	else
	if (!memcmp(str,";set ", 5))  // ==
		hist_set(str+5);
	else
		found = 0;
#if 0
	if (!strcmp(str,"!h"))
		return;
	if (!strcmp(str,"!l"))
		return;
	if (!strcmp(str,"!~"))
		return;
#endif
	if (emulating)
		return;

#if 0
	for(i=0,found=0;!found&&i<size;i++) {
		found = strcmp(hist+i, str)?1:0;
		i += strlen(hist+i);
	}

	if (found)
		return;
#endif


	if (((size%BLOCK)+len)>=BLOCK)
		hist = realloc(hist, BLOCK+size+len+1);
	strcat(hist+size, str);
	size+=len;
}

void hist_dump(char *file)
{
	FILE *fd = fopen(file, "w");
	int i, n=0;

	if (fd == NULL) {
		eprintf("hist_dump: Cannot open '%s' for writing\n", file);
		return;
	}

	for(i=0;i<size;i++,n++) {
		fprintf(fd, "%s\n", hist+i);
		i += strlen(hist+i);
	}

	fclose(fd);
}

void hist_load(char *file)
{
	FILE *fd = fopen(file, "r");
	int i, n=0;

	if (fd == NULL) {
		eprintf("Cannot open '%s' for reading\n", file);
		return;
	}

	hist_reset();
	for(i=0;i<size;i++,n++) {
		hist_add(hist+i, 0);
		i += strlen(hist+i);
	}

	fclose(fd);
}

char *hist_get_i(int p)
{
	int i, n=0;
	for(i=0;i<size;i++,n++) {
		if (n==p)
			return hist+i;
		i+=strlen(hist+i);
	}
	return NULL;
}

int hist_show()
{
	int i, n=0;
	for(i=0;i<size;i++,n++) {
		printf("  %3d  %s\n", n, hist+i);
		i += strlen(hist+i);
	}
	return n;
}
