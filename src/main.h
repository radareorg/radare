#ifndef _INCLUDE_MAIN_H_
#define _INCLUDE_MAIN_H_

#include "../global.h"

#undef __BSD__
#undef __UNIX__
#undef __WINDOWS__

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define __BSD__ 1
#endif

#if __WIN32__ || __CYGWIN__ || MINGW32

#define __addr_t_defined
#include <windows.h>
#ifdef USE_SOCKETS
#include <winsock.h>
#undef USE_SOCKETS
#endif

#define __WINDOWS__ 1
#else
#define __UNIX__ 1
#endif

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "types.h"
#include "print.h"
#include "list.h"
#include "config.h"
#include "radare.h"
#include "data.h"
#include "flags.h"
#include "utils.h"
#include "code.h"
#include "io.h"
#include "plugin.h"
#include "section.h"
#include "ranges.h"

#endif
