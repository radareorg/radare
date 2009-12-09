#ifndef _INCLUDE_RADARE_H_
#define _INCLUDE_RADARE_H_

#define __addr_t_defined
#include "main.h"

#include "../global.h"
#include "types.h"
#include "cmds.h"

#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>

extern int search_nocase;
extern struct list_head hacks;
#undef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8

#if SIZEOF_OFF_T == 8
#ifdef OFF_FMT
#undef OFF_FMT
#endif
#define OFF_FMT "0x%08llX"
#undef OFF_FMTs
#define OFF_FMTs "0x%08llX"
#undef OFF_FMTx
#define OFF_FMTx "%llX"
#undef OFF_FMTd
#define OFF_FMTd "%lld"
#define offtd long long
#define offtx long long
#else
#define OFF_FMT "%08X"
#define OFF_FMTx "%X"
#define OFF_FMTs "0x%08llX"
#define OFF_FMTd "%d"
#define offtd int
#define offtx unsigned int
#endif

#if __FreeBSD__ || __OpenBSD__ || __NetBSD__
#define __BSD__ 1
#else
#define __BSD__ 0
#endif

#define LANG_PERL 1
#define LANG_PYTHON 2

// conditional goto
enum {
	OP_JE,
	OP_JNE,
	OP_JA,
	OP_JB
};

int java_disasm(const u8 *bytes, char *output);

void metadata_comment_add(ut64 offset, const char *str);
void metadata_comment_del(ut64 offset, const char *str);
void metadata_comment_list();
void metadata_comment_init(int new);

extern int fixed_width;
extern int inc;
void monitors_run();
void radare_init();
int radare_go();
void radare_exit(int ret);
void radare_resize(const char *arg);
void radare_prompt_command();
void radare_sync();
int radare_search (const unsigned char *arg, unsigned int slen, print_fmt_t print_fmt);
int stripstr_from_file(const char *filename, int min, int max, int encoding, ut64 seek, ut64 limit);
int stripstr_iterate(const u8 *buf, int i, int min, int max, int enc, ut64 offset, const char *match);
void radare_search_set_mask (const unsigned char *arg, unsigned int slen , unsigned char op);
int radare_strsearch(const char *str);
int radare_cmd(const char *command, int log);
int radare_cmd_raw(const char *tmp, int log);
char *pipe_command_to_string(const char *cmd);
char *radare_cmd_str(const char *cmd);
int radare_interpret(const char *file);
void radare_controlc();
void radare_controlc_end();
void udis(int len, int rows);
int monitor_init();
int radare_move(char *arg);
void disassemble(int len, int rows);
void radare_search_aes();
int aes_key_test(unsigned char *buf);
int radare_system(const char *cmd);

const char *project_get_file(const char *file);
int project_save(const char *file);
void project_close();
int project_open(const char *file);
int project_info(const char *file);

int rabin_load();

void radare_set_block_size(char *arg);
int radare_set_block_size_i(int sz);
int radare_hack_help();
void help_message_short();
int radare_hack(const char *cmd);
void rdb_help();
void radare_fortunes();
int radare_compare(unsigned char *f, unsigned char *d, int len);
int radare_compare_hex(ut64 addr, unsigned char *f, unsigned char *d, int len);
int search_range(char *range);
int search_from_file(char *file);
int radare_read_at(ut64 offset, unsigned char *data, int len);
int radare_write_at(ut64 offset, const u8 *data, int len);
int radare_write(const char *arg, int mode);
int radare_write_xor(const char *arg);
void radare_poke(const char *arg);
int radare_get_region(ut64 *from, ut64 *to);
int rasm_asm(const char *arch, ut64 *offset, const char *str, unsigned char *data);
int radare_cmdf(const char *cmd, ...);
int radare_systemf(const char *format, ...);
u8 *radare_block();

int resolve_encoding(const char *name);
int trace_get_between(ut64 from, ut64 to);
struct trace_t *trace_get(ut64 addr, int tag);
int radare_close();

/* rabin.h */
int rabin_id();
int rabin_flag();
int java_classdump(const char *file);

/* macros */
void radare_macro_init();
int radare_macro_add(const char *name);
int radare_macro_rm(const char *name);
int radare_macro_list();

/* env.h */
void env_init();
void env_update();
void env_prepare(const char *line);
void env_destroy(const char *line);
int radare_compare_code(ut64 off, const u8 *a, int len);
//int env_var_required(const char *str, const char *var);
int radare_search_replace(const char *input, int hex);

/* user graphs */
void ugraph_reset();
void ugraph_node(ut64 from, ut64 size, const char *cmd);
void ugraph_edge(ut64 from, ut64 to);
struct ugraph_node_t *ugraph_get(ut64 addr);

int radare_seek_search(const char *str);
int radare_seek_search_backward(const char *str);

#endif
