/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
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

#include <signal.h>
#include <stdio.h>
#include "../signal.h"

// <asm/signal.h>
struct {
  int sig;
  char *name;
  char *string;
} signals [] = {
  { SIGHUP,  "SIGHUP",  "Hangup"},
  {0, NULL, "Unknown signal"}
};

void debug_print_sigh(char *signame, unsigned long handler)
{
        switch (handler) {
        case (long)SIG_DFL:
                printf("%-10s (DEFAULT)\n", signame);
                break;
        case  (long)SIG_IGN:
                printf("%-10s (IGNORE)\n", signame);
                break;
        case    -1:
                printf("%-10s (ERROR)\n", signame);
                break;
        default:
                printf("%-10s (0x%x)\n", signame, (unsigned int)handler);
        }
}

