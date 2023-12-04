/*
 * Copyright (C) 2008, 2009, 2010
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

int yesno(int def, const char *fmt, ...);
#define CONS_MAX_USER 102400
int cons_stdout_fd = 6676;
int cons_stdout_file = -1;
FILE *cons_stdin_fd = NULL;
static unsigned int cons_buffer_sz = 0;
int cons_buffer_len = 0;
static char *cons_buffer = NULL;
char *cons_filterline = NULL;
char *cons_teefile = NULL;
int cons_is_html = 0;
int cons_floodprot = 1;
int _print_fd = 1;
int cons_lines = 0;
int cons_noflush = 0;
#define CONS_BUFSZ 0x4f00

// TODO rename to cons_stdout_open
void stdout_open(char *file)
{
	int fd = open(file, O_RDWR|O_CREAT, 0644);
	if (fd==-1) {
		eprintf("stdout_open: Cannot open '%s'\n", file);
		return;
	}
	cons_stdout_file = fd;
	dup2(1, cons_stdout_fd);
	//close(1);
	dup2(fd, 1);
}

void stdout_close()
{
	//if (cons_stdout_fd != 1)
	//	close(cons_stdout_fd);
	dup2(cons_stdout_fd, 1);
}

const char *cons_palette_default = "7624 6646 2378 6824 3623";
char cons_palette[CONS_PALETTE_SIZE][8] = {
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

static const char *cons_color_names[CONS_COLORS_SIZE+1] = {
	"black",
	"gray",
	"white",
	"red",
	"magenta",
	"blue",
	"green",
	"yellow",
	"turqoise",
	"bblack",
	"bgray",
	"bwhite",
	"bred",
	"bmagenta",
	"bblue",
	"bgreen",
	"byellow",
	"bturqoise",
	"reset",
	"bgblack",
	"bgred",
	NULL
};

void cons_invert(int set)
{
	if (set) cons_strcat("\x1b[7m");
	else cons_strcat("\x1b[0m");
}

static const char *cons_colors[CONS_COLORS_SIZE+1] = {
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

static const char *cons_get_color_by_name(const char *str)
{
	int i;
	for(i=0;cons_color_names[i];i++) {
		if (!strcmp(str, cons_color_names[i]))
			return cons_colors[i];
	}
	return nullstr;
}

static inline int cons_lines_count(const char *str)
{
	int i,ctr = 0;
	for(i=0;cons_buffer[i];i++) {
		if (cons_buffer[i] == '\n')
			ctr++;
	}
	return ctr;
}

static void cons_print_real(const char *buf)
{
#if __WINDOWS__
	if (_print_fd == 1)
		cons_w32_print(buf);
	else
#endif
	if (cons_is_html)
		cons_html_print(buf);
	else write(_print_fd, buf, cons_buffer_len);
}

int cons_palette_set(const char *key, const u8 *value)
{
	const char *str;
	int i;

	for(i=0;pal_names[i];i++) {
		if (!strcmp(key, pal_names[i])) {
			str = cons_get_color_by_name(value);
			if (str != NULL) {
				strcpy(cons_palette[i], str);
				return 0;
			}
		}
	}
	return 1;
}

int cons_palette_init(const unsigned char *pal)
{
	int palstrlen;
	int i,j=1,k;

	cons_stdin_fd = stdin;
	if (pal==NULL || pal[0]=='\0') {
		cons_printf("\n=>( Targets ):");
		for(j=0;pal_names[j]&&*pal_names[j];j++)
			cons_printf("%s .%s\x1b[0m ", cons_palette[j], pal_names[j]);
		cons_printf("\n\n=>( Colors ): "
		"/*normal*/, " "black, = 0, " "gray, = 1, " "white, = 2, " "red, = 3, " "magenta, = 4, "
		"blue, = 5, " "green, = 6, " "yellow, = 7, " "turqoise, = 8, " "/*bold*/, " "bblack, = a, "
		"bgray, = b, " "bwhite, = c, " "bred, = d, " "bmagenta, = e, " "bblue, = f, " "bgreen, = g, "
		"byellow, = h, " "bturqoise, = i, " "/*special*/, " "reset, = r\n");
		cons_printf("\nExample: eval scr.palette = .prompt=3.address=4\n\n");
		return 0;
	}

	palstrlen = strlen((const char *)pal);
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
				const char *ptr = cons_get_color(pal[i]);
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
	DWORD mode;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode (h, &mode);
	SetConsoleMode (h, 0); // RAW
	ret = ReadConsole(h, buf,1, &out, NULL);
	if (!ret)
		return -1;
	SetConsoleMode (h, mode);
