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

int iswhitespace(char ch)
{
   return (ch==' '||ch=='\t');
}

#if HAVE_LIB_READLINE

#include <stdio.h>
#include <stdlib.h>
#include "readline.h"
#include "cmds.h"
#include "list.h"
#include "config.h"
#include "radare.h"
#include "flags.h"

int rad_rl_init=0;
extern command_t *commands[];

/*
 * Count the number of words in the given string
 *
 */
int word_count(const char *string)
{
	char *text = (char *)string;
	char *tmp  = (char *)string;
	int word   = 0;

	for(;(*text)&&(iswhitespace(*text));text=text+1);

	for(word = 0; *text; word++) {
		for(;*text && !iswhitespace(*text);text = text +1);
		tmp = text;
		for(;*text &&iswhitespace(*text);text = text +1);
		if (tmp == text)
			word-=1;
	}

	return word-1;
}

char *get_first_word(const char *string)
{
	char *text  = (char *)string;
	char *start = NULL;
	char *ret   = NULL;
	int len     = 0;

	for(;*text &&iswhitespace(*text);text = text + 1);
	start = text;
	for(;*text &&!iswhitespace(*text);text = text + 1) len++;

	/* strdup */
	ret = (char *)malloc(len+1);
	if (ret == 0) {
		fprintf(stderr, "Cannot allocate %d bytes.\n", len+1);
		exit(1);
	}
	strncpy(ret, start, len);
	ret[len]='\0';

	return ret;
}

enum {
	ARG_NULL = 0,
	ARG_COMMAND,
	ARG_FLAG,
	ARG_FILENAME,
	ARG_NUMBER,
	ARG_OFFSETS,
	ARG_BOOL,
	ARG_ARCH,
	ARG_EVAL
};

typedef struct {
	char *name;
	int arg1;
	int arg2;
} cmd_t;

