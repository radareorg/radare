/*
 * Copyright (C) 2008
 *       pancake <@youterm.com>
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
#include "print.h"
#include <stdarg.h>
#if __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#endif
#if __WINDOWS__
#include <windows.h>
#endif

int _print_fd = 1;

const unsigned char *cons_palette_default = "7624 6646 2378 6824 3623";
int cons_palette[CONS_PALETTE_SIZE][8] = {
	/* PROMPT */
	/* ADDRESS */
	/* DEFAULT */
	/* CHANGED */

	/* JUMPS */
	/* CALLS */
	/* PUSH */
	/* TRAP */

	/* CMP */
	/* RET */
	/* NOP */
	/* METADATA */

	/* HEADER */
	/* PRINTABLE */
	/* LINES0 */
	/* LINES1 */

	/* LINES2 */
	/* 00 */
	/* 7F */
	/* FF */
};

// 23 elements
const unsigned char *cons_colors[CONS_COLORS_SIZE] = {
	C_BLACK,      // 0
	C_GRAY,       // 1
	C_WHITE,      // 2
	C_RED,        // 3
	C_MAGENTA,    // 4
	C_BLUE,       // 5
	C_GREEN,      // 6
	C_YELLOW,     // 7
	C_TURQOISE,   // 8
	/* BOLD */
	C_BBLACK,     // a
	C_BGRAY,      // b
	C_BWHITE,     // c
	C_BRED,       // d
	C_BMAGENTA,   // e
	C_BBLUE,      // f
	C_BGREEN,     // g
	C_BYELLOW,    // h
	C_BTURQOISE,  // i
	/* SPECIAL */
	C_RESET,      // r
	C_BGBLACK,    //
	C_BGRED,
	NULL
};

const char *pal_names[CONS_PALETTE_SIZE]={
	"prompt",
	"address",
	"default",
	"changed",
	"jumps",
	"calls",
	"push",
	"trap",
	"cmp",
	"ret",
	"nop",
	"metadata",
	"header",
	"printable",
	"lines0",
	"lines1",
	"lines2",
	"00",
	"7f",
	"ff",
	NULL
};

const char *cons_get_color(int ch)
{
	if (ch>='0' && ch<='8')
		return cons_colors[ch-'0'];
	if (ch>='a' && ch<='i')
		return cons_colors['8'-'0'+ch-'a'];
	return NULL;
}

int cons_palette_init(const unsigned char *pal)
{
	int palstrlen = strlen(pal);
	int i,j=1,k;

	if (pal==NULL || pal[0]=='\0') {
		cons_printf("\n=>( Targets ):");
		for(j=0;pal_names[j]&&*pal_names[j];j++)
			cons_printf("%s .%s\e[0m ", cons_palette[j], pal_names[j]);
		cons_printf("\n\n=>( Colors ): "
		"/*normal*/, " "black, = 0, " "gray, = 1, " "white, = 2, " "red, = 3, " "magenta, = 4, "
		"blue, = 5, " "green, = 6, " "yellow, = 7, " "turqoise, = 8, " "/*bold*/, " "bblack, = a, "
		"bgray, = b, " "bwhite, = c, " "bred, = d, " "bmagenta, = e, " "bblue, = f, " "bgreen, = g, "
		"byellow, = h, " "bturqoise, = i, " "/*special*/, " "reset, = r\n");
		cons_printf("\nExample: eval scr.palette = .prompt=3.address=4\n\n");
		return 0;
	}

	for(i=k=0;i<CONS_PALETTE_SIZE;i++,k++)
		if (j && pal[i]) {
			if (pal[i] == '.') { // che! action!!
				for(j=0;pal_names[j]&&*pal_names[j];j++) {
					int memcmp_len = palstrlen-i-1;
					if (!pal_names[j]) break;
					if (strlen(pal_names[j])<memcmp_len)
						memcmp_len = strlen(pal_names[j]);
					else continue;
				//	printf("CHK %s,%s,%d\n", pal_names[j], pal+i, memcmp_len);
					if (!memcmp(pal_names[j], pal+i+1, memcmp_len -1)) {
						i+=memcmp_len+1;
						if (pal[i] != '=') {
							printf("oops (%c) invalid format string (%s)\n", pal[i], pal+i);
							continue;
						}
				//		printf("KEYWORD FOUND = %s (value = %c)\n", pal_names[j], pal[i+1]);
						strcpy(cons_palette[j], cons_get_color(pal[i+1]));
					}
				}
			} else {
				char *ptr = cons_get_color(pal[i]);
				if (ptr)
					strcpy(cons_palette[k], ptr);
				else k--;
			}
		} else {
			j = 0;
			strcpy(cons_palette[i], C_RESET);
		}
	return 1;
}

