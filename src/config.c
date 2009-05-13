/*
 * Copyright (C) 2006, 2007, 2008, 2009
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

int rdb_init()
{
	const char *rdbdir;
	const char *rdbfile;
	int fd = -1;
	char *str =
		"# RDB (radare database) file\n"
		"chdir=\nchroot=\nsetuid=\nsetgid=\n";
	/* FUCKY! */
	// just set project name (cfg.project)

	rdbfile = config_get("file.project");
	if (strnull(rdbfile))
		return -1;

	fd = open(rdbfile, O_APPEND|O_RDWR, 0644);
	rdbdir = config_get("dir.project");
	if (rdbdir&&rdbdir[0]) {
		util_mkdir(rdbdir);
		chdir(rdbdir);
	}
	if (fd == -1) {
		fd = open(rdbfile, O_CREAT|O_APPEND|O_RDWR, 0644);
		if (fd != -1 )
			write(fd, (const void *)str, strlen(str));
		else {
			eprintf("Cannot open '%s' for writting.\n", rdbfile);
			return -1;
		}
	}
	close(fd);

	return open(rdbfile, O_CREAT|O_APPEND|O_RDWR, 0644);
}

static void config_old_init()
{
	memset(&config, '\0', sizeof(config));
	config.mode        = MODE_SHELL;
	config.endian      = 1;
	config.noscript    = 0;
	config.script      = NULL;
	config.vaddr       = 0;
	config.paddr       = 0;
	config.seek        = 0;
	config.arch        = ARCH_X86;
	config.lines       = 0;
	config.debug       = 0;
	config.color     = 0;
	config.unksize     = 0;
	config.buf         = 0; // output buffered
	config.ene         = 10;
	config.width       = cons_get_columns();
	config.last_seek   = 0;
	config.file        = NULL;
	config.scrfit      = 1;
	config.block_size  = DEFAULT_BLOCK_SIZE;
	config.cursor      = 0;
	config.acursor     = 0;
	config.ocursor     = -1;
	config.block       = (unsigned char *)malloc(config.block_size);
	config.verbose     = 1;
	config.interrupted = 1;
	config.graph       = 0;
	config.visual      = 0;
	config.scrdelta    = 0;
	config.lang        = 0;
	config.fd          = -1;
	config.zoom.size   = config.size;
	config.zoom.from   = 0;
	config.zoom.enabled= 0;
	config.zoom.piece  = config.size/config.block_size;
#if HAVE_PERL
	config.lang        = LANG_PERL;
#endif
#if HAVE_PYTHON
	config.lang        = LANG_PYTHON;
#endif
	INIT_LIST_HEAD(&config.rdbs);
}

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
				if (!strchr(value, '%')) {
					if (strchr(value, '/'))
						node->i_value = get_offset(value);
					else  node->i_value = get_math(value);
				} else node->i_value = 0;
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

static int config_scr_interactive_callback(void *data)
{
	struct config_node_t *node = data;
	cons_interactive = node->i_value?1:0;
	return 1;
}

static int config_scrhtml_callback(void *data)
{
	struct config_node_t *node = (struct config_node_t *)data;
	cons_is_html = node->i_value?1:0;
	return 1;
}

static int config_filterfile_callback(void *data)
{
	struct config_node_t *node = (struct config_node_t *)data;
	if (!node->value || node->value[0]=='\0') {
		efree(&cons_filterline);
	} else cons_filterline = estrdup(cons_filterline, node->value);
	return 1;
}

static int config_teefile_callback(void *data)
{
	struct config_node_t *node = (struct config_node_t *)data;
	if (!node->value || node->value[0]=='\0') {
		efree(&cons_teefile);
	} else cons_teefile = estrdup(cons_teefile, node->value);
	return 1;
}