cmd_t cmds[] = {
#if DEBUGGER
	{ "!?"        , ARG_NULL     , ARG_NULL   } , 
	{ "!help"        , ARG_NULL     , ARG_NULL   } , 
	{ "!pid"         , ARG_NULL     , ARG_NULL   } , 
	{ "!info"        , ARG_NULL     , ARG_NULL   } , 
	{ "!alloc"        , ARG_NULL     , ARG_NULL   } , 
	{ "!mmap"        , ARG_NULL     , ARG_NULL   } , 
	{ "!imap"        , ARG_NULL     , ARG_NULL   } , 
	{ "!attach"        , ARG_NUMBER, ARG_NULL   } , 
	{ "!dettach"        , ARG_NULL     , ARG_NULL   } , 
	{ "!free", ARG_NULL     , ARG_NULL   } , 
	{ "!dump"        , ARG_NULL     , ARG_NULL   } , 
	{ "!fd"     , ARG_NUMBER , ARG_FILENAME } , 
	{ "!th"     , ARG_NULL     , ARG_NULL   } , 
	{ "!th"     , ARG_NULL     , ARG_NULL   } , 
	{ "!restore"     , ARG_NULL     , ARG_NULL   } , 
	{ "!load"        , ARG_NULL     , ARG_NULL   } , 
	{ "!unload"      , ARG_NULL     , ARG_NULL   } , 
	{ "!trace"       , ARG_NULL     , ARG_NULL   } , 
	{ "!dr"          , ARG_NUMBER, ARG_NULL   } , 
	{ "!drx"         , ARG_NUMBER, ARG_NULL   } , 
	{ "!drw"         , ARG_NUMBER , ARG_NULL   } , 
	{ "!drr"         , ARG_NUMBER     , ARG_NULL   } , 
	{ "!jmp"         , ARG_NUMBER, ARG_NULL   } , 
	{ "!call"        , ARG_NUMBER, ARG_NULL   } , 
	{ "!signal"      , ARG_NULL     , ARG_NULL   } , 
	{ "!step"        , ARG_NULL     , ARG_NULL   } , 
	{ "!stepu"       , ARG_NULL     , ARG_NULL   } , 
	{ "!stepo"       , ARG_NULL     , ARG_NULL   } , 
	{ "!stepall"     , ARG_NULL     , ARG_NULL   } , 
	{ "!cont"        , ARG_NULL     , ARG_NULL   } , 
	{ "!contu"       , ARG_NULL, ARG_NULL   } , 
	{ "!contsc"      , ARG_NULL     , ARG_NULL   } , 
	{ "!contfork"    , ARG_NULL, ARG_NULL   } , 
	{ "!syms"        , ARG_NULL     , ARG_NULL   } , 
	{ "!pstree"       , ARG_NULL     , ARG_NULL   } , 
	{ "!pid"       , ARG_NULL     , ARG_NULL   } , 
	{ "!maps"        , ARG_NULL     , ARG_NULL   } , 
	{ "!regs"        , ARG_NULL     , ARG_NULL   } , 
	{ "!regs*"       , ARG_NULL     , ARG_NULL   } , 
	{ "!lregs"       , ARG_NULL     , ARG_NULL   } , 
	{ "!oregs"       , ARG_NULL     , ARG_NULL   } , 
	{ "!bp"          , ARG_NUMBER   , ARG_NULL   } , 
	{ "!wp"          , ARG_NUMBER   , ARG_NULL   } , 
#else
	{ "!"            , ARG_FILENAME , ARG_NULL   } , 
#endif
	{ "!rsc"         , ARG_NULL     , ARG_NULL   } , 
	{ "%SEARCH["     , ARG_NUMBER   , ARG_NULL   } , 
	{ "%SEARCH[0]"   , ARG_NUMBER   , ARG_NULL   } , 
//	{ "Baddr"        , ARG_NUMBER   , ARG_NULL   } , 
	{ "Comment"      , ARG_FILENAME , ARG_NULL   } , 
	{ "bsize"        , ARG_NUMBER   , ARG_NULL   } , 
	{ "flag"         , ARG_FLAG     , ARG_NULL   } , 
	{ "help"         , ARG_NULL     , ARG_NULL   } , 
	{ "yank"         , ARG_NULL     , ARG_NULL   } , 
	{ "Ypaste"       , ARG_NULL     , ARG_NULL   } , 
	{ "s"            , ARG_NUMBER   , ARG_NULL   } , 
	{ "seek"         , ARG_NUMBER   , ARG_NULL   } , 
	{ "resize"       , ARG_NUMBER   , ARG_NULL   } , 
	{ "move"         , ARG_NUMBER   , ARG_NUMBER } , 
	{ "e"            , ARG_EVAL     , ARG_NULL   } , 
	{ "eval"         , ARG_EVAL     , ARG_NULL   } , 
	{ "x"            , ARG_NULL     , ARG_NULL   } , 
	{ "w"            , ARG_NULL     , ARG_NULL   } , 
	{ "wa"           , ARG_NULL     , ARG_NULL   } , 
	{ "wx"           , ARG_NULL     , ARG_NULL   } , 
	{ "ww"           , ARG_NULL     , ARG_NULL   } , 
	{ "wf"           , ARG_NULL     , ARG_NULL   } , 
	{ "Visual"       , ARG_NULL     , ARG_NULL   } , 
	{ "W"            , ARG_NUMBER   , ARG_NULL   } , 
	{ "."            , ARG_FILENAME , ARG_NULL   } , 
	{ "./s *"        , ARG_NULL     , ARG_NULL   } , 
	{ "open"         , ARG_FILENAME , ARG_NULL   } , 
	{ "p"            , ARG_NULL     , ARG_NULL   } , 
	{ "pIx"          , ARG_NUMBER   , ARG_NULL   } ,
	{ "pIX"          , ARG_NUMBER   , ARG_NULL   } ,
	{ "pIo"          , ARG_NUMBER   , ARG_NULL   } ,
	{ "pa"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pb"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pc"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pd"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pr"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "ps"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pS"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pz"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "po"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pO"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pu"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pU"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "px"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pX"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pq"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pw"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pW"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pf"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pt"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pT"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pi"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pl"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "pL"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "R"            , ARG_FILENAME , ARG_NULL   } , 
	{ "Rm"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "Rg"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "Rd"           , ARG_NUMBER   , ARG_NULL   } , 
	{ "Hack"         , ARG_NUMBER,   ARG_NULL   } , 
	{ "P"            , ARG_NULL     , ARG_NULL   } , 
	{ "Ps"           , ARG_FILENAME , ARG_NULL   } , 
	{ "Po"           , ARG_FILENAME , ARG_NULL   } , 
	{ "Pi"           , ARG_FILENAME , ARG_NULL   } , 
	{ "undo"         , ARG_NULL     , ARG_NULL   } , 
	{ "ulist"        , ARG_NULL     , ARG_NULL   } , 
	{ "uu"           , ARG_NULL     , ARG_NULL   } , 
	{ "u?"           , ARG_NULL     , ARG_NULL   } , 
	{ "u!"           , ARG_NULL     , ARG_NULL   } , 
	{ "u*"           , ARG_NULL     , ARG_NULL   } , 
	{ "#!perl"       , ARG_NULL     , ARG_NULL   } , 
	{ "#!python"     , ARG_NULL     , ARG_NULL   } , 
	{ "#all"         , ARG_NULL     , ARG_NULL   } , 
	{ "#par"         , ARG_NULL     , ARG_NULL   } , 
	{ "#xor"         , ARG_NULL     , ARG_NULL   } , 
	{ "#xorpair"     , ARG_NULL     , ARG_NULL   } , 
	{ "#mod255"      , ARG_NULL     , ARG_NULL   } , 
	{ "#crc16"       , ARG_NULL     , ARG_NULL   } , 
	{ "#crc32"       , ARG_NULL     , ARG_NULL   } , 
	{ "#md4"         , ARG_NULL     , ARG_NULL   } , 
	{ "#md5"         , ARG_NULL     , ARG_NULL   } , 
	{ "#sha1"        , ARG_NULL     , ARG_NULL   } , 
	{ "#sha256"      , ARG_NULL     , ARG_NULL   } , 
	{ "#sha384"      , ARG_NULL     , ARG_NULL   } , 
	{ "#sha512"      , ARG_NULL     , ARG_NULL   } , 
	{ "#entropy"     , ARG_NULL     , ARG_NULL   } , 
	{ "#hamdist"     , ARG_NULL     , ARG_NULL   } , 
	{ "?"            , ARG_NUMBER   , ARG_NULL   } , 
	{ "/"            , ARG_NULL     , ARG_NULL   } , 
	{ "/."           , ARG_FILENAME , ARG_NULL   } , 
	{ "//"           , ARG_NULL     , ARG_NULL   } , 
	{ "/a"           , ARG_NULL     , ARG_NULL   } , 
	{ "/r"           , ARG_NULL     , ARG_NULL   } , 
	{ "/s"           , ARG_NULL     , ARG_NULL   } , 
	{ "/x"           , ARG_NULL     , ARG_NULL   } , 
	{ "/m"           , ARG_NUMBER , ARG_FILENAME } , 
	{ "/k"           , ARG_NUMBER , ARG_FILENAME } , 
	{ "+"            , ARG_NUMBER   , ARG_NULL   } , 
	{ "-"            , ARG_NUMBER   , ARG_NULL   } , 
	{ ">"            , ARG_NULL     , ARG_NULL   } , 
	{ "<"            , ARG_NULL     , ARG_NULL   } , 
	{ "info"         , ARG_NULL     , ARG_NULL   } , 
	{ "quit"         , ARG_NULL     , ARG_NULL   } , 
	{ 0              , 0            , 0 }
};