#else
	if (read (0, buf, 1)==-1)
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

void cons_gotoxy(int x, int y)
{
#if __WINDOWS__
        static HANDLE hStdout = NULL;
        COORD coord;

        coord.X = x;
        coord.Y = y;

        if(!hStdout)
                hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleCursorPosition(hStdout,coord);
#else
	cons_strcat("\x1b[0;0H");
#endif
}

void cons_clear00()
{
	cons_lines = 0;
	cons_clear();
	cons_gotoxy(0,0);
}

void cons_clear()
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
        cons_gotoxy(0,0);
#else
	cons_strcat("\x1b[2J");
#endif
	cons_lines = 0;
}

int cons_html_print(const char *ptr)
{
	int esc = 0;
	const char *str = (char *)ptr;
	int len = 0;
	int inv = 0;
	int color = 0;

	for (;ptr[0]; ptr = ptr + 1) {
		if (ptr[0] == '\n') {
			printf("<br />");
			fflush(stdout);
		}
		if (ptr[0] == 0x1b) {
			esc = 1;
			write(1, str, ptr-str);
			str = ptr + 1;
			continue;
		}
		if (esc == 1) {
			// \x1b[2J
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
				printf("<hr />\n"); fflush(stdout);
				esc = 0;
				str = ptr;
				continue;
			} else
			if (ptr[0]=='0'&&ptr[1]==';'&&ptr[2]=='0') {
				ptr = ptr + 4;
				cons_gotoxy(0,0);
				esc = 0;
				str = ptr;
				continue;
			} else
			if (ptr[0]=='0'&&ptr[1]=='m') {
				ptr = ptr + 1;
				str = ptr + 1;
				inv = 0;
				esc = 0;
				continue;
				// reset color
			} else
			if (ptr[0]=='7'&&ptr[1]=='m') {
				inv = 128;
				ptr = ptr + 1;
				str = ptr + 1;
				esc = 0;
				continue;
				// reset color
			} else
			if (ptr[0]=='3' && ptr[2]=='m') {
				color = 1;
				switch(ptr[1]) {
				case '0': // BLACK
					printf("<font color=black>"); fflush(stdout);
					break;
				case '1': // RED
					printf("<font color=red>"); fflush(stdout);
					break;
				case '2': // GREEN
					printf("<font color=green>"); fflush(stdout);
					break;
				case '3': // YELLOW
					printf("<font color=yellow>"); fflush(stdout);
					break;
				case '4': // BLUE
					printf("<font color=blue>"); fflush(stdout);
					break;
				case '5': // MAGENTA
					printf("<font color=magenta>"); fflush(stdout);
					break;
				case '6': // TURQOISE
					printf("<font color=#0ae>"); fflush(stdout);
					break;
				case '7': // WHITE
					printf("<font color=white>"); fflush(stdout);
					break;
				case '8': // GRAY
					printf("<font color=#777>"); fflush(stdout);
					break;
				case '9': // ???
					break;
				}
				ptr = ptr + 1;
				str = ptr + 2;
				esc = 0;
				continue;
			} else
			if (ptr[0]=='4' && ptr[2]=='m') {
				/* background color */
				switch(ptr[1]) {
				case '0': // BLACK
					printf("<font style='background-color:#000'>"); fflush(stdout);
					break;
				case '1': // RED
					printf("<font style='background-color:#f00'>"); fflush(stdout);
					break;
				case '2': // GREEN
					printf("<font style='background-color:#0f0'>"); fflush(stdout);
					break;
				case '3': // YELLOW
					printf("<font style='background-color:#ff0'>"); fflush(stdout);
					break;
				case '4': // BLUE
					printf("<font style='background-color:#00f'>"); fflush(stdout);
					break;
				case '5': // MAGENTA
					printf("<font style='background-color:#f0f'>"); fflush(stdout);
					break;
				case '6': // TURQOISE
					printf("<font style='background-color:#aaf'>"); fflush(stdout);
					break;
				case '7': // WHITE
					printf("<font style='background-color:#fff'>"); fflush(stdout);
					break;
				case '8': // GRAY
					printf("<font style='background-color:#777'>"); fflush(stdout);
					break;
				case '9': // ???
					break;
				}
			}
		} 
		len++;
	}
	write(1, str, ptr-str);
	return len;
}

