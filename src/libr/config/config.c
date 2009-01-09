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

#include "code.h"
#include "main.h"
#include "cons.h"
#include "radare.h"
#include "utils.h"
#include "config.h"
#include "list.h"
#include "readline.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct config_t config;

/* new stuff : radare config 2.0 */

struct config_new_t config_new;

struct config_node_t* config_node_new(const char *name, const char *value)
{
	struct config_node_t *node = 
		(struct config_node_t *)malloc(sizeof(struct config_node_t));

	INIT_LIST_HEAD(&(node->list));
	node->name = strdup(name);
	node->hash = strhash(name);
	node->value = value?strdup(value):strdup("");
	node->flags = CN_RW | CN_STR;
	node->i_value = 0;
	node->callback = NULL;

	return node;
}

void config_list(const char *str)
{
	struct list_head *i;
	int len = 0;

	if (str&&str[0]) {
		str = strclean(str);
		len = strlen(str);
	}

	list_for_each(i, &(config_new.nodes)) {
		struct config_node_t *bt = list_entry(i, struct config_node_t, list);
		if (str) {
			if (strncmp(str, bt->name,len) == 0)
				cons_printf("%s = %s\n", bt->name, bt->value);
		} else {
			cons_printf("%s = %s\n", bt->name, bt->value);
		}
	}
}

struct config_node_t *config_node_get(const char *name)
{
	struct list_head *i;
	int hash = strhash(name);

	list_for_each_prev(i, &(config_new.nodes)) {
		struct config_node_t *bt = list_entry(i, struct config_node_t, list);
		if (bt->hash == hash)
			return bt;
	}

	return NULL;
}

int config_get_notfound= 0;
const char *config_get(const char *name)
{
	struct config_node_t *node;

	node = config_node_get(name);
	if (node) {
		config_get_notfound = 0;
		if (node->flags & CN_BOOL)
			return (const char *)
				(((!strcmp("true", node->value))
				  || (!strcmp("1", node->value)))?(const char *)1:NULL);
		return node->value;
	}

	config_get_notfound = 1;
	return NULL;
}

u64 config_get_i(const char *name)
{
	struct config_node_t *node;

	node = config_node_get(name);
	if (node) {
		if (node->i_value != 0)
			return node->i_value;
		return (u64)get_math(node->value);
	}

	return (u64)0LL;
}

struct config_node_t *config_set(const char *name, const char *value)
{
	struct config_node_t *node;

	if (name[0] == '\0')
		return NULL;

	node = config_node_get(name);

	// TODO: store old value anywhere or something..
	if (node) {
		if (node->flags & CN_RO) {
			eprintf("(read only)\n");
			return node;
		}
		free(node->value);
		if (node->flags & CN_BOOL) {
			int b = (!strcmp(value,"true")||!strcmp(value,"1"));
			node->i_value = (u64)(b==0)?0:1;
			//node->value = estrdup(node->value, b?"true":"false");
			node->value = strdup(b?"true":"false");
		} else {
			if (value == NULL) {
				node->value = strdup("");
				node->i_value = 0;
			} else {
				node->value = strdup(value);
				if (strchr(value, '/'))
					node->i_value = get_offset(value);
				else  node->i_value = get_math(value);
				node->flags |= CN_INT;
			}
		}
	} else {
		if (config_new.lock) {
			eprintf("(config-locked: '%s' no new keys can be created)\n", name);
		} else {
			node = config_node_new(name, value);
			if (value)
				if (!strcmp(value,"true")||!strcmp(value,"false")) {
					node->flags|=CN_BOOL;
					node->i_value = (!strcmp(value,"true"))?1:0;
				}
			list_add_tail(&(node->list), &(config_new.nodes));
		}
	}

	if (node&&node->callback)
		node->callback(node);

	return node;
}

