/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *
 * dietline is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * dietline is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dietline; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "dietline.h"

/* dietline is a lighweight and portable library similar to GNU readline */

#if RADARE_CORE
#include "main.h"
#else
static void cons_set_raw(int b);
static int cons_get_real_columns();
#define __UNIX__ 1
#endif

#include <string.h>
#include <stdlib.h>

#if __WINDOWS__
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#endif

/* line input */
int dl_echo = 1;
const char *dl_prompt = "> ";
const char *dl_clipboard = NULL;
static char *dl_nullstr = "";
static char dl_buffer[DL_BUFSIZE];
static int dl_buffer_len = 0;
static int dl_buffer_idx = 0;

/* autocompletion callback */
char **(*dl_callback)(const char *text, int start, int end) = NULL;

/* history */
char **dl_history = NULL;
int dl_histsize = DL_HISTSIZE;
int dl_histidx = 0;
int dl_autosave = 0; // TODO
int dl_disable = 0; // TODO use fgets..no autocompletion

// TODO : FULL READLINE COMPATIBILITY
// rl_attempted_completion_function = rad_autocompletion;
// char **rad_autocompletion(const char *text, int start, int end)
// return  matches = rl_completion_matches (text, rad_offset_matches);

int dl_readchar()
{
	char buf[2];
#if __WINDOWS__
	LPDWORD out;
	BOOL ret;
	LPDWORD mode;
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

	GetConsoleMode(h, &mode);
	SetConsoleMode(h, 0); // RAW
	ret = ReadConsole(h, buf,1, &out, NULL);
	if (!ret) {
		// wine hack-around
		if (read(0,buf,1) == 1)
			return buf[0];
		return -1;
	}
	SetConsoleMode(h, mode);
#else
	int ret = read(0,buf,1);
	if (ret <1)
		return -1;
#endif
	return buf[0];
}


/* history stuff */
int dl_hist_add(const char *line)
{
#if HAVE_LIB_READLINE
	add_history(line);
#else
	if (dl_histidx>=dl_histsize)
		dl_histidx = 0; // workaround
	if (*line) { // && dl_histidx < dl_histsize) {
		dl_history[dl_histidx++] = strdup(line);
		return 1;
	}
	return 0;
#endif
}

int dl_hist_up()
{
	if (dl_histidx>0) {
		strncpy(dl_buffer, dl_history[--dl_histidx], DL_BUFSIZE-1);
		dl_buffer_idx=
		dl_buffer_len = strlen(dl_buffer);
		return 1;
	}
	return 0;
}

int dl_hist_down()
{
	dl_buffer_idx=0;
	if (dl_histidx<dl_histsize) {
		if (dl_history[dl_histidx] == NULL) {
			dl_buffer[0]='\0';
			dl_buffer_idx = dl_buffer_len = 0;
			return 0;
		}
		strncpy(dl_buffer, dl_history[dl_histidx++], DL_BUFSIZE-1);
		dl_buffer_idx=
		dl_buffer_len = strlen(dl_buffer);
		return 1;
	}
	return 0;
}

int dl_hist_list()
{
	int i = 0;

	if (dl_history != NULL)
	for(i=0;i<dl_histsize; i++) {
		if (dl_history[i] == NULL)
			break;
		printf("%.3d  %s\n", i, dl_history[i]);
	}

	return i;
}

int dl_hist_free()
{
	int i;
	if (dl_history != NULL)
	for(i=0;i<dl_histsize; i++) {
		free(dl_history[i]);
		dl_history[i] = NULL;
	}
	return dl_histidx=0, dl_histsize;
}

void dl_free()
{
	printf("Bye!\n");
	dl_hist_free();
	free(dl_history);
}

/* load history from file. if file == NULL load from ~/.<prg>.history or so */
int dl_hist_load(const char *file)
{
#if HAVE_LIB_READLINE
	rad_readline_init();
	return 0;
#else
	char buf[1024];
	FILE *fd;

	snprintf(buf, 1023, "%s/%s", getenv("HOME"), file);
	fd = fopen(buf, "r");
	if (fd == NULL)
		return 0;

	fgets(buf, 1023, fd);
	while (!feof(fd)) {
		buf[strlen(buf)-1]='\0';
		dl_hist_add(buf);
		fgets(buf, 1023, fd);
	}
	fclose(fd);

	return 1;
#endif
}

