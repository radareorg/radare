#ifndef _INCLUDE_PRINT_H_
#define _INCLUDE_PRINT_H_

#include "types.h"
#include "list.h"

#define PRINT_BIN(x) D cons_printf("%d%d%d%d%d%d%d%d  ",\
(x&128)?1:0, (x&64)?1:0, (x&32)?1:0, (x&16)?1:0,\
(x&8)?1:0, (x&4)?1:0, (x&2)?1:0, (x&1)?1:0); else \
cons_printf("%d%d%d%d%d%d%d%d",\
(x&128)?1:0, (x&64)?1:0, (x&32)?1:0, (x&16)?1:0,\
(x&8)?1:0, (x&4)?1:0, (x&2)?1:0, (x&1)?1:0);
extern int dec;

#define CLRSCR() cons_strcat("\x1b[2J\x1b[0;0H");
int is_cursor(int from, int len);

/* inverted print block */
typedef enum {
	FMT_ERR   = -1,
	FMT_BIN,  FMT_FLOAT,  FMT_INT,   FMT_LONG,  FMT_LLONG,     FMT_OCT, FMT_BARS,
	FMT_RAW,  FMT_ASC,    FMT_URLE,  FMT_CSTR,  FMT_TIME_UNIX, FMT_TIME_DOS,
	FMT_TIME_FTIME,       FMT_HEXQ,  FMT_HEXD,  FMT_HEXW,      FMT_HEXB,  FMT_COMMENT,
	FMT_HEXBS,FMT_REGEXP, FMT_SHORT, FMT_VISUAL,FMT_ASHC,      FMT_DISAS, FMT_UDIS,
	FMT_PRINT, FMT_ASCP,  FMT_ASC0,  FMT_WASC0, FMT_DBG,       FMT_ANAL, FMT_CODE,
	FMT_CODEGRAPH, FMT_ZOOM, FMT_USER, FMT_LSB, FMT_MEMORY,    FMT_PERCENT, FMT_REF,
	FMT_7BIT, FMT_87BIT,  FMT_DOUBLE, FMT_BASE64, FMT_EBASE64, FMT_HEXPAIRS, FMT_DWORDDUMP
} print_fmt_t;

#include "cons.h"

/** Print capabilities for formats */
typedef enum {
	MD_ONCE   = 1,		/* print once */
	MD_BLOCK  = 1<<1,		/* print an entire block */
	MD_EXTRA  = 1<<2,
	MD_ALWAYS = 1<<3,		/* print always (once | block) */
} print_mode_t;

	//print_mode_t mode;
typedef struct format_info {
	char id;
	print_fmt_t print_fmt;
	char *name;
	char *sizeo;
	char *sizeb;
} format_info_t;

typedef struct {
	char *name;
	char *fmt;
	struct list_head list;
} print_mem_t;
extern struct list_head print_mems;

extern format_info_t formats[];
extern print_fmt_t last_print_format;
print_fmt_t format_get (char fmt);
void format_show_help (print_mode_t mode);
void print_addr(ut64 off);
void print_data(ut64 seek, char *arg, u8 *buf, int len, print_fmt_t print_fmt);
void print_color_byte_i(int i, char *str, int c);
void radare_dump_and_process(int type, int size);
void print_zoom(ut64 from, ut64 to, const char *byte, int enable);
void print_mem(ut64 addr, const u8 *buf, ut64 len, const char *fmt, int endian);
const char *print_mem_get(const char *name);
void print_mem_list(char *name, char *fmt);

#endif
