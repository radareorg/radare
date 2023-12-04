/*
 * Copyright (C) 2008, 2009
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
#include "code.h"
#include "data.h"
#include "undo.h"
#include "flags.h"
#include "arch/csr/dis.h"
#include "arch/arm/disarm.h"
#if NONFREE
#include "arch/ppc/ppc_disasm.h"
#endif
#include "arch/m68k/m68k_disasm.h"
#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"
#include "list.h"

struct reflines_t *reflines = NULL;

static struct list_head vartypes;
static struct list_head data;
static struct list_head comments;
static struct list_head xrefs;

ut64 var_functions_show(int idx)
{
	const char *mark = nullstr;
	int i = 0;
	struct list_head *pos;
	ut64 seek = config.seek;
	ut64 addr = config.seek;
	int window = 15;
	int wdelta = (idx>5)?idx-5:0;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == DATA_FUN) {
			if (i>=wdelta) {
				if (i> window+wdelta) {
					cons_printf("   ...\n");
					break;
				}
				if (seek > d->from+config.vaddr && seek < d->to+config.vaddr)
					mark = "<SEEK IS HERE>";
				else mark = nullstr;
				if (idx == i) {
					addr = d->from;
					cons_printf(" * ");
				} else cons_printf("   ");
				cons_printf("0x%08llx (%s) %s\n", d->from+config.vaddr,
					flag_name_by_offset(d->from), mark);
			}
			i++;
		}
	}
	return addr;
}

int data_set_len(ut64 off, ut64 len)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off<= d->to) {
			d->to = d->from+len;
			d->size = d->to-d->from+1;
			return 0;
		}
	}
	return -1;
}

ut64 data_prev(ut64 off, int type)
{
	struct list_head *pos;
	ut64 ret = 0;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == type) {
			if (d->from <= off && d->to >= off)
				ret = d->from;
		}
	}
	return ret;
}

ut64 data_prev_size(ut64 off, int type)
{
	struct list_head *pos;
	ut64 ret = 0;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == type) {
			if (d->from < off && d->to > off)
				ret = (d->to-d->from) - (off-d->from);;
		}
	}
	return ret;
}

int data_get_fun_for(ut64 addr, ut64 *from, ut64 *to)
{
	struct list_head *pos;
	int n_functions = 0;
	int n_xrefs = 0;
	int n_dxrefs = 0;
	struct data_t *rd = NULL;
	ut64 lastfrom = 0LL;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == DATA_FUN) {
			//if (d->from < addr && d->from > lastfrom) {
			if (d->from < addr && addr < d->to ) { //&& d->from > lastfrom) {
				rd = d;
			}
		}
	}
	if (rd) {
		*from = rd->from;
		*to = rd->to;
		return 1;
	}
	return 0;
}

void data_info()
{
	struct list_head *pos;
	int n_functions = 0;
	int n_xrefs = 0;
	int n_dxrefs = 0;
	int n_structs = 0;
	int n_strings = 0;

	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->type == DATA_FUN)
			n_functions++;
		if (d->type == DATA_STR)
			n_strings++;
		if (d->type == DATA_STRUCT)
			n_structs++;
	}

	list_for_each(pos, &xrefs) {
		struct xrefs_t *x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->type == 0)
			n_dxrefs++;
		else n_xrefs++;
	}
	
	cons_printf("strings: %d\n", n_strings);
	cons_printf("functions: %d\n", n_functions);
	cons_printf("structs: %d\n", n_structs);
	cons_printf("data_xrefs: %d\n", n_dxrefs);
	cons_printf("code_xrefs: %d\n", n_xrefs);
}

int data_set(ut64 off, int type)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (off>= d->from && off<= d->to) {
			d->type = type;
			return 0;
		}
	}
	return -1;
}

struct data_t *data_add_arg(ut64 off, int type, const char *arg)
{
	struct data_t *d;
	if (arg == NULL)
		return NULL;
	d = data_add(off, type);
	if (d != NULL)
		strncpy(d->arg , arg, sizeof(d->arg));
	return d;
}

void data_del(ut64 addr, int type, int len/* data or code */)
{
	struct data_t *d;
	struct list_head *pos;
	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->from == addr && type == d->type && (len==0||len==d->size)) {
			list_del(&(d->list));
			break;
		}
	}
}

struct data_t *data_add(ut64 off, int type)
{
	ut64 tmp;
	struct data_t *d = NULL;
	struct list_head *pos;