int dl_hist_save(const char *file)
{
#if HAVE_LIB_READLINE
	rad_readline_finish();
#else
	char buf[1024];
	FILE *fd;
	int i;

	snprintf(buf, 1023, "%s/%s", getenv("HOME"), file);
	fd = fopen(buf, "w");
	if (fd == NULL)
		return 0;
	for(i=0;i<dl_histidx;i++) {
		fputs(dl_history[i], fd);
		fputs("\n", fd);
	}
	fclose(fd);
	
	return 1;
#endif
}

int dl_hist_chop(const char *file, int limit)
{
	/* TODO */
	return 0;
}

/* initialize history stuff */
int dl_init()
{
#if HAVE_LIB_READLINE
	rad_readline_init();
#endif
	dl_history = (char **)malloc(dl_histsize*sizeof(char *));
	if (dl_history==NULL)
		return 0;
	memset(dl_history, '\0', dl_histsize*sizeof(char *));
	dl_histidx = 0;
	dl_histsize = DL_HISTSIZE;
	dl_histidx = 0;
	dl_autosave = 0;
	dl_disable = 0;
	return 1;
}

/* test */
int dl_printchar()
{
	unsigned char buf[10];

	cons_set_raw(1);
	buf[0]=dl_readchar();

	switch(buf[0]) {
		case 226:
		case 197:
		case 195:
		case 194:
			buf[0] = dl_readchar();
			printf("unicode-%02x-%02x\n", buf[0],buf[1]);
			break;
		case 8: // wtf is 127?
		case 127: printf("backspace\n"); break;
		case 32: printf("space\n"); break;
		case 27:
			read(0, buf, 5);
			printf("esc-%02x-%02x-%02x-%02x\n",
					buf[0],buf[1],buf[2],buf[3]);
			break;
		case 12: printf("^L\n"); break;
		case 13: printf("intro\n"); break;
		case 18: printf("^R\n"); break;
		case 9: printf("tab\n"); break;
		case 3: printf("control-c\n"); break;
		case 0: printf("control-space\n"); break;
		default:
			printf("(code:%d)\n", buf[0]);
			break;
	}

	cons_set_raw(0);

	return buf[0];
}