#if __WINDOWS__
int cons_w32_print(unsigned char *ptr)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int esc = 0;
	int bg = 0;
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
			// \x1b[2J
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
				cons_gotoxy(0,0);
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
					SetConsoleTextAttribute(hConsole, bg|0|inv);
					break;
				case '1': // RED
					SetConsoleTextAttribute(hConsole, bg|4|inv);
					break;
				case '2': // GREEN
					SetConsoleTextAttribute(hConsole, bg|2|inv);
					break;
				case '3': // YELLOW
					SetConsoleTextAttribute(hConsole, bg|2|4|inv);
					break;
				case '4': // BLUE
					SetConsoleTextAttribute(hConsole, bg|1|inv);
					break;
				case '5': // MAGENTA
					SetConsoleTextAttribute(hConsole, bg|1|4|inv);
					break;
				case '6': // TURQOISE
					SetConsoleTextAttribute(hConsole, bg|1|2|8|inv);
					break;
				case '7': // WHITE
					SetConsoleTextAttribute(hConsole, bg|1|2|4|inv);
					break;
				case '8': // GRAY
					SetConsoleTextAttribute(hConsole, bg|8|inv);
					break;
				case '9': // ???
					break;
				}
				ptr = ptr + 1;
				str = ptr + 2;
				esc = 0;
				continue;
			} else
			if (ptr[0]=='4' && ptr[2]=='m') {
				/* TODO: must be implemented . actually skipped */
				/* background color */
				switch(ptr[1]) {
				case '0': // BLACK
					bg = 0;
					break;
				case '1': // RED
					bg = 40;
					break;
				case '2': // GREEN
					bg = 20;
					break;
				case '3': // YELLOW
					bg = 20|40;
					break;
				case '4': // BLUE
					bg = 10;
					break;
				case '5': // MAGENTA
					bg = 10|40;
					break;
				case '6': // TURQOISE
					bg = 10|20|80;
					break;
				case '7': // WHITE
					bg = 10|20|40;
					break;
				case '8': // GRAY
					bg = 80;
					break;
				case '9': // ???
					break;
				}
				ptr = ptr + 1;
				str = ptr + 2;
				continue;
			}
		} 
		len++;
	}
	write(1, str, ptr-str);
	return len;
}
#endif