	__reloop:
	// TODO: use safe foreach here
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d && (off>= d->from && off< d->to) ) {
			list_del((&d->list));
			goto __reloop;
		}
	}

	if (type == DATA_CODE)
		return d;

	d = (struct data_t *)malloc(sizeof(struct data_t));
	memset(d, '\0', sizeof(d));
	d->arg[0]='\0';
	d->from = off;
	d->to = d->from + config.block_size;  // 1 byte if no cursor // on strings should autodetect

	if (config.cursor_mode) {
		d->to = d->from + 1;
		d->from+=config.cursor;
		if (config.ocursor!=-1)
			d->to = config.seek+config.ocursor;
		if (d->to < d->from) {
			tmp = d->to;
			d->to  = d->from;
			d->from = tmp;
		}
	}
	d->type = type;
	if (d->to > d->from) {
	//	d->to++;
		d->size = d->to - d->from+1;
	} else d->size = d->from - d->to+1;
	if (d->size<1)
		d->size = 1;

	list_add(&(d->list), &data);

	return d;
}

ut64 data_seek_to(ut64 offset, int type, int idx)
{
	ut64 ret = 0ULL;
	struct list_head *pos;
	int i = 0;
	idx--;

	list_for_each(pos, &xrefs) {
		struct xrefs_t *d = (struct xrefs_t *)list_entry(pos, struct xrefs_t , list);
		if (d->type == type || type == -1) {
			if (d->addr == offset && idx == i) {
				ret = d->from;
				break;
			}
			i++;
		}
	}
	return ret;
}

struct data_t *data_get(ut64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to)
			return d;
	}
	return NULL;
}

struct data_t *data_get_range(ut64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset >= d->from && offset < d->to)
			return d;
	}
	return NULL;
}

/* TODO: OPTIMIZE: perform cache here */
struct data_t *data_get_between(ut64 from, ut64 to)
{
	int hex = 0;
	int str = 0;
	int fun = 0;
	int stc = 0;
	int code = 0;
	struct list_head *pos;
	struct data_t *d = NULL;
	static struct data_t ret;

	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		//if (from >= d->from && to <= d->to) {
		if (d->from >= from && d->to < to) {
			switch(d->type) {
			case DATA_HEX: hex++; break;
			case DATA_STR: str++; break;
			case DATA_CODE: code++; break;
			case DATA_FUN: fun++; break;
			case DATA_STRUCT: stc++; break;
			}
		}
	}

#if 0
	if (d == NULL)
		return NULL;

	if (hex>=str && hex>=code && hex>=fun && hex >= stc) {
		d->type = DATA_HEX;
		d->times = hex;
	} else
	if (str>=hex && str>=code && str>=fun && str >= stc) {
		d->type = DATA_STR;
		d->times = str;
	} else
	if (fun>=hex && fun>=str && fun>=code && fun >= stc) {
		d->type = DATA_FUN;
		d->times = fun;
	} else
	if (code>=hex && code>=str && code>=fun && code >=stc) {
		d->type = DATA_CODE;
		d->times = code;
	} else
	if (stc>=hex && stc>=str && stc>=fun && stc>=code) {
		d->type = DATA_STRUCT;
		d->times = stc;
	}
	// TODO add struct
//printf("0x%llx-0x%llx: %d %d %d = %d\n", from, to, hex, str, code, d->type);

	return d;
#endif
	
	if (hex>=str && hex>=code && hex>=fun && hex >= stc) {
		ret.type = DATA_HEX;
		ret.times = hex;
	} else
	if (str>=hex && str>=code && str>=fun && str >= stc) {
		ret.type = DATA_STR;
		ret.times = str;
	} else
	if (fun>=hex && fun>=str && fun>=code && fun >= stc) {
		ret.type = DATA_FUN;
		ret.times = fun;
	} else
	if (code>=hex && code>=str && code>=fun && code >=stc) {
		ret.type = DATA_CODE;
		ret.times = code;
	} else
	if (stc>=hex && stc>=str && stc>=fun && stc>=code) {
		ret.type = DATA_STRUCT;
		ret.times = stc;
	}

	return &ret;
}

int data_type_range(ut64 offset)
{
	struct data_t *d = data_get_range(offset);
	if (d != NULL)
		return d->type;
	return -1;
}

int data_type(ut64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->type;
	}
	return -1;
}

int data_end(ut64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from+d->size) // XXX: must be d->to..but is buggy ?
			return d->type;
	}
	return -1;
}