int cons_readchar()
{
	char buf[2];
#if __WINDOWS__
	DWORD out;
	BOOL ret;
	LPDWORD mode;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(h, &mode);
	SetConsoleMode(h, 0); // RAW
	ret = ReadConsole(h, buf,1, &out, NULL);
	if (!ret)
		return -1;
	SetConsoleMode(h, mode);
#else
	if (read(0,buf,1)==-1)
		return -1;
#endif
	return buf[0];
}

int cons_set_fd(int fd)
{
	if (_print_fd == 0)
		return fd;
	return _print_fd = fd;
}

#if __WINDOWS__
void gotoxy(int x, int y)
{
        static HANDLE hStdout = NULL;
        COORD coord;

        coord.X = x;
        coord.Y = y;

        if(!hStdout)
                hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleCursorPosition(hStdout,coord);
}
#else

inline void gotoxy(int x, int y)
{
	cons_printf("\e[0;0H");
}
#endif

void cons_clear(void)
{
#if __WINDOWS__
        static HANDLE hStdout = NULL;
        static CONSOLE_SCREEN_BUFFER_INFO csbi;
        const COORD startCoords = {0,0};
        DWORD dummy;

        if(!hStdout) {
                hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                GetConsoleScreenBufferInfo(hStdout,&csbi);
        }

        FillConsoleOutputCharacter(hStdout, ' ', csbi.dwSize.X * csbi.dwSize.Y, startCoords, &dummy);
        gotoxy(0,0);
#else
	write(1, "\e[2J", 4);
#endif
}