/* main readline function */
char *dl_readline(int argc, const char **argv)
{
	int buf[10];
	int i, len = 0;
	int opt = 0;
	int columns = cons_get_real_columns()-2;

	dl_buffer_idx = dl_buffer_len = 0;
	dl_buffer[0]='\0';

#if RADARE_CORE
	dl_echo = config.verbose;
#endif

	if (dl_disable) {
		dl_buffer[0]='\0';
		fgets(dl_buffer, DL_BUFSIZE-1, stdin);
		dl_buffer[strlen(dl_buffer)] = '\0';
		return (*dl_buffer)? dl_buffer : NULL;
	}

	memset(&buf,0,sizeof buf);
	cons_set_raw(1);

	if (dl_echo) {
		printf("%s", dl_prompt);
		fflush(stdout);
	}

#if __UNIX__
	if (feof(stdin))
		return NULL;
#endif

	while(1) {
#if 0
		if (dl_echo) {
			printf("  (");
			for(i=1;i<argc;i++) {
				if (dl_buffer_len==0||!strncmp(argv[i], dl_buffer, dl_buffer_len)) {
					len+=strlen(argv[i])+1;
					if (len+dl_buffer_len+4 >= columns) break;
					printf("%s ", argv[i]);
				}
			}
			printf(")");
			fflush(stdout);
		}
#endif

		dl_buffer[dl_buffer_len]='\0';
		buf[0] = dl_readchar();
		
//		printf("\x1b[K\r");
		columns = cons_get_real_columns()-2;
		if (columns <1)
			columns = 40;
		if (dl_echo)
		printf("\r%*c\r", columns, ' ');

		switch(buf[0]) {
			case -1:
				return NULL;
			case 0: // control-space
				/* ignore atm */
				break;
			case 1: // ^A
				dl_buffer_idx = 0;
				break;
			case 5: // ^E
				dl_buffer_idx = dl_buffer_len;
				break;
			case 3: // ^C 
				if (dl_echo)
					printf("\n^C\n");
				dl_buffer[dl_buffer_idx = dl_buffer_len = 0] = '\0';
				goto _end;
			case 4: // ^D
				if (dl_echo)
					printf("^D\n");
				if (!dl_buffer[0]) { /* eof */
					cons_set_raw(0);
					return NULL;
				}
				break;
			case 10: // ^J -- ignore
				return dl_buffer;
			case 11: // ^K -- ignore
				break;
			case 19: // ^S -- backspace
				dl_buffer_idx = dl_buffer_idx?dl_buffer_idx-1:0;
				break;
			case 12: // ^L -- right
				dl_buffer_idx = dl_buffer_idx<dl_buffer_len?dl_buffer_idx+1:dl_buffer_len;
				if (dl_echo)
					printf("\x1b[2J\x1b[0;0H");
				fflush(stdout);
				break;
			case 21: // ^U - cut
				dl_clipboard = strdup(dl_buffer);
				dl_buffer[0]='\0';
				dl_buffer_len = 0;
				dl_buffer_idx = 0;
				break;
			case 23: // ^W
				if (dl_buffer_idx>0) {
					for(i=dl_buffer_idx-1;i&&dl_buffer[i]==' ';i--);
					for(;i&&dl_buffer[i]!=' ';i--);
					for(;i>0&&dl_buffer[i]==' ';i--);
					if (i>1) {
						if (dl_buffer[i+1]==' ')
						i+=2;
					} else if (i<0) i=0;
					strcpy(dl_buffer+i, dl_buffer+dl_buffer_idx);
					dl_buffer_len = strlen(dl_buffer);
					dl_buffer_idx = i;
				}
				break;
			case 25: // ^Y - paste
				if (dl_clipboard != NULL) {
					dl_buffer_len += strlen(dl_clipboard);
					// TODO: support endless strings
					if (dl_buffer_len < DL_BUFSIZE) {
						dl_buffer_idx = dl_buffer_len;
						strcat(dl_buffer, dl_clipboard);
					} else dl_buffer_len -= strlen(dl_clipboard);
				}
				break;
			case 16:
				dl_hist_up();
				break;
			case 14:
				dl_hist_down();
				break;
			case 27: //esc-5b-41-00-00
				buf[0] = dl_readchar();
				buf[1] = dl_readchar();
				if (buf[0]==0x5b) {
					switch(buf[1]) {
					case 0x33: // supr
						if (dl_buffer_idx<dl_buffer_len)
							strcpy(dl_buffer, dl_buffer+1);
						break;
					/* arrows */
					case 0x41:
						dl_hist_up();
						break;
					case 0x42:
						dl_hist_down();
						break;
					case 0x43:
						dl_buffer_idx = dl_buffer_idx<dl_buffer_len?dl_buffer_idx+1:dl_buffer_len;
						break;
					case 0x44:
						dl_buffer_idx = dl_buffer_idx?dl_buffer_idx-1:0;
						break;
					}
				}

				break;
			case 8:
			case 127:
				if (dl_buffer_idx < dl_buffer_len) {
					if (dl_buffer_idx>0) {
						dl_buffer_idx--;
						memcpy(dl_buffer+dl_buffer_idx, dl_buffer+dl_buffer_idx+1,strlen(dl_buffer+dl_buffer_idx));
					}
				} else {
					dl_buffer_idx = --dl_buffer_len;
					if (dl_buffer_len<0) dl_buffer_len=0;
					dl_buffer[dl_buffer_len]='\0';
				}
				if (dl_buffer_idx<0)
					dl_buffer_idx = 0;
				break;
			case 9:// tab
				/* autocomplete */
				// XXX does not autocompletes correctly
				// XXX needs to check if valid results have the same prefix (from 1 to N)
				if (dl_callback != NULL) {
					//const char *from = strrchr(dl_buffer, ' ');
					//char **res = dl_callback(dl_buffer, (from==NULL)?dl_buffer_idx:from-dl_buffer, dl_buffer_len);
					/* TODO: manage res */
				} else {
					if (dl_buffer_idx>0)
					for(i=1,opt=0;i<argc;i++)
						if (!strncmp(argv[i], dl_buffer, dl_buffer_idx))
							opt++;

					if (dl_buffer_len>0&&opt==1)
						for(i=1;i<argc;i++) {
							if (!strncmp(dl_buffer, argv[i], dl_buffer_len)) {
								strcpy(dl_buffer, argv[i]);
								dl_buffer_idx = dl_buffer_len = strlen(dl_buffer);
								// TODO: if only 1 keyword hits:
								//		if (argv[i][dl_buffer_len]=='\0') {
								//			strcat(dl_buffer, " ");
								//			dl_buffer_len++;
								//		}
								break;
							}
						}

					/* show options */
					if (dl_buffer_idx==0 || opt>1) {
						if (dl_echo)
							printf("%s%s\n",dl_prompt,dl_buffer);
						for(i=1;i<argc;i++) {
							if (dl_buffer_len==0||!strncmp(argv[i], dl_buffer, dl_buffer_len)) {
								len+=strlen(argv[i]);
					//			if (len+dl_buffer_len+4 >= columns) break;
								if (dl_echo)
									printf("%s ", argv[i]);
							}
						}
						if (dl_echo)
							printf("\n");
					}
					fflush(stdout);
				}
				break;
			case 18:
				// TODO: SUPPORT FOR ^R (search command in history)
				break;
			case 13: 
				goto _end;
#if 0
				// force command fit
				for(i=1;i<argc;i++) {
					if (dl_buffer_len==0 || !strncmp(argv[i], dl_buffer, dl_buffer_len)) {
						printf("%*c", columns, ' ');
						printf("\r");
						printf("\n\n(%s)\n\n", dl_buffer);
						cons_set_raw(0);
						return dl_buffer;
					}
				}
#endif
			default:
				/* XXX use ^A & ^E */
				if (dl_buffer_idx<dl_buffer_len) {
					for(i = ++dl_buffer_len;i>dl_buffer_idx;i--)
						dl_buffer[i] = dl_buffer[i-1];
					dl_buffer[dl_buffer_idx] = buf[0];
				} else {
					dl_buffer[dl_buffer_len]=buf[0];
					dl_buffer_len++;
					if (dl_buffer_len>1000)
						dl_buffer_len--;
					dl_buffer[dl_buffer_len]='\0';
				}
				dl_buffer_idx++;
				break;
		}
		if (dl_echo) {
			printf("\r%s%s", dl_prompt, dl_buffer);
			printf("\r%s", dl_prompt);
		
			for(i=0;i<dl_buffer_idx;i++)
				printf("%c", dl_buffer[i]);
			fflush(stdout);
		}
	}

_end:
	cons_set_raw(0);
	if (dl_echo) {
		printf("\r%s%s\n", dl_prompt, dl_buffer);
		fflush(stdout);
	}

	if (dl_buffer[0]=='!' && dl_buffer[1]=='\0') {
		dl_hist_list();
		return dl_nullstr;
	}
	//write(1,"\n",1);
	return dl_buffer;
}

