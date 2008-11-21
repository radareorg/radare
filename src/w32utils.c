/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * + 2006-05-12 Lluis Vilanova xscript <gmx.net>
 * 	Code refactorization and unbounded search
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

#if __WINDOWS__ 

#include <sys/types.h>
#include <windows.h>
#include <winsock.h>

static char buf[1024];

char *getenv(const char *str)
{
	GetEnvironmentVariable(str, &buf, 1023);
	return &buf;
}

int unsetenv(const char *foo)
{
	SetEnvironmentVariable(foo, NULL);
	return 0;
}

int setenv(const char *foo, const char *bar)
{
	SetEnvironmentVariable(foo, bar);
	return 0;
}

int fork()
{
	fprintf(stderr, "fork: not available for w32\n");
}

int sleep(int s)
{
	Sleep(s*1000);
}

int usleep(int s)
{
	Sleep(s);
}

int mkstemp(char *path)
{
	int fd;
	strcpy(path, "foo.tmp");
	fd = open(path, O_RDONLY|O_CREAT, 0666);
	return fd;
}

WINSOCK_API_LINKAGE u_short PASCAL ntohs(u_short foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_short PASCAL htons(u_short foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_long PASCAL ntohl(u_long foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE u_long PASCAL htonl(u_long foo)
{
	// swap endian here !!
	return 0;
}

WINSOCK_API_LINKAGE SOCKET PASCAL accept(SOCKET fd,struct sockaddr* sa,int* le)
{
	return -1;
}

/* code ripped from OpenBSD */
#if defined(LIBC_SCCS) && !defined(lint)
/* static char sccsid[] = "from: @(#)getopt.c	8.2 (Berkeley) 4/2/94"; */
static char *rcsid = "$Id: getopt.c,v 1.2 1998/01/21 22:27:05 billm Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt,			/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
getopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	static char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */

	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			++optind;
			place = EMSG;
			return (-1);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", nargv[0], optopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			optarg = place;
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    nargv[0], optopt);
			return (BADCH);
		}
	 	else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return (optopt);			/* dump back option letter */
}

#endif
