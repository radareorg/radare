#ifndef _INCLUDE_UTILS_H_
#define _INCLUDE_UTILS_H_

#include <sys/types.h>
#include "radare.h"
#include "print.h"

#define uint unsigned int

int _print_fd;
extern const char hex[16];

#define TMPFILE_MAX 50
#define BUFLEN 4096
#define CMPMIN(a,b) (a<b? a : b)

void eprintf(const char *format, ...);
void pprintf(const char *format, ...);
char *estrdup(char *ptr, char *string);

enum {
	DUMP_MAGIC,
	DUMP_DISASM,
	DUMP_USER
};

#define CHECK_TMP_FILE( file )\
	if (*file == '\0') {\
		perror("mkstemp");\
		unlink( file );\
		return;\
	}

#define go_alarm(x) signal(SIGALRM, x); if (x!=SIG_IGN) alarm(1);

// XXX this is ugly. seek and destroy xrefs to this macro
#define clear_string(str) {         \
	int i; \
	for (i=0;str[i]&&str[i]!=' ';i++);  \
	for (;str[i]&&str[i]==' ';i++);     \
	if (i>0) strcpy(str,str+i); \
	for(i=0;str[i];i++) {       \
		if (!is_printable(str[i])||str[i]=='\t'||str[i]=='\n'||str[i]==' ') { \
 			if (i>0&&str[i-1]!='\\') { \
				str[i]='\0';       \
				break;                       \
			}                               \
		} \
	} \
}

extern off_t tmpoff;
extern int std;
extern char **environ;
extern char *last_tsearch;
void radare_dump(char *arg, int size);
void radare_dump_and_process(int type, int size);
int pipe_stdout_to_tmp_file(char *tmp, char *cmd);
int pprint_fd(int fd);
int make_tmp_file(char *str);
void progressbar(int pc);
void terminal_set_raw(int b);
int radare_read(int next);
off_t get_offset (char *arg);
char *lstrchr(char *str, char chr);
off_t get_math(const char* text);
void print_msdos_date(unsigned char _time[2], unsigned char _date[2]);
off_t radare_seek(off_t offset, int whence);
int is_printable (int c);
void radare_print(char *arg, print_fmt_t print_fmt, print_mode_t mode);
void data_print (off_t seek, unsigned char *buf, int len, print_fmt_t print_fmt, print_mode_t mode);
int hex2int (unsigned char *val, unsigned char c);
int get_cmp(const char *str0, const char *str1);
int terminal_get_real_columns();
int terminal_get_columns();
void radare_poke(char *arg);
int radare_open(int);
int string_flag_offset(char *buf, unsigned long long seek);
int yesno(int def, const char *fmt, ...);
// write modes
enum {
	WMODE_CHAR = 0,
	WMODE_STRING,
	WMODE_HEX,
	WMODE_WSTRING
};
int radare_write(char *arg, int mode);
//int hexstr2binstr(unsigned char *arg);
int hexstr2binstr(const char *in, unsigned char *out);
int escape_buffer(char *buf);
int radare_tsearch(char *range);
int radare_tsearch_file(char *file);
int hexpair2bin(const char *arg);
void endian_memcpy_e(unsigned char *dest, unsigned char *orig, unsigned int size, int endian);
int iswhitechar(char c);
char *strclean(char *str);
int strnull(const char *str);

#endif