#define CMDS 54
static const char *radare_argv[CMDS] ={
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
	"ad",
	"ac",
	"ag",
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

char *dl_readline(int argc, const char **argv);
int cons_fgets(char *buf, int len, int argc, const char **argv)
{
	char *ptr;
	buf[0]='\0';
	ptr = dl_readline((argv)?argc:CMDS, (argv)?argv:radare_argv);
	if (ptr == NULL)
		return -1;
	strncpy(buf, ptr, len);

	return strlen(buf);
}
static void palloc(int moar)
{
	if (cons_buffer == NULL) {
		cons_buffer_sz = moar+4096;
		cons_buffer = (char *)malloc(cons_buffer_sz);
		cons_buffer[0]='\0';
	} else
	if (moar + cons_buffer_len > cons_buffer_sz) {
		cons_buffer_sz += moar+4096;
		cons_buffer = (char *)realloc(cons_buffer, cons_buffer_sz);
	}
}

void cons_reset()
{
	if (cons_buffer)
		cons_buffer[0]='\0';
	cons_buffer_len = 0;
}

const char *cons_get_buffer()
{
	return cons_buffer;
}

int cons_set_buffer(const char *buf)
{
	cons_buffer_len = strlen(buf);
	palloc(cons_buffer_len);
	memcpy(cons_buffer, buf, cons_buffer_len+1);
	return 0;
}


static int grepline = -1, greptoken = -1, grepcounter = 0, grepneg = 0;
static char *grepstr = NULL;
static char *grephigh = NULL;
//static int grepstrings_n = 1;
static char *cons_lastline =NULL;

int cons_grepbuf(char *buf, int len)
{
	int donotline = 0;
	int i, j, hit = 0;
	int linelen;
	int lines = 0;
	int toklen;
	char delims[6][2] = {"|", "/", "\\", ",", ";", "\t"};
	char *n; 
	char *grepstrings[2];
	char *obuf = buf;
	grepstrings[0] = grepstr;
	grepstrings[1] = NULL;
	int grepstrings_n = 1;
	if (grepstr == NULL)
		return len;
begin:
	if (buf==NULL || buf[0]=='\0')
		return len;
	n = memchr(buf, '\n', len);

	if (cons_lastline==NULL)
		cons_lastline = buf; //cons_buffer;
	if (!n) return len;

	n[0]='\0';
	linelen = (int)(n-buf);

	//for(i=0;i<grepstrings_n;i++) {
	//	grepstr = grepstrings[i];
	//	if (grepstr == NULL)
	//		break;
	if ( (!grepneg && strstr(buf, grepstr)) || (grepneg && !strstr(buf, grepstr)))
		hit = 1;
	else hit = 0;
	//}

	donotline = 0;
	if (hit) {
		if (grepline != -1) {
			if (grepline==lines) {
		if (greptoken != -1) {
			char *tok = NULL;
			char *ptr = buf;
			for (i=0; i<linelen; i++) for (j=0;j<6;j++)
				if (ptr[i] == delims[j][0])
					ptr[i] = ' ';
			tok = ptr;
			for (i=0;tok != NULL && i<=greptoken;i++) {
				if (i==0) tok = (char *)strtok(ptr, " ");
				else tok = (char *)strtok(NULL, " ");
			}
			if (tok) {
				toklen = strlen(tok);
				len -= (linelen-toklen);
				str_cpy(buf, tok);
				memcpy(buf+toklen, "\n", 1);
				if (!n) return len;
				str_cpy(buf+toklen+1, buf+linelen); //n+1);
				buf = buf+toklen+1;
				goto begin;
			}
		} else { 
				len = strlen(obuf);
				return len;
		}
			} else {
				donotline = 1;
				lines++;
			}
		}
		n[0]='\n';
	} else donotline = 1;

	if (donotline) {
		/* ok */
		str_cpy(buf, n+1);
		len-=linelen;
		goto begin;
	} else {
		/* broken */
		if (greptoken != -1) {
			char *tok = NULL;
			char *ptr = buf;
			for (i=0; i<linelen; i++) for (j=0;j<6;j++)
				if (ptr[i] == delims[j][0])
					ptr[i] = ' ';
			tok = ptr;
			for (i=0;tok != NULL && i<=greptoken;i++) {
				if (i==0) tok = (char *)strtok(ptr, " ");
				else tok = (char *)strtok(NULL, " ");
			}
			if (tok) {
				toklen = strlen(tok);
				len -= (linelen-toklen);
				str_cpy(buf, tok);
				memcpy(buf+toklen, "\n", 1);
				if (!n) return len;
				str_cpy(buf+toklen+1, buf+linelen); //n+1);
				buf = buf+toklen+1;
				goto begin;
			}
		} 
		lines++;
	}
	if (n) {
	n[0]='\n';
		if (linelen>0) {
			buf = n+1;
			goto begin;
		}
	}
	grepstr = NULL;
	greptoken = -1;
	grepline = -1;
	return len;
}

void cons_grep(const char *str)
{
	char *ptr, *ptr2, *ptr3, *ptr4;
	grepcounter = 0;

	/* set grep string */
	greptoken = -1;
	grepline = -1;
	efree((void **)&grepstr);
	efree((void **)&grephigh);

	if (str != NULL && *str) {
		if (*str == '!') {
			grepneg = 1;
			str = str + 1;
		} else grepneg = 0;
		if (*str == '?') {
			grepcounter = 1;
			str = str + 1;
		}
		ptr = alloca(strlen(str)+2);
		strcpy(ptr, str);

		ptr3 = strchr(ptr, '[');
		ptr2 = strchr(ptr, '#');
		ptr4 = strchr(ptr, '*');

		if (ptr4) {
			ptr4[0]='\0';
			grephigh = estrdup(grephigh, ptr4+1);
		}
		if (ptr3) {
			ptr3[0]='\0';
			greptoken = get_offset(ptr3+1);
			if (greptoken<0)
				greptoken--;
		}
		if (ptr2) {
			ptr2[0]='\0';
			grepline = get_offset(ptr2+1);
		}
		if (*ptr)
			grepstr = estrdup(grepstr, ptr);
	} else {
		greptoken = -1;
		grepline = -1;
		efree((void **)&grepstr);
		efree((void **)&grephigh);
	}
#if 0
	if (grephigh == NULL || *grephigh == '\0')
		grephigh = strdup(config_get("scr.grephigh"));
#endif
}

void cons_grepbuf_end()
{
	greptoken = -1;
	grepline = -1;
	efree((void **)&grepstr);
	efree((void **)&grephigh);
}

char *str_replace(char *str, char *from, char *to, int str_len)
{
	char *ostr = str;
	char *ofrom = from;
	char *p, *tstr;
	int len, olen = strlen(str);
	int flen = strlen(from);
	int tlen = strlen(to);
	int ftdelta = tlen-flen;

	while(*str) {
		if (*str == *from) {
			from = from +1;
		} else {
			from = ofrom;
		}
		if (*from == '\0') {
			str = str - flen;
			tstr = str;
			/* realloc */
			if (flen >= tlen) {
				strcpy(str+tlen, str+flen);
				memcpy(str, to, tlen);
			} else {
				int strdelta = str-ostr;
				len = olen;
				olen += (tlen-flen);
				if (olen > str_len) {
					//eprintf("FUCK!!! %d %d\n", str_len, olen);
					// XXX use palloc() here
					ostr = realloc(ostr, olen+4096);
					str_len = olen+4096;
					str = ostr+strdelta;
				}
				for(p=ostr+olen; p>str;p=p-1)
					p[ftdelta] = *p;
				memcpy(str+1, to, tlen);
			}
			from = ofrom;
			str = str+tlen;
		}
		str = str+1;
	}
	return ostr;
}

static char *cons_buffer2 = NULL;
static int cons_buffer2_len = 0;
int cons_flushable = 0;

void cons_flush()
{
	if (!cons_flushable) {
		cons_render();
		return;
	}
	if (cons_buffer2 == NULL) {
		cons_buffer2 = cons_buffer;
		cons_buffer2_len = cons_buffer_len;
		cons_buffer = malloc(cons_buffer_sz);
		cons_buffer_len = 0;
	} else {
		/* concat */
		int newlen = cons_buffer2_len + cons_buffer_len;
		cons_buffer2 = realloc(cons_buffer2, newlen+1);
		memcpy(cons_buffer2+cons_buffer2_len, cons_buffer, cons_buffer_len);
		cons_buffer2_len = newlen;
	}
	cons_buffer[0] = '\0';
	cons_buffer_len = 0;
}

int ansistrlen(const char *str)
{
	int i=0, len = 0;
	while(str[i]) {
		if (str[i]==0x1b && str[i+1]=='[')
			for(++i;str[i]&&str[i]!='J'&&str[i]!='m'&&str[i]!='H';i++);
		else len++;
		i++;
	}
	return len;
}

char *ansistrchrn(char *str, int n)
{
	int i, len = 0;
	for (i=0;str[i];i++) {
		if (n == len)
			break;
		if (str[i]==0x1b && str[i+1]=='[')
			for(++i;str[i]&&str[i]!='J'&&str[i]!='m'&&str[i]!='H';i++);
		else len++;
	}
	return str+i;
}

static int cons_skipx = 0;
static int cons_skipy = 0;

void cons_skipxy(int x, int y)
{
	if (x==y && x==0) {
		cons_skipx = 0;
		cons_skipy = 0;
	} else {
		cons_skipx += x;
		cons_skipy += y;
		if (cons_skipx<0) cons_skipx = 0;
		if (cons_skipy<0) cons_skipy = 0;
	}
}

void cons_fitbuf(char *buf, int len)
{
	char *next;
	char *ptr = buf;
	int linelen, i = 0, olen = len;
	int lines = 0;
	int rows = config.height;
	int cols = config.width;// -cons_skipx;
	if (buf == NULL)
		return;
	if (len<1) {
		buf[0]='\0';
		return;
	}
	buf[len]='\0';
	do {
		if (lines>0 && cons_skipx) {
			ptr = memchr(buf, '\n', len);
			if (ptr) linelen = ptr-buf;
			else linelen = strlen(buf);
			next = ansistrchrn(buf, cons_skipx);
			/* too short line */
			if (next>=buf+linelen) {
				buf[0]='\n';
				len -= linelen;
				if (ptr) str_cpy(buf+1, ptr+1);
				buf = buf+1;
			} else {
				len -= cons_skipx;
				str_cpy(buf, next);
				//str_cpy(buf, buf+cons_skipx);
			}
			if (len<=0) break;
		}
		ptr = memchr(buf, '\n', len);
		if (ptr) {
			linelen = (int)(ptr-buf);
			len -= linelen;
			if (lines>config.scrdelta+3 && cons_skipy && cons_skipy>=lines) {
				str_cpy(buf, ptr+1);
				lines++;
				continue;
			}
			if (len<=0) break;
			ptr[0]='\0';
			i += linelen;
			/* fit columns */
			linelen = ansistrlen(buf);
			if (linelen>cols) {
				next = ansistrchrn(buf,cols);
				next[0]='\n';
				//str_cpy(buf+cols+1, ptr+1); // ok for b&w
				str_cpy(next+1, ptr+1);
				buf = next+1;
			} else {
				ptr[0]='\n';
				buf = ptr+1;
			}
		}
		if (lines > rows+cons_skipy) {
			buf[0]='\0';
			break;
		}
		lines++;
	} while(i<olen && ptr);
}

void cons_flushit()
{
	char *ob;
	cons_flush();
	ob=cons_buffer;
	cons_buffer = cons_buffer2;
	cons_buffer_len = cons_buffer2_len;
	if (config.scrfit)
	cons_fitbuf(cons_buffer, cons_buffer_len);
	cons_render();
	cons_buffer = ob;
	cons_buffer_len = 0;
	if (cons_buffer2&&cons_buffer2[0]) /* ugly hack :) */
		free(cons_buffer2);
	cons_buffer2 = NULL;
	cons_buffer2_len = 0;
}

void cons_render()
{
	FILE *fd;
	char buf[1024];
	int i, j, lines_counter = 0;

	if (cons_noflush)
		return;

	if (cons_floodprot && cons_buffer) {
		cons_buffer_len = strlen(cons_buffer);
		if (cons_buffer && (cons_buffer_len > CONS_MAX_USER)) {
			eprintf("NOTE: Use 'e scr.floodprot=false' to disable this message\n");
			if (!yesno('n', "Do you want to print %d bytes? (y/N)", cons_buffer_len)) {
				cons_reset();
				return;
			}
		}
	}
	if (grephigh == NULL) {
		grephigh = (char *)config_get("scr.grephigh");
		if (grephigh &&*grephigh) {
			char bat[64];
			sprintf(bat, "\x1b[7m%s\x1b[0m", grephigh);
			cons_buffer = str_replace(cons_buffer, grephigh, bat, cons_buffer_sz);
		}
		grephigh = NULL;
	} else {
		if (grephigh &&*grephigh) {
			char bat[64];
			sprintf(bat, "\x1b[7m%s\x1b[0m", grephigh);
			cons_buffer = str_replace(cons_buffer, grephigh, bat, cons_buffer_sz);
		}
	}

	if (!strnull(cons_buffer)) {
		char *file = cons_filterline;
		char *tee = cons_teefile;
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
							cons_buffer = (char *)strsub(cons_buffer, buf, ptr, 1);
							cons_buffer_len = strlen(cons_buffer);
						}
					}
				}
				fclose(fd);
			}
		}
		
		if (tee&&tee[0]) {
			FILE *d = fopen(tee, "a+");
			if (d != NULL) {
				fwrite(cons_buffer, strlen(cons_buffer),1, d);
				fclose(d);
			}
		}

		// XXX merge grepstr with cons_lines loop //
		cons_lines += cons_lines_count(buf);

		// XXX major cleanup here!
		if (grepstr != NULL) {
			int line, len;
			char *one = cons_buffer;
			char *two;
			char *ptr, *tok;
			char delims[6][2] = {"|", "/", "\\", ",", ";", "\t" };

			for(line=0;;) {
				two = strchr(one, '\n');
				if (two) {
					two[0] = '\0';
					len = two-one;
				//	len = strlen(one);
					if ( (!grepneg && strstr(one, grepstr))
					|| (grepneg && !strstr(one, grepstr))) {
						if (grepline ==-1 || grepline==line) {
							if (greptoken != -1) {
								ptr = alloca(len+1);
								strcpy(ptr, one);
								for (i=0; i<len; i++)
									for (j=0;j<6;j++)
										if (ptr[i] == delims[j][0])
											ptr[i] = ' ';
								tok = ptr;
								if (greptoken<0) {
									int i, idx = greptoken+1;
									for(i = 0;ptr[i]; i++) {
										if (ptr[i]==' ')
											idx++;
										if (idx == 0) {
											ptr = ptr +i;
											cons_buffer_len = strlen(ptr);
											break;
										}
									}
								} else {
									for (i=0;tok != NULL && i<=greptoken;i++) {
										if (i==0)
											tok = strtok(ptr, " ");
										else tok = strtok(NULL, " ");
									}
									if (tok) {
										ptr = tok;
										cons_buffer_len = strlen(tok);
									}
								}
							} else {
								ptr = one;
								cons_buffer_len=len;
							}
							if (grepcounter==0) {
								cons_print_real(ptr);
								cons_buffer_len=1;
								cons_print_real("\n");
							} else lines_counter++;
						}
						line++;
					}
					two[0] = '\n';
					one = two + 1;
				} else break;
			}
		} else {
			if (grepline != -1 || grepcounter || greptoken != -1) {
				int len, line;
				char *one = cons_buffer;
				char *two;
				char *ptr, *tok;
				char delims[6][2] = {"|", "/", "\\", ",", ";", "\t"};
				for(line=0;;line++) {
					two = strchr(one, '\n');
					if (two) {
						two[0] = '\0';
						len=two-one;
						if (grepline ==-1 || grepline==line) {
							if (greptoken != -1) {
								ptr = alloca(len+1);
								strcpy(ptr, one);

								for (i=0; i<len; i++)
									for (j=0;j<6;j++)
										if (ptr[i] == delims[j][0])
											ptr[i] = ' ';
								tok = ptr;
								if (greptoken<0) {
									int i, idx = greptoken+1;
									for(i = 0;ptr[i]; i++) {
										if (ptr[i]==' ')
											idx++;
										if (idx == 0) {
											ptr = ptr +i;
											cons_buffer_len = strlen(ptr);
											break;
										}
									}
								} else {
									for (i=0;tok != NULL && i<=greptoken;i++) {
										if (i==0)
											tok = strtok(ptr, " ");
										else tok = strtok(NULL," ");
									}
								}

								if (tok) {
									ptr = tok;
									cons_buffer_len=strlen(tok);
								}
							} else {
								ptr = one;
								cons_buffer_len=len;
							}
							if (grepcounter==0) {
								cons_print_real(ptr);
								cons_buffer_len=1;
								cons_print_real("\n");
							} else lines_counter++;
						}
						two[0] = '\n';
						one = two + 1;
					} else break;
				}
			} else cons_print_real(cons_buffer);
		}

		cons_buffer[0] = '\0';
	}
	//cons_buffer_sz=0;

	if (grepcounter) {
		char buf[64];
		sprintf(buf, "%d\n", lines_counter);
		cons_buffer_len = strlen(buf);
		cons_print_real(buf);
	}

	cons_buffer_len=0;
}