int data_size(ut64 offset)
{
	struct list_head *pos;
	list_for_each(pos, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (offset == d->from)
			return d->size;
	}
	return 0;
}

// TODO: add grep flags here
int data_list_ranges()
{
	struct data_t *d;
	struct list_head *pos;

	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		switch(d->type) {
		case DATA_FUN:
			cons_printf("ar+ 0x%08llx 0x%08llx\n",
				d->from, d->to);
			break;
		}
	}
}

/* TODO: add grep flags argument */
int data_list(const char *mask)
{
	char pfx[16];
	char *arg;
	char label[1024], str[1024];
	struct data_t *d;
	struct list_head *pos;

	list_for_each(pos, &data) {
		d = (struct data_t *)list_entry(pos, struct data_t, list);
		label[0]='\0';
		string_flag_offset(NULL, label, d->from, 0);
		arg = NULL;
		switch(d->type) {
		case DATA_FOLD_O: strcpy(pfx,"Cu"); break;
		case DATA_FOLD_C: strcpy(pfx,"Cf"); break;
		case DATA_FUN:    strcpy(pfx,"CF"); break;
		case DATA_HEX:    strcpy(pfx,"Cd"); break;
		case DATA_STR:    strcpy(pfx,"Cs"); break;
		case DATA_STRUCT: strcpy(pfx,"Cm"); arg = d->arg; break;
		default:          strcpy(pfx,"Cc"); break; }
		sprintf(str, "%s %lld %s@ 0x%08llx ; %s", 
			pfx, d->to-d->from, arg?arg:"", d->from, label);
		if (!mask || !*mask || str_grep(str, mask)) {
			cons_strcat(str);
			cons_newline();
		}
#if 0
		if (verbose)
		if (d->type == DATA_STR) {
			cons_printf("  (");
			sprintf(label, "pz@0x%08llx", d->from);
			radare_cmd(label, 0);
		}else
#endif
	}
	return 0;
}
/* -- metadata -- */
int data_xrefs_print(ut64 addr, int type)
{
	char str[1024];
	int n = 0;
	struct xrefs_t *x;
	struct list_head *pos;
	int xrefsto = (int)config_get_i("asm.xrefsto"); // XXX pretty slow
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr) {
			str[0]='\0';
			string_flag_offset(NULL, str, x->from, 0);
			switch(type) {
			case 0: if (x->type == type) {
					cons_printf("; 0x%08llx CODE xref 0x%08llx (%s)\n",
						addr, x->from, str);
					n++;
				}
				break;
			case 1: if (x->type == type) {
					cons_printf("; 0x%08llx DATA xref 0x%08llx (%s)\n",
						addr, x->from), str;
					n++;
				}
				break;
			default:
				cons_printf("; 0x%08llx %s xref from 0x%08llx (%s)\n",
					addr, (x->type==1)?"DATA":(x->type==0)?"CODE":"UNKNOWN",x->from, str);
				n++;
				break;
			}
		} else
		if (xrefsto && x->from == addr) {
			cons_printf("; 0x%08llx %s xref to 0x%08llx (%s)\n",
				addr, (x->type==1)?"DATA":(x->type==0)?"CODE":"UNKNOWN",x->addr, str);
		}
	}

	return n;
}

int data_xrefs_add(ut64 addr, ut64 from, int type)
{
	struct xrefs_t *x;
	struct list_head *pos;

	/* avoid dup */
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from)
			return 0;
	}

	x = (struct xrefs_t *)malloc(sizeof(struct xrefs_t));

	x->addr = addr;
	x->from = from;
	x->type = type;

	list_add(&(x->list), &xrefs);

	return 1;
}

int data_xrefs_at(ut64 addr)
{
	int ctr = 0;
	struct xrefs_t *x;
	struct list_head *pos;

	/* avoid dup */
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr)
			ctr++;
	}
	return ctr;

}

void data_xrefs_del(ut64 addr, ut64 from, int data /* data or code */)
{
	struct xrefs_t *x;
	struct list_head *pos;
	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr == addr && x->from == from) {
			list_del(&(x->list));
			break;
		}
	}
}

