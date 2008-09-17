/*
 * Copyright (C) 2007, 2008
 *       pancake <@youterm.com>
 *       esteve <@pof.eslack.org>
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
#include "config.h"
#include "utils.h"
#include "flags.h"
#include "binparse.h"
#include <stdlib.h>
#include <signal.h>

static const char *search_cmdhit = NULL;
static int search_count = 0;
static int search_flag = 1;
static int search_verbose = 0;
char *search_last_keyword = NULL;

static int nhit = 0;

static void search_alarm()
{
	progressbar((int)(config.seek/config.size*100));  // slowdowns 170%
#if __UNIX__
	go_alarm(search_alarm);
#endif
}

static int hit_idx = 0;

void radare_search_seek_hit(int idx)
{
	flag_t *flag;
	u8 buf[64];

	sprintf(buf, "hit0_%d", hit_idx);
	flag = flag_get(buf);

	if (flag == NULL) {
		if (idx>0)
			hit_idx -=idx;
		else	hit_idx +=idx*2;
	} else radare_seek(flag->offset, SEEK_SET);

	hit_idx += idx;
	if (hit_idx<0) hit_idx=0;
}

// TODO: handle Control-C
void radare_search_aes()
{
	u64 oseek  = config.seek;
	u64 limit  = config.size;
	size_t bsize = config.block_size;
	int found = 0;
	int i;

	config.block_size = 512;

	if (config.limit)
		limit = config.limit;

	radare_controlc();
	while(!config.interrupted && (( limit != -1 && config.seek < limit) || limit == -1)) {
		radare_read(0);
		for(i=0;i<256;i++) {
			if (aes_key_test(config.block+i)) {
				printf("%03d 0x%08x\n", found, (uint)config.seek+i);
				fflush(stdout);
				found++;
			}
		}

		radare_seek(config.seek + 256,SEEK_SET);
	}
	eprintf("%d AES keys found\n",found );
	radare_controlc_end();

	config.seek = oseek;
	config.block_size = bsize;
}

int radare_search_asm(const char *str)
{
	eprintf("TODO\n");
#if 0
	u64 seek = config.seek;
	radare_seek(seek);
#endif
	return 0;
}

static int radare_tsearch_callback(struct _tokenizer *t, int i, u64 where)
{
	char flag_name[128];
	u64 off = config.seek;

	if (search_count && nhit >= search_count)
		return 1;

	sprintf(flag_name, "hit%d_%d", i, nhit++);
	//config.seek = where;

	radare_seek(where, SEEK_SET);
	radare_read(0);
	if (search_flag)
		flag_set(flag_name, where, 0);

	if (search_cmdhit && search_cmdhit[0]!='\0') {
		char *cmdhit = strdup(search_cmdhit);
		radare_seek(where, SEEK_SET);
		setenv("KEYWORD", search_last_keyword, 1); // XXX this is not last-keyword!! must array this!
		radare_cmd(cmdhit, 0);
		free(cmdhit);
		radare_controlc();
	}


	if (search_verbose) {
		u8 *ptr = config.block; //+(where-config.seek)-3;
		cons_printf("%03d  0x%08llx  %s ", nhit, where, flag_name);
		for(i=0;i<20;i++) {
			if (is_printable(ptr[i]))
				cons_printf("%c", ptr[i]);
		}
		cons_printf("\n");
	} 
	D {	fprintf(stderr, "\r%d", nhit); fflush(stderr); }

	fflush(stdout);
	config.seek = off;

	return 0;
}


int search_from_file(char *file)
{
	int i;
	u64 tmp = config.seek;
	tokenizer *t = binparse_new_from_file(file);

	if (t == NULL)
		return 0;

	nhit = 0;
#if __UNIX__
	D go_alarm(search_alarm);
#endif
	radare_controlc();
	for(radare_read(0);!config.interrupted;radare_read(1))
		for(i=0;i<config.block_size;i++)
			update_tlist(t, config.block[i], config.seek+i);

	radare_controlc_end();
#if __UNIX__
	D go_alarm(SIG_IGN);
#endif
	binparser_free(t);

	radare_seek(tmp, SEEK_SET);

	return 1;
}

int search_range(char *range)
{
	int len, i,j;
	char str[128];
	int num = -1, num2 = -1;
	tokenizer *t;
	u64 tmp = config.seek;
	u64 search_from;
	u64 search_to;
	u64 limit;
	int f0 = 0;
	u64 s;

	if (range == NULL)
		return 0;
	free(search_last_keyword);
	search_last_keyword = strdup(range);

	// init stuff
	search_cmdhit = config_get("cmd.hit");
	search_count = (int)config_get_i("cfg.count");
	search_flag = (int)config_get("search.flag");
	search_from = config_get_i("search.from");
	search_to = config_get_i("search.to");
	search_verbose = (int)config_get("search.verbose");

	nhit = 0;
	t = binparse_new();
	t->callback = &radare_tsearch_callback;
	len = strlen(range);
	// foreach token in range
	for(j=i=0;i<len;i++,j++) {
		str[j] = range[i];
		str[j+1] = '\0';
		switch(range[i+1]) {
		case '-':
			num = atoi(str);
			i++; j=-1;
			f0=1;
			break;
		case '\0':
		case ',':
			if (str[0]=='\0') break;
			num2 = atoi(str);
			if (f0) {
				f0=0;
				if (num == -1) {
					printf("syntax error\n");
					break;
				}
				for(j = num;j<=num2;j++)
					binparse_add_search(t, j);
			} else	binparse_add_search(t, num2);
			j=-1;
			str[0]='\0';
			i++;
			break;
		}
	}

	// twice
	radare_cmd("f -hit0_*", 0);
	radare_cmd("f -hit0_*", 0);
	radare_cmd("fs search", 0);
#if __UNIX__
	go_alarm(search_alarm);
#endif

	/* search loop */
	radare_controlc();
	if (search_from!=0)
		config.seek = search_from;
	limit = config.limit;
	if (search_to!=0)
		limit = search_to;
	for(i=1, radare_read(0); !config.interrupted; i = radare_read(1)) {
		s = config.seek;
		if (i==0) {
			eprintf("read err at 0x%08llx\n", config.seek);
			break;
		}
		if (limit && config.seek >= limit) break;
		if (config.debug && config.seek == 0xFFFFFFFF) break;
		for(i=0;i<config.block_size;i++) {
			if (update_tlist(t, config.block[i], config.seek+i)) {
				config.seek = s;
				radare_read(0);
			}
		}
		config.seek = s;
	}
	binparser_free(t);
#if __UNIX__
	go_alarm(SIG_IGN);
#endif
	if (config.interrupted) {
		printf("\nStopped at 0x"OFF_FMTx" (flag search_stop)\n", config.seek);
		flag_set("search_stop",config.seek, 0);
	}
	radare_controlc_end();

	radare_seek(tmp, SEEK_SET);
	if (!search_verbose)
		printf("\n");

	return 1;
}