/*
 * Convert from hex to dec when autocompleting a number
 * TODO: Support for 64 bits
 *
 */
char *rad_offset_matches(const char *text, int state)
{
	static int i, len;
	char buf[1024];
	int n;

	if (!state) {
		len = strlen(text);
		i   = 0;

		if (state == 0) {
			if (text[1]=='x') {
				sscanf(text, "0x%x", &n);
				sprintf(buf, "%d", n);
			} else {
				sprintf(buf, "0x%x", atoi(text));
			}
			return strdup(buf);
		}
	}

	return ((char *)NULL);
}

/*
 * Find matching configuration variable names
 *
 */
char *rad_eval_matches(const char *text, int state)
{
	static struct list_head *pos;
	static int len;

	if (!state) {
		len = strlen(text);
		pos = (&(config_new.nodes))->next;
		//prefetch(pos->next);
	}

	for (; pos != (&(config_new.nodes)); pos = pos->next) { //, prefetch(pos->next)) {
		struct config_node_t *bt = list_entry(pos, struct config_node_t, list);
		if (strncmp(text, bt->name, len) == 0) {
			pos = pos->next;
			return strdup(bt->name);
		}
	}

	return ((char *)NULL);
}

/*
 * Find matching flags
 *
 */
char *rad_flags_matches(const char *text, int state)
{
	static int i, len;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while(i<nflags) {
		if (strncmp (text, flags[i]->name, len) == 0)
			return strdup(flags[i++]->name);
		i++;
	}

	return ((char *)NULL);
}

