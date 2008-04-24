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
	int fd = -1;
	char *rdbdir;
	char *rdbfile;
	char *str =
		"# RDB (radare database) file\n"
		"chdir=\nchroot=\nsetuid=\nsetgid=\n";
/* FUCKY! */
// just set project name (cfg.project)

	rdbdir = config_get("dir.rdb");
	if (rdbdir&&rdbdir[0])
		chdir(rdbdir);

	rdbfile = config_get("file.rdb");
	if (!strnull(rdbfile))
		fd = open(rdbfile, O_APPEND|O_RDWR, 0644);
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
	config.baddr       = 0;
	config.seek        = 0;
	config.lines       = 0;
	config.debug       = 0;
	config.color	   = 0;
	config.unksize     = 0;
	config.buf         = 0; // output buffered
	config.ene         = 10;
	config.width       = terminal_get_columns();
	config.last_seek   = 0;
	config.file        = NULL;
	config.block_size  = DEFAULT_BLOCK_SIZE;
	config.cursor      = 0;
	config.ocursor     = -1;
	config.block       = (unsigned char *)malloc(config.block_size);
	config.verbose     = 1;
	config.interrupted = 1;
	config.visual      = 0;
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

void config_list(char *str)
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

struct config_node_t *config_node_get(char *name)
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

const char *config_get(const char *name)
{
	struct config_node_t *node;

	node = config_node_get(name);
	if (node) {
		if (node->flags & CN_BOOL)
			return !strcmp("true", node->value) || !strcmp("1", node->value);
		return node->value;
	}

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

	return NULL;
}

struct config_node_t *config_set(const char *name, const char *value)
{
	struct config_node_t *node;

	node = config_node_get(name);

