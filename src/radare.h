#ifndef _INCLUDE_RADARE_H_
#define _INCLUDE_RADARE_H_

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#ifndef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8
#endif
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
#if __WINDOWS__
#undef off_t
#define off_t long long
#endif

#include "list.h"
#include <sys/types.h>
#include <fcntl.h>
#include "cmds.h"
#include "print.h"

extern struct list_head comments;
struct comment_t {
	off_t offset;
	char *comment;
	struct list_head list;
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


#define LANG_PERL 1
#define LANG_PYTHON 2

int java_disasm(unsigned char *bytes, char *output);

void metadata_comment_add(off_t offset, const char *str);
void metadata_comment_del(off_t offset);
char *metadata_comment_list();
char *metadata_comment_get(off_t offset);
void metadata_comment_init(int new);

#define uchar unsigned char
extern int fixed_width;
extern int inc;
void endian_memcpy(unsigned char *dest, unsigned char *orig, unsigned int size);
int radare_go();
void radare_prompt_command();
void radare_sync();
int radare_search (const unsigned char *arg, unsigned int slen, print_fmt_t print_fmt);
int stripstr_from_file(const char *filename, int min, off_t seek);
void radare_search_set_mask (const unsigned char *arg, unsigned int slen , unsigned char op);
int radare_strsearch(char *str);
int radare_command(char *input, int log);
int radare_interpret(char *file);
void radare_controlc();
void radare_controlc_end();
void init_environment();
void update_environment();
void prepare_environment(char *line);
void destroy_environment(char *line);
void udis(int len, int rows);
char *hist_get_i(int p);
int hist_show();
void label_show();
void hist_add(char *str, int log);
void hist_clean();
int monitor_init();
void radare_move(char *arg);
void disassemble(int len, int rows);
void radare_search_aes();
int aes_key_test(unsigned char *buf);

CMD_DECL(gotoxy);
CMD_DECL(baddr);
CMD_DECL(seek0);
CMD_DECL(hack);
CMD_DECL(store);
CMD_DECL(blocksize);
CMD_DECL(count);
CMD_DECL(comment);
CMD_DECL(show_info);
CMD_DECL(envvar);
CMD_DECL(compare);
CMD_DECL(dump);
CMD_DECL(endianess);
CMD_DECL(limit);
CMD_DECL(move);
CMD_DECL(print);
CMD_DECL(quit);
CMD_DECL(resize);
CMD_DECL(seek);
CMD_DECL(undoseek);
CMD_DECL(status);
CMD_DECL(rdb);
CMD_DECL(project);
CMD_DECL(yank);
CMD_DECL(yank_paste);
CMD_DECL(visual);
CMD_DECL(write);
CMD_DECL(examine);
CMD_DECL(prev);
CMD_DECL(next);
CMD_DECL(prev_align);
CMD_DECL(next_align);
CMD_DECL(search);
CMD_DECL(shell); 
CMD_DECL(cmd);
CMD_DECL(help);
CMD_DECL(flag);
CMD_DECL(interpret);
CMD_DECL(interpret_perl);
CMD_DECL(echo);
CMD_DECL(open);
CMD_DECL(math);
CMD_DECL(width);
CMD_DECL(hash);
CMD_DECL(config_eval);
CMD_DECL(default);
#endif
