#ifndef _INCLUDE_UTILS_H_
#define _INCLUDE_UTILS_H_

#include "main.h"
#include "radare.h"
//#include "print.h"


#define uint unsigned int

int _print_fd;

extern const char hex[16];
//extern u64 tmpoff;
extern int std;
extern char **environ;
extern char *last_tsearch;

#define TMPFILE_MAX 50
#define BUFLEN 4096
#define CMPMIN(a,b) (a<b? a : b)

void eprintf(const char *format, ...);
void cons_printf(const char *format, ...);
int _strnstr(char *from, char *to, int size);
char *estrdup(char *ptr, char *string);
char *slurp(char *str);


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

int radare_dump(char *arg, int size);
void radare_dump_and_process(int type, int size);
int pipe_stdout_to_tmp_file(char *tmp, char *cmd);
int pprint_fd(int fd);
int make_tmp_file(char *str);
void progressbar(int pc);
int radare_read(int next);
u64 get_offset (const char *arg);
char *lstrchr(char *str, char chr);
u64 get_math(const char* text);
void print_msdos_date(unsigned char _time[2], unsigned char _date[2]);
u64 radare_seek(u64 offset, int whence);
int is_printable (int c);
void radare_print(char *arg, print_fmt_t print_fmt);
void data_print (u64 seek, char *arg, unsigned char *buf, int len, print_fmt_t print_fmt);
int hex2int (unsigned char *val, unsigned char c);
int get_cmp(const char *str0, const char *str1);
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
void endian_memcpy(u8 *dest, u8 *orig, unsigned int size);
void endian_memcpy_e(u8 *dest, u8 *orig, int size, int endian);
int iswhitechar(char c);
char *strclean(char *str);
int strnull(const char *str);
int gnu_disarm(unsigned char *address, unsigned long seek);
char *gnu_dismips(unsigned char *inst, unsigned long offset);

#endif