/* stream is ignored */
void cons_fprintf(FILE *stream, const char *format, ...)
{
	/* dupped */
	int len;
	char buf[CONS_BUFSZ];
	va_list ap;

	va_start(ap, format);

	len = vsnprintf(buf, CONS_BUFSZ-1, format, ap);
	if (len>0) {
		len = strlen(buf);
		palloc(len);
		memcpy(cons_buffer+cons_buffer_len, buf, len+1);
		cons_buffer_len += len;
	}

	va_end(ap);
}

void cons_printf(const char *format, ...)
{
	int len;
	char buf[CONS_BUFSZ];
	va_list ap;

	if (strchr(format,'%')==NULL) {
		cons_strcat(format);
		return;
	}

	va_start(ap, format);

	len = vsnprintf(buf, CONS_BUFSZ-1, format, ap);
	if (len>0) {
		// TODO: use cons_strcat here
		palloc(len+1024);
	//	cons_lines += cons_lines_count(buf);
		memcpy(cons_buffer+cons_buffer_len, buf, len+1);
		cons_buffer_len += len;
	}

	va_end(ap);
}

void cons_strcat(const char *str)
{
	int len = strlen(str);
	if (len>0) {
		palloc(len+1024);
	//	cons_lines += cons_lines_count(str);
		memcpy(cons_buffer+cons_buffer_len, str, len+1);
		cons_buffer_len += len;
	}
}