static int config_zoom_callback(void *data)
{
	struct config_node_t *node = (struct config_node_t *)data;
	print_zoom(
		config_get_i("zoom.from"),
		config_get_i("zoom.to"),
		config_get("zoom.byte"),
		node->i_value?1:0
		);
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
	} else if (!strcmp(profile, "default")) {
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
	} else if (!strcmp(profile, "compact")) {
		asm_profile("simple");
		config_set("asm.lines", "true");
		config_set("asm.comments", "false");
		config_set("scr.color", "false");
	} else if (!strcmp(profile, "gas")) {
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
	} else if (!strcmp(profile, "smart")) {
		asm_profile("default");
		config_set("asm.section", "false");
		config_set("asm.trace", "false");
		config_set("asm.bytes", "false");
		config_set("asm.stackptr", "false");
	} else if (!strcmp(profile, "graph")) {
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
	} else if (!strcmp(profile, "debug")) {
		asm_profile("default");
		config_set("asm.trace", "true");
	} else if (!strcmp(profile, "full")) {
		asm_profile("default");
		config_set("asm.bytes", "true");
		config_set("asm.lines", "true");
		config_set("asm.linesout", "true");
		config_set("asm.lineswide", "true");
		config_set("asm.section", "true");
		config_set("asm.size", "true");
	} else if (!strcmp(profile, "simple")) {
		asm_profile("default");
		config_set("asm.bytes", "false");
		config_set("asm.lines", "false");
		config_set("asm.comments", "true");
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

	if (node) // && node->i_value)
		config.debug = node->i_value;

	return 0;
}

static int config_verbose_callback(void *data)
{
	struct config_node_t *node = data;

	if (node) { // && node->i_value) {
		config.verbose = node->i_value;
		dl_echo = config.verbose;
	}

	return 0;
}

static int config_scrfit_callback(void *data)
{
	struct config_node_t *node = data;
	if (node)
		config.scrfit = node->i_value;
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
	if (node) // && node->i_value)
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
		section_set(config.seek, -1, config.paddr, -1, -1, NULL);
	}
	return 1;
}