int config_rm(const char *name)
{
	struct config_node_t *node;

	node = config_node_get(name);

	if (node)
		cons_printf("TODO: remove: not yet implemented\n");
	else
		cons_printf("node not found\n");

	return 0;
}

struct config_node_t *config_set_i(const char *name, const u64 i)
{
	char buf[128];
	struct config_node_t *node;

	node = config_node_get(name);

	if (node) {
		if (node->flags & CN_RO)
			return NULL;
		free(node->value);
		if (node->flags & CN_BOOL) {
			node->value = strdup(i?"true":"false");
		} else {
			sprintf(buf, "%lld", i); //0x%08lx", i);
			node->value = strdup(buf);
		}
		node->flags = CN_RW | CN_INT;
		node->i_value = i;
	} else {
		if (config_new.lock) {
			eprintf("(locked: no new keys can be created)");
		} else {
			sprintf(buf, "%d", (unsigned int)i);//OFF_FMTd, (u64) i);
			node = config_node_new(name, buf);
			node->flags = CN_RW | CN_OFFT;
			node->i_value = i;
			list_add_tail(&(node->list), &(config_new.nodes));
		}
	}

	if (node&&node->callback)
		node->callback(node);

	return node;
}

void config_eval(char *str)
{
	char *ptr,*a,*b;
	char *name;
	int len;

	if (str == NULL)
		return;

	len = strlen(str)+1;
	name = alloca(len);
	memcpy(name, str, len);
	str = strclean(name);

	if (str == NULL)
		return;

	if (str[0]=='\0'||!strcmp(str, "help")) {
		config_list(NULL);
		return;
	}

	if (str[0]=='-') {
		config_rm(str+1);
		return;
	}

	ptr = strchr(str, '=');
	if (ptr) {
		/* set */
		ptr[0]='\0';
		a = strclean(name);
		b = strclean(ptr+1);
		config_set(a, b);
	} else {
		char *foo = strclean(name);
		if (foo[strlen(foo)-1]=='.') {
			/* list */
			config_list(name);
		} else {
			/* get */
			const char * str = config_get(foo);
			if (config_get_notfound)
				config_list(name);
			else cons_printf("%s\n", (((int)str)==1)?"true":(str==0)?"false":str);
		}
	}
}

static int config_bigendian_callback(void *data)
{
	struct config_node_t *node = data;
	config.endian = node->i_value?1:0;
	return 1;
}

static int config_scrhtml_callback(void *data)
{
	struct config_node_t *node = data;
	cons_is_html = node->i_value?1:0;
	return 1;
}

static int config_filterfile_callback(void *data)
{
	struct config_node_t *node = data;
	if (!node->value || node->value[0]=='\0') {
		efree(&cons_filterline);
	} else cons_filterline = estrdup(cons_filterline, node->value);
	return 1;
}

static int config_teefile_callback(void *data)
{
	struct config_node_t *node = data;
	if (!node->value || node->value[0]=='\0') {
		efree(&cons_teefile);
	} else cons_teefile = estrdup(cons_teefile, node->value);
	return 1;
}

static int config_zoombyte_callback(void *data)
{
	struct config_node_t *node = data;

	switch(node->value[0]) {
		/* ok */
		case 'h': // head
		case 'f': // flags
		case 'F': // 0xFF
		case 'e': // entropy
		case 'p': // print
		case 't': // traces
		case 'c': // code
		case 's': // strings
			break;
			/* not ok */
		default:
			free(node->value);
			node->value = strdup("head");
			return 0;
	}
	return 1;
}