#if __WINDOWS__
int cons_w32_print(unsigned char *ptr)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int esc = 0;
	unsigned char *str = ptr;
	int len = 0;
	int inv = 0;

	for (;ptr[0]; ptr = ptr + 1) {
		if (ptr[0] == 0x1b) {
			esc = 1;
			write(1, str, ptr-str);
			str = ptr + 1;
			continue;
		}
		if (esc == 1) {
			// \e[2J
			if (ptr[0] != '[') {
				eprintf("Oops invalid escape char\n");
				esc = 0;
				str = ptr + 1;
				continue;
			}
			esc = 2;
			continue;
		} else 
		if (esc == 2) {
			if (ptr[0]=='2'&&ptr[1]=='J') {
				ptr = ptr +1;
				cons_clear();
				esc = 0;
				str = ptr;
				continue;
			} else
			if (ptr[0]=='0'&&ptr[1]==';'&&ptr[2]=='0') {
				ptr = ptr + 4;
				gotoxy(0,0);
				esc = 0;
				str = ptr;
				continue;
			} else
			if (ptr[0]=='0'&&ptr[1]=='m') {
				SetConsoleTextAttribute(hConsole, 1|2|4|8);
				ptr = ptr + 1;
				str = ptr + 1;
				inv = 0;
				esc = 0;
				continue;
				// reset color
			} else
			if (ptr[0]=='7'&&ptr[1]=='m') {
				SetConsoleTextAttribute(hConsole, 128);
				inv = 128;
				ptr = ptr + 1;
				str = ptr + 1;
				esc = 0;
				continue;
				// reset color
			} else
			if (ptr[0]=='3' && ptr[2]=='m') {
				// http://www.betarun.com/Pages/ConsoleColor/
				switch(ptr[1]) {
				case '0': // BLACK
					SetConsoleTextAttribute(hConsole, 0|inv);
					break;
				case '1': // RED
					SetConsoleTextAttribute(hConsole, 4|inv);
					break;
				case '2': // GREEN
					SetConsoleTextAttribute(hConsole, 2|inv);
					break;
				case '3': // YELLOW
					SetConsoleTextAttribute(hConsole, 2|4|inv);
					break;
				case '4': // BLUE
					SetConsoleTextAttribute(hConsole, 1|inv);
					break;
				case '5': // MAGENTA
					SetConsoleTextAttribute(hConsole, 1|4|inv);
					break;
				case '6': // TURQOISE
					SetConsoleTextAttribute(hConsole, 1|2|8|inv);
					break;
				case '7': // WHITE
					SetConsoleTextAttribute(hConsole, 1|2|4|inv);
					break;
				case '8': // GRAY
					SetConsoleTextAttribute(hConsole, 8|inv);
					break;
				case '9': // ???
					break;
				}
				ptr = ptr + 1;
				str = ptr + 2;
				esc = 0;
				continue;
			}
		} 
		len++;
	}
	write(1, str, ptr-str);
	return len;
}

#endif

char *dl_readline(int argc, char **argv);
int cons_fgets(char *buf, int len)
{
	#define CMDS 50
	const char *argv[CMDS] ={
		NULL, // padding
		"? ",
		"!step ",
		"!stepo ",
		"!cont ",
		"!signal ",
		"!fd ",
		"!maps ",
		".!maps*",
		"!bp ",
		"!!",
		"#md5",
		"#sha1",
		"#crc32",
		"#entropy",
		"Visual",
		"emenu ",
		"eval ",
		"seek ",
		"info ",
		"help ",
		"move ",
		"quit ",
		"flag ",
		"Po ",
		"Ps ",
		"Pi ",
		"H ",
		"H no ",
		"H nj ",
		"H fj ",
		"H lua ",
		"x ",
		"b ",
		"y ",
		"yy ",
		"y? ",
		"wx ",
		"ww ",
		"wf ",
		"w?",
		"pD ",
		"pG ",
		"pb ",
		"px ",
		"pX ",
		"po ",
		"pm ",
		"pz ",
		"pr > ",
		"p? "
	};
	char *ptr = dl_readline(CMDS, argv);
	if (ptr == NULL)
		return -1;
	strncpy(buf, ptr, len);

	return strlen(buf);
}

static int cons_buffer_len = 0;
static char *cons_buffer = NULL;

void cons_reset()
{
	if (cons_buffer)
		cons_buffer[0]='\0';
	cons_buffer_len = 0;
}

char *cons_get_buffer()
{
	return cons_buffer;
}

void palloc(int moar)
{
	if (cons_buffer == NULL) {
		cons_buffer_len = moar+1024;
		cons_buffer = (char *)malloc(cons_buffer_len);
		cons_buffer[0]='\0';
	} else
	if (moar + strlen(cons_buffer)>cons_buffer_len) {
		cons_buffer = (char *)realloc(cons_buffer,
			cons_buffer_len+moar+strlen(cons_buffer)+1);
	}
}

