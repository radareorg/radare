/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
 *       th0rpe <nopcode.org>
 *
 * libps2fd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libps2fd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libps2fd; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "../libps2fd.h"
#if __UNIX__
#include <sys/wait.h>
#endif
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
  { SIGINT,  "SIGINT",  "Interrupt"},
  { SIGQUIT, "SIGQUIT", "Quit"},
  { SIGILL,  "SIGILL",  "Illegal instruction"},
  { SIGTRAP, "SIGTRAP", "Trace/breakpoint trap"}, // noisy for step
  { SIGABRT, "SIGABRT", "Aborted"},
  { SIGFPE,  "SIGFPE",  "Arithmetic exception" },
  { SIGKILL, "SIGKILL", "Killed"},
  { SIGBUS,  "SIGBUS",  "Bus error"},
  { SIGSEGV, "SIGSEGV", "Segmentation fault"},
  { SIGUSR2, "SIGSYS",  "Bad system call"},
  { SIGPIPE, "SIGPIPE", "Broken pipe"},
  { SIGALRM, "SIGALRM", "Alarm clock"},
  { SIGTERM, "SIGTERM", "Terminated"},
  { SIGURG,  "SIGURG",  "Urgent I/O condition (SIGURG, SIGSTKFLT)"},
  { SIGSTOP, "SIGSTOP", "Stopped (signal)"},
  { SIGTSTP, "SIGTSTP", "Stopped (user)"},
  { SIGCONT, "SIGCONT", "Continued"},
  { SIGCHLD, "SIGCHLD", "Child status changed"},
  { SIGTTIN, "SIGTTIN", "Stopped (tty input)"},
#if 0
  { "SIGTTOU",    "Stopped (tty output)"},
  {"SIGIO",    "I/O possible"},
  {"SIGXCPU",  "CPU time limit exceeded"},
  {"SIGXFSZ",  "File size limit exceeded"},
  {"SIGVTALRM","Virtual timer expired"},
  {"SIGPROF", "Profiling timer expired"},
  {"SIGWINCH", "Window size changed"},
  {"SIGLOST", "Resource lost"},
  {"SIGUSR1",    "User defined signal 1"},
  {"SIGUSR2",    "User defined signal 2"},
  {"SIGPWR",     "Power fail/restart"},
  {"SIGPOLL",    "Pollable event occurred"},
  {"SIGWIND",    "SIGWIND"},
  {"SIGPHONE",   "SIGPHONE"},
  {"SIGWAITING", "Process's LWPs are blocked"},
  {"SIGLWP", "Signal LWP"},
  {"SIGDANGER", "Swap space dangerously low"},
  {"SIGGRANT", "Monitor mode granted"},
  {"SIGRETRACT", "Need to relinquish monitor mode"},
  {"SIGMSG", "Monitor mode data available"},
  {"SIGSOUND", "Sound completed"},
  {"SIGSAK", "Secure attention"},
  {"SIGPRIO", "SIGPRIO"},
  {"SIGCANCEL", "LWP internal signal"},
  {"SIGINFO", "Information request"},
#endif
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
	case	-1:
		printf("%-10s (ERROR)\n", signame);
		break;
	default:
		printf("%-10s (0x%x)\n", signame, (unsigned int)handler); 
	}
}
