
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
	PAL_LINES0,
	PAL_LINES1,
	PAL_LINES2,
	PAL_00,
	PAL_7F,
	PAL_FF
};
extern const unsigned char *cons_palette_default;
#define CONS_PALETTE_SIZE 18
extern int cons_palette[CONS_PALETTE_SIZE][8];
char *cons_get_buffer();
void cons_reset();
void cons_clear();
int cons_readchar();
void cons_flush();
int cons_fgets(char *buf, int len);
int cons_set_fd(int fd);
void cons_strcat(const char *str);
void cons_newline();
