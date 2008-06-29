#ifndef _INCLUDE_RADARE_H_
#define _INCLUDE_RADARE_H_

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#define __addr_t_defined

// basic data types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef u64 addr_t;

#ifndef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8
#endif
#include "cmds.h"
#if 0
#if SIZEOF_OFF_T == 1
#warning Oops. SIZEOF_OFF_T = 1 means invalid autodetection. Using 8
#endif
#endif

// conditional goto
enum {
	OP_JE,
	OP_JNE,
	OP_JA,
	OP_JB
};

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include "cmds.h"
#include "print.h"

extern struct list_head hacks;
extern struct list_head traces;

int trace_count(u64 addr);
int trace_times(u64 addr);
int trace_add(u64 addr);
void trace_init();

struct trace_t {
	u64 addr;
	int times;
	int count;
	struct timeval tm;
	struct list_head list;
};

enum {
	DATA_HEX    = FMT_HEXB, /* hex byte pairs */
	DATA_STR    = FMT_ASC0, /* ascii string */
	DATA_CODE   = FMT_UDIS, /* plain assembly code */
	DATA_FOLD_O = 0x100,    /* open folder */
	DATA_FOLD_C = 0x101,    /* closed folder */
	DATA_EXPAND = 0x200 
};


#undef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8

#if SIZEOF_OFF_T == 8
#ifdef OFF_FMT
#undef OFF_FMT
#endif
#define OFF_FMT "0x%08llX"
#define OFF_FMTs "0x%08llX"
#define OFF_FMTx "%llX"
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

int java_disasm(unsigned char *bytes, char *output);

void metadata_comment_add(u64 offset, const char *str);
void metadata_comment_del(u64 offset, const char *str);
void metadata_comment_list();
char *metadata_comment_get(u64 offset);
void metadata_comment_init(int new);

#define uchar unsigned char
extern int fixed_width;
extern int inc;
void monitors_run();
void endian_memcpy(unsigned char *dest, unsigned char *orig, unsigned int size);
void radare_init();
int radare_go();
void radare_exit();
void radare_resize(const char *arg);
void radare_prompt_command();
void radare_sync();
int radare_search (const unsigned char *arg, unsigned int slen, print_fmt_t print_fmt);
int stripstr_from_file(const char *filename, int min, u64 seek);
void radare_search_set_mask (const unsigned char *arg, unsigned int slen , unsigned char op);
int radare_strsearch(char *str);
int radare_cmd(char *command, int log);
int radare_cmd_raw(const char *tmp, int log);
char *pipe_command_to_string(char *cmd);
char *radare_cmd_str(const char *cmd);
int radare_interpret(char *file);
void radare_controlc();
void radare_controlc_end();
void init_environment();
void update_environment();
void prepare_environment(char *line);
void destroy_environment(char *line);
void udis(int len, int rows);
void label_show();
int monitor_init();
void radare_move(char *arg);
void disassemble(int len, int rows);
void radare_search_aes();
int aes_key_test(unsigned char *buf);

int project_save(const char *file);
void project_close();
int project_open(const char *file);
int project_info(const char *file);

int rabin_load();

int radare_hack_help();
int radare_hack(const char *cmd);
void rdb_help();
void radare_fortunes();
int radare_compare(unsigned char *f, unsigned char *d, int len);
int search_range(char *range);
int search_from_file(char *file);
int radare_read_at(u64 offset, unsigned char *data, int len);
int radare_write_at(u64 offset, unsigned char *data, int len);
int radare_write(char *arg, int mode);
void radare_poke(const char *arg);

#endif
