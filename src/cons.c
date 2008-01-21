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
#include <stdarg.h>
#if __UNIX__
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#endif

int _print_fd = 1;

int cons_readchar()
{
	char buf[2];
#if __WINDOWS__
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

int pprint_fd(int fd)
{
	if (_print_fd == 0)
		return fd;
	return _print_fd = fd;
}

static int pprintf_buffer_len = 0;
static char *pprintf_buffer = NULL;

// TODO : rename pprintf to console or so
char *pprintf_get()
{
	return pprintf_buffer;
}

void palloc(int moar)
{
	if (pprintf_buffer == NULL) {
		pprintf_buffer_len = moar+1024;
		pprintf_buffer = (char *)malloc(pprintf_buffer_len);
		pprintf_buffer[0]='\0';
	} else
	if (moar + strlen(pprintf_buffer)>pprintf_buffer_len) {
		pprintf_buffer = (char *)realloc(pprintf_buffer,
			pprintf_buffer_len+moar+strlen(pprintf_buffer)+1);
	}
}

void pprintf_flush()
{
	//eprintf("Flush\n");
	if (pprintf_buffer && pprintf_buffer[0]) {
		write(_print_fd, pprintf_buffer, strlen(pprintf_buffer));
		pprintf_buffer[0] = '\0';
	}
}

void pprintf_newline()
{
	pprintf("\n");
#if RADARE_CORE
	if (!config.buf)
		pprintf_flush();
#endif
}

void pprintf(const char *format, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, format);

	buf[0]='\0';
	if (vsnprintf(buf, 4095, format, ap)<0)
		buf[0]='\0';

	palloc(strlen(buf)+1000);
	strcat(pprintf_buffer, buf);

	va_end(ap);
}

void pstrcat(const char *str)
{
	palloc(strlen(str));
	strcat(pprintf_buffer, str);
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
