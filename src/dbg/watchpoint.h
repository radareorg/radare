#ifndef _INCLUDE_WATCHPOINT_H_
#define _INCLUDE_WATCHPOINT_H_
#include "libps2fd.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include "list.h"
#include "mem.h"
#include "arch/i386.h"
#include "list.h"

#define ACTION_STOP      0x0000
#define ACTION_MONITOR   0x0001
#define WP_REGISTER      0x0000
#define WP_MEMORY        0x0100
#define WP_FLAG          0x0200

struct watchpoint_t {
	int type;
	int action;
	off_t address;
	int len;
	char *value;
	struct list_head list;
};
#endif