#if 0
static int config_core_callback(void *data)
{
	struct config_node_t *node = data;

	if (!strcmp(node->name, "core.jmp")) {
		hist_goto(node->value);
	} else
		if (!strcmp(node->name, "core.echo")) {
			cons_printf("%s\n", node->value);
		} else
			if (!strcmp(node->name, "core.cmp")) {
				hist_cmp(node->value);
			} else
				if (!strcmp(node->name, "core.load")) {
					hist_load(node->value);
				} else
					if (!strcmp(node->name, "core.dump")) {
						hist_dump(node->value);
					} else
						if (!strcmp(node->name, "core.list")) {
							hist_show();
						} else
							if (!strcmp(node->name, "core.reset")) {
								hist_reset();
							} else
								if (!strcmp(node->name, "core.je")) {
									hist_cgoto(node->value, OP_JE);
								} else
									if (!strcmp(node->name, "core.jne")) {
										hist_cgoto(node->value, OP_JNE);
									} else
										if (!strcmp(node->name, "core.ja")) {
											hist_cgoto(node->value, OP_JA);
										} else
											if (!strcmp(node->name, "core.jb")) {
												hist_cgoto(node->value, OP_JB);
											} else
												if (!strcmp(node->name, "core.loop")) {
													hist_loop(node->value);
												} else
													if (!strcmp(node->name, "core.break")) {
														// ignored
														//hist_break();
													} else
														if (!strcmp(node->name, "core.label")) {
															hist_add_label(node->value);
														}
													// TODO needs more work
}
#endif

static int config_limit_callback(void *data)
{
	struct config_node_t *node = data;

	config.limit = get_offset(node->value);

	return 0;
}

static int asm_profile(const char *profile)
{
	if (!strcmp(profile, "help")) {
		eprintf("Available asm.profile:\n");
		eprintf(" default\n");
		eprintf(" gas\n");
		eprintf(" smart\n");
		eprintf(" graph\n");
		eprintf(" debug\n");
		eprintf(" full\n");
		eprintf(" simple\n");
	} else
		if (!strcmp(profile, "default")) {
			config_set("asm.bytes", "true");
			config_set("asm.lines", "true");
			config_set("asm.linesout", "false");
			config_set("asm.lineswide", "false");
			config_set("asm.offset", "true");
			config_set("asm.comments", "true");
			config_set("asm.flagsline", "false");
			config_set("asm.section", "false");
			config_set("asm.trace", "false");
			config_set("asm.split", "true");
			config_set("asm.flags", "true");
			config_set("asm.size", "false");
			config_set("asm.xrefs", "true");
			config_set("scr.color", "true");
		} else
			if (!strcmp(profile, "gas")) {
				asm_profile("default");
				config_set("asm.lines", "false");
				config_set("asm.comments", "false");
				config_set("asm.section", "false");
				config_set("asm.trace", "false");
				config_set("asm.bytes", "false");
				config_set("asm.stackptr", "false");
				config_set("asm.offset", "false");
				config_set("asm.flags", "true");
				config_set("asm.flagsline", "true");
				config_set("asm.jmpflags", "true");
				config_set("scr.color", "false");
			} else
				if (!strcmp(profile, "smart")) {
					asm_profile("default");
					config_set("asm.section", "false");
					config_set("asm.trace", "false");
					config_set("asm.bytes", "false");
					config_set("asm.stackptr", "false");
				} else
					if (!strcmp(profile, "graph")) {
						asm_profile("default");
						config_set("asm.section", "false");
						config_set("asm.bytes", "false");
						config_set("asm.trace", "false");
						config_set("scr.color", "false");
						config_set("asm.lines", "false");
						config_set("asm.stackptr", "false");
						if (config_get("graph.offset"))
							config_set("asm.offset", "true");
						else   config_set("asm.offset", "false");
					} else
						if (!strcmp(profile, "debug")) {
							asm_profile("default");
							config_set("asm.trace", "true");
						} else
							if (!strcmp(profile, "full")) {
								asm_profile("default");
								config_set("asm.bytes", "true");
								config_set("asm.lines", "true");
								config_set("asm.linesout", "true");
								config_set("asm.lineswide", "true");
								config_set("asm.section", "true");
								config_set("asm.size", "true");
							} else
								if (!strcmp(profile, "simple")) {
									asm_profile("default");
									config_set("asm.bytes", "false");
									config_set("asm.lines", "false");
									config_set("asm.comments", "false");
									config_set("asm.split", "false");
									config_set("asm.flags", "false");
									config_set("asm.flagsline", "true");
									config_set("asm.xrefs", "false");
									config_set("asm.stackptr", "false");
									config_set("asm.section", "false");
								}
							return 0;
}