/*
 * Boolean autocompletion
 *
 */
char *rad_bool_matches(const char *text, int state)
{
	static int i, len;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while(i<2)
		if (i++)
			return strdup("1");
		else
			return strdup("0");

	return ((char *)NULL);
}

#define narchs 3
static unsigned char *arches[] = {
	"arm", "intel", "java"
};

char *rad_arch_matches(const char *text, int state)
{
	static int i, len;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while(i<narchs) {
		if (strncmp (text, arches[i], len) == 0)
			return strdup(arches[i++]);
		i++;
	}

	return ((char *)NULL);
}

/*
 * Find matching commands
 *
 */
char *rad_command_matches(const char *text, int state)
{
	static int i=0, len;
	char *name;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while((name = cmds[i].name)!=NULL) {
		i++;
		if (strncmp (name, text, len) == 0)
			return strdup(name);
	}

	return ((char *)NULL);
}

/*
 * Function to autocomplete radare commands
 *
 */
char **rad_autocompletion(const char *text, int start, int end)
{
	char **matches = (char **)NULL;
	char *word = NULL;
	int w;
	int i;

	w = word_count(rl_line_buffer);
   
	/* keyword exists */
	switch(w) {
	case -1: // null string
		matches = rl_completion_matches (text, rad_command_matches);
		break;
	case 0: // first word
		word = get_first_word(rl_line_buffer);
		if (word == NULL) break;
		for (i = 0; cmds[i].name ; i++) {
			if (!strcmp(cmds[i].name, word))
			switch(cmds[i].arg1) {
			case ARG_EVAL:
				matches = rl_completion_matches(text, rad_eval_matches);
				break;
			case ARG_NUMBER:
				matches = rl_completion_matches(text, rad_flags_matches);
				if (matches == NULL)
				matches = rl_completion_matches (text, rad_offset_matches);
				break;
			case ARG_FLAG:
				matches = rl_completion_matches(text, rad_flags_matches);
				break;
			case ARG_FILENAME:
				return NULL;
			case ARG_BOOL:
				matches = rl_completion_matches(text, rad_bool_matches);
				break;
			case ARG_ARCH:
				matches = rl_completion_matches(text, rad_arch_matches);
				break;
			default: // no reply
				return NULL;
			}
		}
		break;
	case 1:
		word = get_first_word(rl_line_buffer);
		if (word == NULL) break;
		for (i = 0; cmds[i].name ; i++) {
			if (!strcmp(cmds[i].name, word))
			switch(cmds[i].arg2) {
			case ARG_NUMBER:
				matches = rl_completion_matches(text, rad_flags_matches);
				if (matches == NULL)
				matches = rl_completion_matches (text, rad_offset_matches);
				break;
			case ARG_FLAG:
				matches = rl_completion_matches(text, rad_flags_matches);
				break;
			// autocomplete with search results offsets
			case ARG_FILENAME:
				return NULL;
			case ARG_BOOL:
				matches = rl_completion_matches(text, rad_bool_matches);
				break;
			default: // no reply
				return NULL;
			}
		}
		break;
	}

	free(word);
	return matches;
}

char history_file[1024];

void rad_readline_finish()
{
	write_history(history_file);
}

void rad_readline_init()
{
	if (rad_rl_init)
		return;

	rl_initialize();
//	rl_set_prompt("~> ");
	rl_redisplay();
	rl_attempted_completion_function = rad_autocompletion;

	snprintf(history_file, 1000, "%s/.radare_history", getenv("HOME"));
	read_history(history_file);
//	rl_completion_entry_function = NULL;
	rad_rl_init = 1;
}

#endif