static int config_vaddr_callback(void *data)
{
	struct config_node_t *node = data;

	if (node) {
		config.vaddr = (u64)node->i_value;
		section_set(config.seek, -1, config.vaddr, -1, -1, NULL);
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

static int config_dbgth_callback(void *data)
{
#if DEBUGGER
	events_init_all();
#endif
	return 1;
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

static int config_scrbuf_callback(void *data)
{
	struct config_node_t *node = data;

	config.buf = node->i_value;
	return 1;
}

static int config_bsize_callback(void *data)
{
	struct config_node_t *node = data;

	if (node->i_value)
		radare_set_block_size_i(node->i_value);
	return 1;
}

void config_lock(int l)
{
	config_new.lock = l;
}

void config_init(int first)
{
	char buf[1024];
	const char *ptr;
	struct config_node_t *node;

	if (first) {
		flag_init();
		config_old_init();
		section_init(0);

		// TODO : dl_callback = radare_dl_autocompletion;
		dl_init();
		dl_hist_load(".radare_history");

		config_new.n_nodes = 0;
		config_new.lock = 0;
		INIT_LIST_HEAD(&(config_new.nodes));
	}

	/* enter keys */
	node = config_set("asm.profile", "default");
	node->callback = &config_asm_profile;

#if __POWERPC__
	node = config_set("asm.arch", "ppc");
#elif __x86_64__
	node = config_set("asm.arch", "intel64");
#elif __arm__
	node = config_set("asm.arch", "arm");
#elif __mips__
	node = config_set("asm.arch", "mips");
#else
	node = config_set("asm.arch", "intel");
#endif
	node->callback = &config_arch_callback;
	config_set("asm.comments", "true"); // show comments in disassembly
	config_set_i("asm.cmtmargin", 10); // show comments in disassembly
	config_set_i("asm.cmtlines", 0); // show comments in disassembly
	config_set("asm.syntax", "intel");
	config_set("asm.case", "false"); // uppercase = true
	config_set("asm.objdump", "objdump -m i386 --target=binary -D");
	config_set("asm.offset", "true"); // show offset
	config_set("asm.section", "true");
	config_set("asm.stackptr", "true");
	config_set("asm.reladdr", "false"); // relative offset
	config_set_i("asm.nbytes", 8); // show hex bytes
	config_set("asm.bytes", "true"); // show hex bytes
	config_set("asm.jmpflags", "false");
	config_set("asm.flags", "true");
	config_set("asm.flagsall", "true");
	config_set("asm.flagsline", "false");
	config_set("asm.functions", "true");
	config_set("asm.lines", "true"); // show left ref lines
	config_set_i("asm.nlines", 6); // show left ref lines
	config_set("asm.lineswide", "false"); // show left ref lines
	config_set("asm.trace", "false"); // trace counter
	config_set("asm.linesout", "false"); // show left ref lines
	config_set("asm.linestyle", "false"); // foreach / prev
	// asm.os = used for syscall tables and so.. redefined with rabin -rI
	config_set("asm.pseudo", "false"); 
#if __linux__
	config_set("asm.os", "linux"); 
#elif __FreeBSD__
	config_set("asm.os", "freebsd");
#elif __NetBSD__
	config_set("asm.os", "netbsd");
#elif __OpenBSD__
	config_set("asm.os", "openbsd");
#elif __Windows__
	config_set("asm.os", "linux");
#endif
	config_set("asm.split", "true"); // split code blocks
	config_set("asm.splitall", "false"); // split code blocks
	config_set("asm.size", "false"); // opcode size
	config_set("asm.xrefs", "true");
	config_set("asm.xrefsto", "false");

	// config_set("asm.follow", "");
	config_set("cmd.wp", "");
	config_set("cmd.flag", "true");
	config_set("cmd.asm", "");
	config_set("cmd.user", "");
	config_set("cmd.trace", "");
	config_set("cmd.visual", "");
	config_set("cmd.hit", "");
	config_set("cmd.visualbind", "");
	config_set("cmd.touchtrace", "");
	config_set("cmd.open", "");
	config_set("cmd.prompt", "");
	config_set("cmd.vprompt", "p%");
	config_set("cmd.vprompt2", "CFV");
	config_set("cmd.vprompt3", "");
	config_set("cmd.bp", "");

	config_set("search.flagname", "hit%d_%d");
	config_set("search.inar", "false");
	config_set_i("search.from", 0);
	config_set_i("search.to", 0);
	config_set_i("search.align", 0);
	config_set("search.flag", "true");
	config_set("search.verbose", "true");
	config_set_i("search.limit", 0);

	config_set("file.id", "false");
	config_set("file.analyze", "false");
	config_set("file.analdata", "false");
	config_set("file.type", "");
	config_set("file.flag", "false");
	config_set("file.trace", "trace.log");
	config_set("file.project", "");
	config_set("file.entrypoint", "");
	node = config_set("file.scrfilter", "");
	node->callback = &config_filterfile_callback;
	config_set_i("file.size", 0);

	node = config_set_i("io.vaddr", 0); // OLD file.baddr
	node->callback = &config_vaddr_callback;
	node = config_set_i("io.paddr", 0); // physical address
	node->callback = &config_paddr_callback;

	config_set("dump.regs", "true");
	config_set("dump.user", "true");
	config_set("dump.libs", "true");
	config_set("dump.fds", "true");

	config_set("trace.bt", "false");
	config_set("trace.bps", "false");
	config_set("trace.calls", "false");
	config_set_i("trace.sleep", 0);
	config_set("trace.smart", "false");
	config_set("trace.libs", "true");
	config_set("trace.log", "false");
	config_set("trace.dup", "false");
	config_set("trace.cmtregs", "false");

	config_set("cfg.editor", "vi");
	node = config_set("cfg.debug", "false");
	node->callback = &config_debug_callback;
	config_set("cfg.noscript", "false");
	config_set("cfg.sections", "true");
	config_set("cfg.encoding", "ascii"); // cp850
	config_set_i("cfg.delta", 4096); // cp850
	node = config_set("cfg.verbose", "true");
	node->callback = &config_verbose_callback;
#if LIL_ENDIAN
	node = config_set("cfg.bigendian", "false");
#else
	node = config_set("cfg.bigendian", "true");
#endif
	node->callback = &config_bigendian_callback;

	config.endian = config_get_i("cfg.bigendian");
	config_set("cfg.inverse", "false");
	config_set_i("cfg.analdepth", 6);
	config_set("file.insert", "false");
	config_set("file.insertblock", "false");
	config_set("file.undowrite", "true");
	if (first) {
		node = config_set("file.write", "false");
		node->callback = &config_wmode_callback;
	}
	node = config_set("cfg.limit", "0");
	node->callback = &config_limit_callback;
#if __mips__
	// ???
	config_set("cfg.addrmod", "32");
#else
	config_set("cfg.addrmod", "4");
#endif
	config_set("cfg.rdbdir", "TODO");
	config_set("cfg.datefmt", "%d:%m:%Y %H:%M:%S %z");
	config_set_i("cfg.count", 0);
	config_set("cfg.fortunes", "true");
	node = config_set_i("cfg.bsize", 512);
	node->callback = &config_bsize_callback;
	config_set_i("cfg.vbsize", 1024);
	config_set("cfg.vbsize_enabled", "false");

	config_set_i("range.from", 0);
	config_set_i("range.to", 0xffff);
	config_set("range.traces", "true");
	config_set("range.graphs", "true");
	config_set("range.functions", "true");

	config_set("child.stdio", "");
	config_set("child.stdin", "");
	config_set("child.stdout", "");
	config_set("child.stderr", "");
	config_set("child.setgid", ""); // must be int ?
	config_set("child.chdir", ".");
	config_set("child.chroot", "/");
	config_set("child.setuid", "");
#if __mips__
	config_set("dbg.fpregs", "true");
#else
	config_set("dbg.fpregs", "false");
#endif
	config_set("dbg.controlc", "true"); // stop debugger if ^C is pressed
	config_set_i("dbg.focus", 0); // focus on ps.pid or not (ignore events of rest of procs)
	config_set("dbg.syms", "true");
	config_set("dbg.stepo", "false"); // step over for !contu (debug_step())
	config_set("dbg.dwarf", "false");
	config_set("dbg.maps", "true");
	config_set("dbg.contall", "true");
	config_set("dbg.sections", "true");
	config_set("dbg.strings", "false");
	config_set("dbg.stop", "false");
	node = config_set("dbg.forks", "false"); // stop debugger in any fork 
	node->callback = &config_dbgth_callback;
	node = config_set("dbg.threads", "false"); // or clone
	node->callback = &config_dbgth_callback;
	config_set("dbg.contscbt", "false");
	config_set("dbg.contsc2", "true"); // WTF?
	config_set("dbg.regs", "true");
	config_set("dbg.regs2", "false");
	config_set("dbg.stack", "true");
	config_set("dbg.vstack", "true");
	config_set("dbg.wptrace", "false");
	config_set_i("dbg.stacksize", 66);
	config_set("dbg.stackreg", "esp");
	config_set("dbg.bt", "false");
	config_set_i("dbg.btlast", 0);
	config_set("dbg.fullbt", "false"); // user backtrace or lib+user backtrace
	config_set("dbg.bttype", "default"); // default, st and orig or so!
#if __APPLE__ || __ARM__ || __mips__
	config_set("dbg.hwbp", "false"); // default, st and orig or so!
#else
	config_set("dbg.hwbp", "true"); // hardware breakpoints by default // ALSO BSD
#endif
	config_set("dbg.bep", "loader"); // loader, main
	config_set("dir.home", getenv("HOME"));

	/* dir.monitor */
	ptr = getenv("MONITORPATH");
	if (ptr == NULL) {
		sprintf(buf, "%s/.radare/monitor/", getenv("HOME"));
		ptr = (const char *)&buf;
	}
	config_set("dir.monitor", ptr);

	/* dir.spcc */
	ptr = getenv("SPCCPATH");
	if (ptr == NULL) {
		sprintf(buf, "%s/.radare/spcc/", getenv("HOME"));
		ptr = buf;
	}
	config_set("dir.spcc", ptr);

	config_set("dir.plugins", LIBDIR"/radare/");
	snprintf(buf, 1023, "%s/.radare/rdb/", getenv("HOME"));
	config_set("dir.project", buf); // ~/.radare/rdb/
	config_set("dir.tmp", get_tmp_dir());
	config_set("graph.userdup", "false");
	config_set("graph.color", "magic");
	config_set("graph.split", "false"); // split blocks // SHOULD BE TRUE, but true algo is buggy
	config_set("graph.jmpblocks", "true");
	config_set("graph.refblocks", "false"); // must be circle nodes
	config_set("graph.callblocks", "false");
	config_set("graph.flagblocks", "true");
	config_set_i("graph.depth", 9);
	config_set("graph.offset", "true");
	config_set("graph.render", "cairo");    // aalib/ncurses/text
	config_set("graph.layout", "default");  // graphviz

	/* gui */
	config_set("gui.top", "gtk-topbar");  // graphviz
	config_set("gui.tabs", "gtk-prefs");  // graphviz
	config_set("gui.left", "scriptedit gtk-actions");  // graphviz
	config_set("gui.right", "gtk-hello");  // graphviz
	config_set("gui.bottom", "gtk-hello");  // graphviz

	node = config_set("zoom.enable", "false");
	node->callback = &config_zoom_callback;
	node = config_set_i("zoom.from", 0);
	node = config_set_i("zoom.to", config.size);
	node = config_set("zoom.byte", "head");
	node->callback = &config_zoombyte_callback;

	node = config_set("scr.fit", "true");
	node->callback = &config_scrfit_callback;
	node = config_set("scr.html", "false");
	node->callback = &config_scrhtml_callback;
	config_set_i("scr.accel", 0);
	node = config_set("scr.interactive", "true");
	node->callback = &config_scr_interactive_callback;

	node = config_set("scr.palette", cons_palette_default);
	node->callback = &config_palette_callback;
	cons_palette_init(config_get("scr.palette"));
#define config_set_scr_pal(x,y) \
	node = config_set("scr.pal."x"", y); \
	node->callback = &config_palette_callback; \
	node->callback(node);

	config_set_scr_pal("prompt","yellow")
		config_set_scr_pal("default","white")
		config_set_scr_pal("changed","green")
		config_set_scr_pal("jumps","green")
		config_set_scr_pal("calls","green")
		config_set_scr_pal("push","green")
		config_set_scr_pal("trap","red")
		config_set_scr_pal("cmp","yellow")
		config_set_scr_pal("ret","red")
		config_set_scr_pal("nop","gray")
		config_set_scr_pal("metadata","gray")
		config_set_scr_pal("header","green")
		config_set_scr_pal("printable","bwhite")
		config_set_scr_pal("lines0","white")
		config_set_scr_pal("lines1","yellow")
		config_set_scr_pal("lines2","bwhite")
		config_set_scr_pal("address","green")
		config_set_scr_pal("ff","red")
		config_set_scr_pal("00","white")
		config_set_scr_pal("7f","magenta")

		config_set("scr.seek", "eip");
	node = config_set("scr.color", (config.color)?"true":"false");
	node->callback = &config_color_callback;
	config_set("scr.grephigh", "");
	node = config_set("scr.tee", "");
	node->callback = &config_teefile_callback;
	node = config_set("scr.buf", "false");
	node->callback = &config_scrbuf_callback;
	config_set_i("scr.bytewidth", 0);
	node = config_set_i("scr.width", config.width);
	node->callback = &config_scrwidth;
	node = config_set_i("scr.height", config.height);
	node->callback = &config_scrheight;

	config_set("vm.realio", "false");
#if 0

	/* core commands */
	node = config_set("core.echo", "(echo a message)");
	node->callback = &config_core_callback;
	node = config_set("core.jmp", "(jump to label)");
	node->callback = &config_core_callback;
	node = config_set("core.je", "(conditional jump)");
	node->callback = &config_core_callback;
	node = config_set("core.jne", "(conditional jump)");
	node->callback = &config_core_callback;
	node = config_set("core.ja", "(conditional jump)");
	node->callback = &config_core_callback;
	node = config_set("core.jb", "(conditional jump)");
	node->callback = &config_core_callback;
	node = config_set("core.cmp", "(compares two comma separated flags)");
	node->callback = &config_core_callback;
	node = config_set("core.loop", "(loop n times to label (core.loop = 3,foo))");
	node->callback = &config_core_callback;
	node = config_set("core.label", "(create a new label)");
	node->callback = &config_core_callback;
	node = config_set("core.break", "(breaks a loop)");
	node->callback = &config_core_callback;
	node = config_set("core.list", "(list the code)");
	node->callback = &config_core_callback;
	node = config_set("core.load", "(loads code from a file)");
	node->callback = &config_core_callback;
	node = config_set("core.dump", "(dumps the to a file)");
	node->callback = &config_core_callback;
	node = config_set("core.reset", "(resets code)");
	node->callback = &config_core_callback;
#endif

	/* lock */
	config_lock(1);
	vm_init(1);

	radis_update();

	if (first) {
		data_comment_init(1);
		radare_hack_init();
		trace_init();
		ranges_init();
	}
}

void config_visual_hit_i(const char *name, int delta)
{
	struct config_node_t *node;
	node = config_node_get(name);
	if (node && node->flags & CN_INT || node->flags & CN_OFFT)
		config_set_i(name, config_get_i(name)+delta);
}

/* Visually activate the config variable */
void config_visual_hit(const char *name)
{
	char buf[1024];
	struct config_node_t *node;
	node = config_node_get(name);
	if (node) {
		if (node->flags & CN_BOOL) {
			/* TOGGLE */
			node->i_value = !node->i_value;
			node->value = estrdup(node->value, node->i_value?"true":"false");
		} else {
			// FGETS AND SO
			cons_printf("New value (old=%s): ", node->value);
			cons_flushit();
			cons_set_raw(0);
			cons_fgets(buf, 1023, 0, 0);
			cons_set_raw(1);
			node->value = estrdup(node->value, buf);
		}
	}
}

/* Like emenu but for real */
void config_visual_menu()
{
	char cmd[1024];
	struct list_head *pos;
#define MAX_FORMAT 2
	const char *ptr;
	char *fs = NULL;
	char *fs2 = NULL;
	int option = 0;
	int _option = 0;
	int delta = 9;
	int menu = 0;
	int i,j, ch;
	int hit;
	int show;
	char old[1024];
	old[0]='\0';

	while(1) {
		cons_gotoxy(0,0);
		cons_clear();

		/* Execute visual prompt */
		ptr = config_get("cmd.vprompt");
		if (ptr&&ptr[0]) {
			int tmp = last_print_format;
			radare_cmd_raw(ptr, 0);
			last_print_format = tmp;
		}

		if (fs&&!memcmp(fs, "asm.", 4))
			radare_cmd_raw("pd 5", 0);

		switch(menu) {
			case 0: // flag space
				cons_printf("\n Eval spaces:\n\n");
				hit = 0;
				j = i = 0;
				list_for_each(pos, &(config_new.nodes)) {
					struct config_node_t *bt = list_entry(pos, struct config_node_t, list);
					if (option==i) {
						fs = bt->name;
						hit = 1;
					}
					show = 0;
					if (old[0]=='\0') {
						strccpy(old, bt->name, '.');
						show = 1;
					} else if (strccmp(old, bt->name, '.')) {
						strccpy(old, bt->name, '.');
						show = 1;
					}

					if (show) {
						if( (i >=option-delta) && ((i<option+delta)||((option<delta)&&(i<(delta<<1))))) {
							cons_printf(" %c  %s\n", (option==i)?'>':' ', old);
							j++;
						}
						i++;
					}
				}
				if (!hit && j>0) {
					option = j-1;
					continue;
				}
				cons_printf("\n Sel:%s \n\n", fs);
				break;
			case 1: // flag selection
				cons_printf("\n Eval variables: (%s)\n\n", fs);
				hit = 0;
				j = i = 0;
				// TODO: cut -d '.' -f 1 | sort | uniq !!!
				list_for_each(pos, &(config_new.nodes)) {
					struct config_node_t *bt = list_entry(pos, struct config_node_t, list);
					if (option==i) {
						fs2 = bt->name;
						hit = 1;
					}
					if (!strccmp(bt->name, fs, '.')) {
						if( (i >=option-delta) && ((i<option+delta)||((option<delta)&&(i<(delta<<1))))) {
							// TODO: Better align
							cons_printf(" %c  %s = %s\n", (option==i)?'>':' ', bt->name, bt->value);
							j++;
						}
						i++;
					}
				}
				if (!hit && j>0) {
					option = i-1;
					continue;
				}
				if (fs2 != NULL)
					cons_printf("\n Selected: %s\n\n", fs2);
		}
		cons_flushit();
		ch = cons_readchar();
		ch = cons_get_arrow(ch); // get ESC+char, return 'hjkl' char
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
			case 'q':
				if (menu<=0) return; menu--;
				break;
			case '*':
			case '+':
				if (fs2 != NULL)
					config_visual_hit_i(fs2, +1);
				continue;
			case '/':
			case '-':
				if (fs2 != NULL)
					config_visual_hit_i(fs2, -1);
				continue;
			case 'l':
			case 'e': // edit value
			case ' ':
			case '\r':
			case '\n': // never happens
				if (menu == 1) {
					if (fs2 != NULL)
						config_visual_hit(fs2);
				} else {
					flag_space_set(fs);
					menu = 1;
					_option = option;
					option = 0;
				}
				break;
			case '?':
				cons_clear00();
				cons_printf("\nVe: Visual Eval help:\n\n");
				cons_printf(" q     - quit menu\n");
				cons_printf(" j/k   - down/up keys\n");
				cons_printf(" h/b   - go back\n");
				cons_printf(" e/' ' - edit/toggle current variable\n");
				cons_printf(" +/-   - increase/decrease numeric value\n");
				cons_printf(" :     - enter command\n");
				cons_flushit();
				cons_any_key();
				break;
			case ':':
				cons_set_raw(0);
#if HAVE_LIB_READLINE
				char *ptr = readline(VISUAL_PROMPT);
				if (ptr) {
					strncpy(cmd, ptr, sizeof(cmd));
					radare_cmd(cmd, 1);
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
					cons_any_key();
				cons_gotoxy(0,0);
				cons_clear();
				continue;
		}
	}
}