#ifndef RADARE_CORE


#if __UNIX__
static struct termios tio_old, tio_new;
#include "main.h"
#include <stdarg.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#endif

static void cons_set_raw(int b)
{
#if __UNIX__
	if (b) {
		tcgetattr(0, &tio_old);
		memcpy ((char *)&tio_new, (char *)&tio_old, sizeof(struct termios));
		tio_new.c_iflag &= ~(BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
		tio_new.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
		tio_new.c_cflag &= ~(CSIZE|PARENB);
		tio_new.c_cflag |= CS8;
		tio_new.c_cc[VMIN]=1; // Solaris stuff hehe
		tcsetattr(0, TCSANOW, &tio_new);
		fflush(stdout);
		return;
	}

	tcsetattr(0, TCSANOW, &tio_old);
	fflush(stdout);
#endif
}

static int cons_get_real_columns()
{
#if __WINDOWS__
        return 78;
#endif
        struct winsize win;

        if (ioctl(1, TIOCGWINSZ, &win)) {
                /* default values */
//                win.ws_col = 80;
 //               win.ws_row = 23;
        }

        return win.ws_col;
}

int main(int argc, const char **argv)
{
	char *ret;

	dl_histsize = 100;
	dl_prompt = "$ ";
	dl_init();

	dl_printchar();

	do {
		ret = dl_readline(argc, argv);
		if (ret) {
			printf(" [line] '%s'\n", ret);
			dl_hist_add(ret);
		}
	} while(ret!=NULL);
	dl_free();
	return 0;
}

#endif