void data_comment_del(ut64 offset, const char *str)
{
	struct comment_t *cmt;
	struct list_head *pos, *n;
	//ut64 off = get_math(str);

	list_for_each_safe(pos, n, &comments) {
		cmt = list_entry(pos, struct comment_t, list);
#if 0
		if (!pos)
			return;
#endif

#if 0
		if (off) {
			if ((off == cmt->offset)) {
				free(cmt->comment);
				list_del(&(pos));
				free(cmt);
				if (str[0]=='*')
					data_comment_del(offset, str);
				pos = comments.next; // list_init
				return;
			}
		} else {
#endif
		    if (offset == cmt->offset) {
			    if (str[0]=='*') {
				    free((void *)cmt->comment);
				    list_del(&(cmt->list));
				    free(cmt);
				    pos = comments.next; // list_init
			    } else
			    if (!strcmp(cmt->comment, str)) {
				    list_del(&(cmt->list));
				    return;
			    }
		    }
#if 0
		}
#endif
	}
}

void data_comment_add(ut64 offset, const char *str)
{
	struct comment_t *cmt;
	char *ptr;

	/* no null comments */
	if (strnull(str))
		return;

	/* avoid dupped comments */
	data_comment_del(offset, str); // XXX segfault here with -O2

	cmt = (struct comment_t *) malloc(sizeof(struct comment_t));
	cmt->offset = offset;
	ptr = strdup(str);
	if (ptr[strlen(ptr)-1]=='\n')
		ptr[strlen(ptr)-1]='\0';
	cmt->comment = ptr;
	list_add_tail(&(cmt->list), &(comments));
}

void data_del_range(ut64 from, ut64 to)
{
	struct list_head *pos, *n;

	list_for_each_safe (pos, n, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		if (cmt->offset >= from && cmt->offset <= to)
			list_del(&(cmt->list));
	}

	list_for_each_safe (pos, n, &xrefs) {
		struct xrefs_t *x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (x->addr >= from && x->addr <= to)
			list_del(&(x->list));
		if (x->from >= from && x->from <= to)
			list_del(&(x->list));
	}

	list_for_each_safe (pos, n, &data) {
		struct data_t *d = (struct data_t *)list_entry(pos, struct data_t, list);
		if (d->from >= from && d->from <= to)
			list_del(&(d->list));
		else
		if (d->to >= from && d->to <= to)
			list_del(&(d->list));
	}
}

void data_comment_list(const char *mask)
{
	char str[1024];
	struct list_head *pos;
	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		sprintf(str, "CC %s @ 0x%llx\n", cmt->comment, cmt->offset);
		if (!mask || !*mask || str_grep(str, mask))
			cons_strcat(str);
	}
}

void data_xrefs_here(ut64 addr)
{
	int count = 0;
	char label[1024];
	struct xrefs_t *x;
	struct list_head *pos;

	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		if (addr = x->addr) {
			label[0]='\0';
			string_flag_offset(NULL, label, x->from, 0);
			cons_printf("%d %s xref 0x%08llx @ 0x%08llx ; %s\n",
				count+1, x->type?"data":"code", x->from, x->addr, label);
			count++;
		}
	}
	if (count == 0) {
		eprintf("No xrefs found\n");
	}
}

void data_xrefs_list(const char *mask)
{
	char label[512], str[1024];
	struct xrefs_t *x;
	struct list_head *pos;

	list_for_each(pos, &xrefs) {
		x = (struct xrefs_t *)list_entry(pos, struct xrefs_t, list);
		label[0]='\0';
		string_flag_offset(NULL, label, x->from, 0);
		sprintf(str, "C%c 0x%08llx @ 0x%08llx ; %s\n", x->type?'d':'x', x->from, x->addr, label);
		if (!mask || !*mask || str_grep(str, mask))
			cons_printf(str);
	}
}

char *data_comment_get(ut64 offset, int lines)
{
	struct list_head *pos;
	char *str = NULL;
	int cmtmargin = (int)config_get_i("asm.cmtmargin");
	int cmtlines = config_get_i("asm.cmtlines");
	char null[128];

	memset(null,' ',126);
	null[126]='\0';
	if (cmtmargin<0) cmtmargin=0; else
		// TODO: use screen width here
		if (cmtmargin>80) cmtmargin=80;
	null[cmtmargin] = '\0';
	if (cmtlines<0)
		cmtlines=0;

	if (cmtlines) {
		int i = 0;
		list_for_each(pos, &comments) {
			struct comment_t *cmt = list_entry(pos, struct comment_t, list);
			if (cmt->offset == offset)
				i++;
		}
		if (i>cmtlines)
			cmtlines = i-cmtlines;
	}

	list_for_each(pos, &comments) {
		struct comment_t *cmt = list_entry(pos, struct comment_t, list);
		if (cmt->offset == offset) {
			if (cmtlines) {
				cmtlines--;
				continue; // skip comment lines
			}
			if (str == NULL) {
				str = malloc(1024);
				str[0]='\0';
			} else {
				str = realloc(str, cmtmargin+strlen(str)+strlen(cmt->comment)+128);
			}
			strcat(str, null);
			strcat(str, "; ");
			strcat(str, cmt->comment);
			strcat(str, "\n");
			if (--lines == 0)
				break;
		}
	}
	return str;
}