static int config_asm_profile(void *data)
{
	struct config_node_t *node = data;

	return asm_profile( node->value );
}

static int config_arch_callback(void *data)
{
	radis_update();

	return 0;
}

static int config_debug_callback(void *data)
{
	struct config_node_t *node = data;

	if (node && node->i_value)
		config.debug = node->i_value;

	return 0;
}

static int config_verbose_callback(void *data)
{
	struct config_node_t *node = data;

	if (node && node->i_value) {
		config.verbose = node->i_value;
		dl_echo = config.verbose;
	}

	return 0;
}

static int config_wmode_callback(void *data)
{
	//struct config_node_t *node = data;
	//if (node && node->i_value)
	// XXX: strange magic conditional
	if (config.fd != -1 && config.file && !config.debug) // && config_new.lock)
		radare_open(0);

	return 1;
}

static int config_palette_callback(void *data)
{
	struct config_node_t *node = data;

	if (!strcmp(node->name, "scr.palette")) {
		cons_palette_init(node->value);
		return 0;
	}
	// 8 = strlen(scr.pal.)
	cons_palette_set(node->name+8, node->value);

	return 1;
}

static int config_color_callback(void *data)
{
	struct config_node_t *node = data;

	if (node && node->i_value)
		config.color = (int)node->i_value;
	return 1;
}

#if 0
int config_asm_dwarf(void *data)
{
	struct config_node_t *node = data;

	if (node && node->value)
		config_set("cmd.asm", "!rsc dwarf-addr ${FILE} ${HERE}");
	else  config_set("cmd.asm", "");
	return 1;
}
#endif

static int config_paddr_callback(void *data)
{
	struct config_node_t *node = data;

	if (node) {
		config.paddr = (u64)node->i_value;
		section_set(config.seek, -1, config.paddr, -1, NULL);
	}
	return 1;
}

static int config_vaddr_callback(void *data)
{
	struct config_node_t *node = data;

	if (node) {
		config.vaddr = (u64)node->i_value;
		section_set(config.seek, -1, config.vaddr, -1, NULL);
	}
	return 1;
}

static int config_scrwidth(void *data)
{
	struct config_node_t *node = data;
	config.width = node->i_value;
	if (config.width<1) {
		cons_get_real_columns();
		if (config.width<1)
			config.width = 80;
	}
	return config.width;
}

static int config_scrheight(void *data)
{
	struct config_node_t *node = data;
	config.height = node->i_value;
	if (config.height<1) {
		cons_get_real_columns();
		if (config.height<1)
			config.height = 24;
	}
	return config.height;
}


config_scrbuf_callback(void *data)
{
	struct config_node_t *node = data;
	config.buf = node->i_value;
	return 1;
}

int config_set_callback_i(const char *key, )
{
}

static int config_bsize_callback(void *data)
{
	struct config_node_t *node = data;

	if (node->i_value)
		radare_set_block_size_i(node->i_value);
	return 1;
}

void config_lock(struct r_config_t *obj, int l)
{
	obj->lock = l;
}

int config_init(struct r_config_t *cfg)
{
	cfg->n_nodes = 0;
	cfg->lock = 0;
	INIT_LIST_HEAD(&(cfg->nodes));
}

void config_visual_hit_i(struct r_config_t *cfg, const char *name, int delta)
{
	struct config_node_t *node;
	node = config_node_get(cfg, name);
	if (node && node->flags & CN_INT || node->flags & CN_OFFT)
		config_set_i(name, config_get_i(name)+delta);
}
