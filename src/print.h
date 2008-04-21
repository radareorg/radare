#ifndef _INCLUDE_PRINT_H_
#define _INCLUDE_PRINT_H_


void print_color_byte_i(int i, char *str, int c);
void radare_dump_and_process(int type, int size);
#define PRINT_BIN(x) D cons_printf("%d%d%d%d %d%d%d%d  ",\
(x&128)?1:0, (x&64)?1:0, (x&32)?1:0, (x&16)?1:0,\
(x&8)?1:0, (x&4)?1:0, (x&2)?1:0, (x&1)?1:0); else \
cons_printf("%d%d%d%d%d%d%d%d",\
(x&128)?1:0, (x&64)?1:0, (x&32)?1:0, (x&16)?1:0,\
(x&8)?1:0, (x&4)?1:0, (x&2)?1:0, (x&1)?1:0);
extern int dec;

//#define INILINE D { int i; if(config.x) printf("\r"); for(i=1;i<config.x;i++) printf(" "); } 
#define INILINE
#undef NEWLINE
#define NEWLINE cons_newline();

#define CLRSCR() cons_strcat("\e[2J\e[0;0H");
char *cons_get_buffer();
void cons_reset();
void cons_clear();
int cons_readchar();
void cons_flush();
int cons_fgets(char *buf, int len);
int cons_set_fd(int fd);
void cons_strcat(const char *str);
void cons_newline();
int is_cursor(int from, int len);

/* inverted print block */
typedef enum {
	FMT_ERR   = -1,
	FMT_BIN,  FMT_FLOAT,  FMT_INT,   FMT_LONG,  FMT_LLONG,     FMT_OCT,
	FMT_RAW,  FMT_ASC,    FMT_URLE,  FMT_CSTR,  FMT_TIME_UNIX, FMT_TIME_DOS,
	FMT_TIME_FTIME,       FMT_HEXQ,  FMT_HEXD,  FMT_HEXW,      FMT_HEXB,
	FMT_HEXBS,FMT_REGEXP, FMT_SHORT, FMT_VISUAL,FMT_ASHC,      FMT_DISAS, FMT_UDIS,
	FMT_PRINT, FMT_ASCP,  FMT_ASC0,  FMT_WASC0, FMT_DBG,       FMT_ANAL, FMT_CODE,
	FMT_CODEGRAPH, FMT_ZOOM, FMT_USER, FMT_LSB, FMT_MEMORY,    FMT_PERCENT, FMT_REF,
	FMT_7BIT, FMT_87BIT
} print_fmt_t;

/* plain colors */
#define C_BLACK    "\e[30m"
#define C_BGBLACK  "\e[40m"
#define C_RED      "\e[31m"
#define C_BGRED    "\e[41m"
#define C_WHITE    "\e[37m"
#define C_RESET    "\e[0m"
#define C_GREEN    "\e[32m"
#define C_MAGENTA  "\e[35m"
#define C_YELLOW   "\e[33m"
#define C_TURQOISE "\e[36m"
#define C_BLUE     "\e[34m"
#define C_GRAY     "\e[38m"
/* bold colors */
#define C_BBLACK    "\e[1;30m"
#define C_BRED      "\e[1;31m"
#define C_BBGRED    "\e[1;41m"
#define C_BWHITE    "\e[1;37m"
#define C_BGREEN    "\e[1;32m"
#define C_BMAGENTA  "\e[1;35m"
#define C_BYELLOW   "\e[1;33m"
#define C_BTURQOISE "\e[1;36m"
#define C_BBLUE     "\e[1;34m"
#define C_BGRAY     "\e[1;38m"
/* default byte colors */
#define COLOR_00 C_TURQOISE
#define COLOR_FF C_RED
#define COLOR_7F C_MAGENTA
#define COLOR_PR C_YELLOW
#define COLOR_HD C_GREEN
// addresses
#define COLOR_AD C_GREEN

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

extern format_info_t formats[];
extern print_fmt_t last_print_format;
print_fmt_t format_get (char fmt);
void format_show_help (print_mode_t mode);
int radare_analyze(u64 seek, int size, int level);
void print_addr(u64 off);

#endif