void data_comment_init(int new)
{
	INIT_LIST_HEAD(&(vartypes));
	INIT_LIST_HEAD(&(xrefs));
	INIT_LIST_HEAD(&(comments));
	INIT_LIST_HEAD(&(data));
	var_init();
}

void data_reflines_init(int lines, int linescall)
{
	reflines = NULL;
	if (lines)
		reflines = code_lines_init(linescall);
}

int data_printd(int delta)
{
	int show_lines = PTRCAST (config_get("asm.lines"));
	ut64 offset = (ut64)config.seek + (ut64)delta;// - config.vaddr;
	int lines = 0;
	const char *ptr;

	D {} else return 0;

	ptr = data_comment_get(offset, config.height-cons_lines);
	if (ptr && ptr[0]) {
		int i;
		for(i=0;ptr[i];i++)
			if (ptr[i]=='\n') lines++;
		C 	cons_printf(C_MAGENTA"%s"C_RESET, ptr);
		else 	cons_strcat(ptr);
		free((void *)ptr);
	}

	lines += data_xrefs_print(offset, -1);
	return lines;
}

/* variables */

int data_var_type_add(const char *typename, int size, const char *fmt)
{
	struct var_type_t *d = (struct var_type_t *)
		malloc(sizeof(struct var_type_t));
	strncpy(d->name, typename, sizeof(d->name));
	strncpy(d->fmt, fmt, sizeof(d->fmt));
	d->size = size;
	list_add(&(d->list), &vartypes);
	
	return 0;
}

int data_var_type_del(const char *typename)
{
	struct list_head *pos;

	if (*typename==' ')typename=typename+1;

	list_for_each(pos, &vartypes) {
		struct var_type_t *d = (struct var_type_t *)list_entry(pos, struct var_type_t, list);
		if (!strcmp(typename, d->name)) {
			list_del(&(d->list));
			return 1;
		}
	}
	
	return 0;
}

int data_var_type_list(const char *mask)
{
	char str[128];
	struct list_head *pos;
	ut64 ret = 0;

	list_for_each(pos, &vartypes) {
		struct var_type_t *d = (struct var_type_t *)list_entry(pos, struct var_type_t, list);
		sprintf(str, "%s %d %s\n", d->name, d->size, d->fmt);
		if (!mask || !*mask || str_grep(str, mask))
			cons_printf(str);
	}
	return ret;
	
}

struct var_type_t *data_var_type_get(const char *datatype)
{
	struct list_head *pos;
	list_for_each(pos, &vartypes) {
		struct var_type_t *d = (struct var_type_t *)list_entry(pos, struct var_type_t, list);
		//eprintf("---(%s)(%s)\n", d->name, datatype);
		if (!strcmp(datatype, d->name))
			return d;
	}
	return NULL;
}

int data_var_help()
{
	cons_printf(
		"Usage: Cv [name] [size] [pm-format-string]\n"
		"  Cv int 4 d   ; define 'int' type\n"
		"  Cv- int      ; remove 'int' var type\n"
		"  Cv float 4 f\n");
	return 0;
}

int data_var_cmd(const char *str)
{
	int len;
	char *vstr;
	char *arg, *arg2;
	STRALLOC(vstr, str, len);

	if (*str==' ')str=str+1;
	switch(*str) {
	case '?':
		return data_var_help();
	case '\0':
		/* list var types */
		data_var_type_list(str+1);
		break;
	case '-':
		data_var_type_del(str+1);
		break;
	default:
		arg = strchr(str, ' ');
		if (arg==NULL)
			return data_var_help();
		*arg='\0'; arg=arg+1;
		arg2 = strchr(arg, ' ');
		if (arg2==NULL)
			return data_var_help();
		*arg2='\0'; arg2=arg2+1;
		data_var_type_add(str, atoi(arg), arg2);
		break;
	}
	
	return 0;
}