void cons_flush()
{
	FILE *fd;
	char *ptr;
	char buf[1024];

	if (!strnull(cons_buffer)) {
		char *file = config_get("file.scrfilter");
		if (!strnull(file)) {
			fd = fopen(file, "r");
			if (fd) {
				while(!feof(fd)) {
					buf[0]='\0';
					fgets(buf, 1020, fd);
					if (buf[0]) {
						buf[strlen(buf)-1]='\0';
						char *ptr = strchr(buf, '\t');;
						if (ptr) {
							ptr[0]='\0'; ptr = ptr +1;
							cons_buffer = strsub(cons_buffer, buf, ptr, 1);
							cons_buffer_len = strlen(cons_buffer);
						}
					}
				}
				fclose(fd);
			}
		}
#if __WINDOWS__
		if (_print_fd == 1)
			cons_w32_print(cons_buffer);
		else
#endif
		write(_print_fd, cons_buffer, strlen(cons_buffer));
		cons_buffer[0] = '\0';
	}
}

/* stream is ignored */
void cons_fprintf(FILE *stream, const char *format, ...)
{
	/* dupped */
	char buf[4096];
	va_list ap;

	va_start(ap, format);

	buf[0]='\0';
	if (vsnprintf(buf, 4095, format, ap)<0)
		buf[0]='\0';

	palloc(strlen(buf)+1000);
	strcat(cons_buffer, buf);

	va_end(ap);
}

void cons_printf(const char *format, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, format);

	buf[0]='\0';
	if (vsnprintf(buf, 4095, format, ap)<0)
		buf[0]='\0';

	palloc(strlen(buf)+1000);
	strcat(cons_buffer, buf);

	va_end(ap);
}

void cons_newline()
{
	cons_printf("\n");
#if RADARE_CORE
	if (!config.buf)
		cons_flush();
#endif
}


void cons_strcat(const char *str)
{
	palloc(strlen(str));
	strcat(cons_buffer, str);
}

int terminal_get_columns()
{
	int columns_i = terminal_get_real_columns();
	char buf[64];

	sprintf(buf, "%d", columns_i);
	setenv("COLUMNS", buf, 0);

	return columns_i;
}

int terminal_get_real_columns()
{
#if __UNIX__
        struct winsize win;

        if (ioctl(1, TIOCGWINSZ, &win)) {
		/* default values */
		win.ws_col = 80;
		win.ws_row = 23;
	}
#ifdef RADARE_CORE
	config.width = win.ws_col;
	config.height = win.ws_row;
#endif

        return win.ws_col;
#else
	return 80;
#endif
	config.width = 78;
	config.height = 24;
}

#ifdef RADARE_CORE
int yesno(int def, const char *fmt, ...)
{
	va_list ap;
	int key = def;

	if (config.visual)
		key='y';
	else D {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fflush(stderr);
		terminal_set_raw(1);
		read(0, &key, 1); write(2, "\n", 1);
		terminal_set_raw(0);
	} else
		key = 'y';

	return key=='y';
}
#endif

/**
 *
 * void terminal_set_raw( [0,1] )
 *
 *   Change canonicality of the terminal
 *
 * For optimization reasons, there's no initialization flag, so you need to
 * ensure that the make the first call to terminal_set_raw() with '1' and
 * the next calls ^=1, so: 1, 0, 1, 0, 1, ...
 *
 * If you doesn't use this order you'll probably loss your terminal properties.
 *
 */
#if __UNIX__
static struct termios tio_old, tio_new;
#endif
static int termios_init = 0;

void terminal_set_raw(int b)
{
#if __UNIX__
	if (b) {
		if (termios_init == 0) {
			tcgetattr(0, &tio_old);
			memcpy (&tio_new,&tio_old,sizeof(struct termios));
			tio_new.c_iflag &= ~(BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
			tio_new.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
			tio_new.c_cflag &= ~(CSIZE|PARENB);
			tio_new.c_cflag |= CS8;
			tio_new.c_cc[VMIN]=1; // Solaris stuff hehe
			termios_init = 1;
		}
		tcsetattr(0, TCSANOW, &tio_new);
	} else
		tcsetattr(0, TCSANOW, &tio_old);
#else
	/* TODO : W32 */
#endif
	fflush(stdout);
}