void cons_newline()
{
	if (cons_is_html)
		cons_strcat("<br />\n");
	else cons_strcat("\n");
#if 0
#if RADARE_CORE
	if (!config.buf)
		cons_flush();
#endif
#endif
}

int cons_get_columns()
{
	int columns_i = cons_get_real_columns();
	char buf[64];

	if (columns_i<2)
		columns_i = 78;

	sprintf(buf, "%d", columns_i);
	setenv("COLUMNS", buf, 0);

	return columns_i;
}

int cons_get_real_columns()
{
#if __WINDOWS__
	config.width = 80;
	config.height = 23;
	return 80;
#elif __UNIX__
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
}

#ifdef RADARE_CORE
int yesno(int def, const char *fmt, ...)
{
	va_list ap;
	char key = (char)def;
	if (config.visual)
		key='y';
	else D {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fflush(stderr);
		cons_set_raw(1);
		read(0, &key, 1); write(2, "\n", 1);
		if (key == 'Y') key = 'y'; // tolower fix
		cons_set_raw(0);
		if (key=='\n'||key=='\r')
			key = def;
	} else key = 'y';
	return key=='y';
}
#endif

/**
 *
 * void cons_set_raw( [0,1] )
 *
 *   Change canonicality of the terminal
 *
 * For optimization reasons, there's no initialization flag, so you need to
 * ensure that the make the first call to cons_set_raw() with '1' and
 * the next calls ^=1, so: 1, 0, 1, 0, 1, ...
 *
 * If you doesn't use this order you'll probably loss your terminal properties.
 *
 */
#if __UNIX__
static struct termios tio_old, tio_new;
#endif
static int termios_init = 0;

void cons_set_raw(int b)
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

int cons_get_arrow(int ch)
{
	if (ch==0x1b) {
		ch = cons_readchar();
		if (ch==0x5b) {
			// TODO: must also work in interactive visual write ascii mode
			ch = cons_readchar();
			switch(ch) {
				case 0x35: ch='K'; break; // re.pag
				case 0x36: ch='J'; break; // av.pag
				case 0x41: ch='k'; break; // up
				case 0x42: ch='j'; break; // down
				case 0x43: ch='l'; break; // right
				case 0x44: ch='h'; break; // left
				case 0x3b:
					   break;
				default:
					   ch = 0;
			}
		}
	}
	return ch;
}

void cons_any_key()
{
	D cons_strcat("\n--press any key--\n");
	cons_flush();
	cons_readchar();
	cons_strcat("\x1b[2J\x1b[0;0H");
}