	if (node) {
		if (node->flags & CN_RO) {
			eprintf("(read only)\n");
			return node;
		}
		free(node->value);
		if (node->flags & CN_BOOL) {
			int b = (!strcmp(value,"true")||!strcmp(value,"1"));
			node->i_value = (u64)b;
			node->value = strdup(b?"true":"false");
		} else {
			if (value == NULL) {
				node->value = strdup("");
				node->i_value = 0;
			} else {
				node->value = strdup(value);
				if (strchr(value, '/'))
					node->i_value = get_offset(value);
				else	node->i_value = get_math(value);
			}
		}
	} else {
		if (config_new.lock) {
			eprintf("(config-locked: '%s' no new keys can be created)\n", name);
		} else {
			node = config_node_new(name, value);
			if (value)
			if (!strcmp(value,"true")||!strcmp(value,"false"))
				node->flags|=CN_BOOL;
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
			return;
		free(node->value);
		sprintf(buf, "%ld", i); //0x%08lx", i);
		node->value = strdup(buf);
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

	if (str == NULL)
		return;
	name = strdup(str);

	str = strclean(name);

	if (str && (str[0]=='\0'||!strcmp(str, "help"))) {
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
			char * str = config_get(foo);
			
			cons_printf("%s\n", (str==1)?"true":(str==0)?"false":str);
		}
	}

	free(name);
}

static int config_zoombyte_callback(void *data)
{
	struct config_node_t *node = data;

	if (!strcmp(node->value, "head")) {
		// ok
	} else
	if (!strcmp(node->value, "flags")) {
		// ok
	} else
	if (!strcmp(node->value, "FF")) {
		// ok
	} else
	if (!strcmp(node->value, "entropy")) {
		// ok
	} else
	if (!strcmp(node->value, "print")) {
		// ok
	} else
	if (!strcmp(node->value, "printable")) {
		// ok
	} else {
		free(node->value);
		node->value = strdup("head");
	}
}

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

static int config_arch_callback(void *data)
{
	arch_set_callbacks();
}

static int config_wmode_callback(void *data)
{
	struct config_node_t *node = data;

	//if (node && node->i_value)
	// XXX: strange magic conditional
	if (config.fd != -1 && config.file && !config.debug) // && config_new.lock)
		radare_open(0);

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
	else	config_set("cmd.asm", "");
	return 1;
}
#endif

static int config_baddr_callback(void *data)
{
	struct config_node_t *node = data;

	if (node && node->i_value)
		config.baddr = (u64)node->i_value;
	return 1;
}

static int config_scrheight(void *data)
{
	struct config_node_t *node = data;
	config.height = node->i_value;
	if (config.height<1)
		config.height = 24;
	return 1;
}

static int config_scrbuf_callback(void *data)
{
	struct config_node_t *node = data;

	config.buf = node->i_value;
}

static int config_bsize_callback(void *data)
{
	struct config_node_t *node = data;

	if (node->i_value)
		radare_set_block_size_i(node->i_value);
/*
	else
		cons_printf("(ignored)");
*/
	// TODO more work
}

void config_lock(int l)
{
	config_new.lock = l;
}

void config_init()
{
	struct config_node_t *node;

	flags_init();
	config_old_init();

	dl_init();
	dl_hist_load(".radare_history");

	config_new.n_nodes = 0;
	config_new.lock = 0;
	INIT_LIST_HEAD(&(config_new.nodes));

	/* enter keys */
#if __x86_64__
	node = config_set("asm.arch", "intel64");
#else
#if __arm__
	node = config_set("asm.arch", "arm");
#else
#if __mips__
	node = config_set("asm.arch", "mips");
#else
#if __POWERPC__
	node = config_set("asm.arch", "ppc");
#else
	node = config_set("asm.arch", "intel");
#endif
#endif
#endif
#endif
	node->callback = &config_arch_callback;
	config_set("asm.syntax", "pseudo");
	config_set("asm.xrefs", "xrefs");
	config_set("asm.objdump", "objdump -m i386 --target=binary -D");
	config_set("asm.offset", "true"); // show offset
	config_set_i("asm.nbytes", 8); // show hex bytes
	config_set("asm.bytes", "true"); // show hex bytes
	config_set("asm.flags", "true"); // show hex bytes
	config_set("asm.flagsline", "false"); // show hex bytes
	config_set("asm.lines", "true"); // show left ref lines
	config_set_i("asm.nlines", 6); // show left ref lines
	config_set("asm.lineswide", "true"); // show left ref lines
	config_set("asm.trace", "false"); // trace counter
	config_set("asm.linesout", "false"); // show left ref lines
	config_set("asm.linestyle", "false"); // foreach / prev
	config_set("asm.comments", "true"); // show comments in disassembly
	config_set_i("asm.cmtmargin", 27); // show comments in disassembly
	config_set_i("asm.cmtlines", 0); // show comments in disassembly
	config_set("asm.split", "true"); // split code blocks
	config_set("asm.splitall", "false"); // split code blocks
	config_set("asm.size", "false"); // opcode size

	config_set("asm.follow", "");
	config_set("cmd.asm", "");
	config_set("cmd.user", "");
	config_set("cmd.visual", "");
	config_set("cmd.hit", "");
	config_set("cmd.prompt", "");
	config_set("cmd.vprompt", "p%");
	config_set("cmd.bp", "");

	config_set("search.flag", "true");
	config_set("search.verbose", "true");

	config_set("file.id", "false");
	config_set("file.type", "");
	config_set("file.flag", "false");
	config_set("file.trace", "trace.log");
	config_set("file.project", "");
	config_set("file.entrypoint", "");
	config_set("file.rdb", "");
	config_set("file.scrfilter", "");
	config_set_i("file.size", 0);
	node = config_set_i("file.baddr", 0);
	node->callback = &config_baddr_callback;

	config_set("trace.bt", "false");
	config_set_i("trace.sleep", 0);
	config_set("trace.smart", "false");
	config_set("trace.libs", "true");
	config_set("trace.log", "false");
	config_set("trace.dup", "false");
	config_set("trace.cmtregs", "false");

	config_set("cfg.noscript", "false");
	config_set("cfg.encoding", "ascii"); // cp850
	config_set_i("cfg.delta", 1024); // cp850
	config_set("cfg.verbose", "true");
#if LIL_ENDIAN
	config_set("cfg.endian", "false");
#else
	config_set("cfg.endian", "true");
#endif
	config_set("cfg.inverse", "false");
	config_set("file.insert", "false");
	node = config_set("file.write", "false");
	node->callback = &config_wmode_callback;
	config_set("cfg.limit", "0");
	config_set("cfg.rdbdir", "TODO");
	config_set("cfg.datefmt", "%d:%m:%Y %H:%M:%S %z");
	config_set_i("cfg.count", 0);
	config_set("cfg.fortunes", "true");
	node = config_set_i("cfg.bsize", 512);
	node->callback = &config_bsize_callback;
	config_set_i("cfg.vbsize", 1024);
	config_set("cfg.vbsize_enabled", "false");

	config_set("child.stdin", "");
	config_set("child.stdout", "");
	config_set("child.stderr", "");
	config_set_i("child.setgid", "");
	config_set("child.chdir", ".");
	config_set("child.chroot", "/");
	config_set("child.setuid", "");

	config_set("dbg.syms", "true");
	config_set("dbg.dwarf", "true");
	config_set("dbg.maps", "true");
	config_set("dbg.strings", "false");
	config_set("dbg.stop", "false");
	config_set("dbg.contscbt", "true");
	config_set("dbg.regs", "true");
	config_set("dbg.stack", "true");
	config_set("dbg.vstack", "true");
	config_set_i("dbg.stacksize", 66);
	config_set("dbg.stackreg", "esp");
	config_set("dbg.bt", "false");
	config_set("dbg.fullbt", "false"); // user backtrace or lib+user backtrace
	config_set("dbg.bttype", "default"); // default, st and orig or so!
	config_set("dbg.bptype", "hard"); // only soft vs hard
	config_set("dbg.bep", "loader"); // loader, main

	config_set("dir.home", getenv("HOME"));
	config_set("dir.monitor", getenv("MONITORPATH"));
	config_set("dir.plugins", LIBDIR"/radare/");
	config_set("dir.rdb", ""); // ~/.radare/rdb/
	config_set("dir.tmp", "/tmp/");

	config_set("graph.color", "magic");
	config_set("graph.callblocks", "false");
	config_set("graph.flagblocks", "true");
	config_set_i("graph.depth", 7);
	config_set("graph.jmpblocks", "true");
	config_set("graph.offset", "false");
	config_set("graph.render", "cairo"); // aalib/ncurses/text

	node = config_set_i("zoom.from", 0);
	node = config_set_i("zoom.size", config.size);
	node = config_set("zoom.byte", "head");
	node->callback = &config_zoombyte_callback;

	config_set_i("scr.accel", 0);
	config_set("scr.seek", "");
	node = config_set("scr.color", (config.color)?"true":"false");
	node->callback = &config_color_callback;
	node = config_set("scr.buf", "false");
	node->callback = &config_scrbuf_callback;
	config_set_i("scr.width", config.width);
	node = config_set_i("scr.height", config.height);
	node->callback = &config_scrheight;
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
	metadata_comment_init(1);
	radare_hack_init();
	trace_init();
}
