#ifndef _INCLUDE_CONS_H_
#define _INCLUDE_CONS_H_

/* plain colors */
#define C_BLACK    "\x1b[30m"
#define C_BGBLACK  "\x1b[40m"
#define C_RED      "\x1b[31m"
#define C_BGRED    "\x1b[41m"
#define C_WHITE    "\x1b[37m"
#define C_RESET    "\x1b[0m"
#define C_GREEN    "\x1b[32m"
#define C_MAGENTA  "\x1b[35m"
#define C_YELLOW   "\x1b[33m"
#define C_TURQOISE "\x1b[36m"
#define C_BLUE     "\x1b[34m"
#define C_GRAY     "\x1b[38m"
/* bold colors */
#define C_BBLACK    "\x1b[1;30m"
#define C_BRED      "\x1b[1;31m"
#define C_BBGRED    "\x1b[1;41m"
#define C_BWHITE    "\x1b[1;37m"
#define C_BGREEN    "\x1b[1;32m"
#define C_BMAGENTA  "\x1b[1;35m"
#define C_BYELLOW   "\x1b[1;33m"
#define C_BTURQOISE "\x1b[1;36m"
#define C_BBLUE     "\x1b[1;34m"
#define C_BGRAY     "\x1b[1;38m"
/* default byte colors */
#if 0
#define COLOR_00 C_TURQOISE
#define COLOR_FF C_RED
#define COLOR_7F C_MAGENTA
#define COLOR_PR C_YELLOW
#define COLOR_HD C_GREEN
// addresses
#define COLOR_AD C_GREEN
#endif

#define CONS_PALETTE_SIZE 22
#define CONS_COLORS_SIZE 21
enum {
	PAL_PROMPT = 0,
	PAL_ADDRESS,
	PAL_DEFAULT,
	PAL_CHANGED,
	PAL_JUMP,
	PAL_CALL,
	PAL_PUSH,
	PAL_TRAP,
	PAL_CMP,
	PAL_RET,
	PAL_NOP,
	PAL_METADATA,
	PAL_HEADER,
	PAL_PRINTABLE,
	PAL_LINES0,
	PAL_LINES1,
	PAL_LINES2,
	PAL_00,
	PAL_7F,
	PAL_FF
};

extern const char *cons_palette_default;
const char *cons_colors[CONS_COLORS_SIZE+1];
extern char cons_palette[CONS_PALETTE_SIZE][8];
char *cons_get_buffer();
void cons_reset();
void cons_clear();
int cons_readchar();
void cons_flush();
int cons_fgets(char *buf, int len, int argc, const char **argv);
int cons_set_fd(int fd);
void cons_strcat(const char *str);
void cons_newline();
void cons_set_raw(int b);
int cons_get_real_columns();
int cons_get_columns();
int cons_palette_init(const unsigned char *pal);
void cons_printf(const char *format, ...);
void cons_gotoxy(int x, int y);
extern const char *dl_prompt;
int cons_get_arrow(int ch);
void cons_clear00();
void cons_any_key();

extern int cons_noflush;
#endif
