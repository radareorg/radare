/*
 * Copyright (C) 2006 Lluis Vilanova <xscript@gmx.net>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _INCLUDE_CMDS_H_
#define _INCLUDE_CMDS_H_

typedef struct command {
    char sname;
    char *options;
    char *help;
    void (*hook) (char *input);
} command_t;

#define CMD_NAME(hook) cmd_ ## hook
#define CMD_DECL(hook) void cmd_ ## hook (char *input)
#define CMD_CALL(hook,input) cmd_ ## hook(input);
#define COMMAND(sn,opt,help,hook) {sn, (char*)opt, help, cmd_ ## hook}

void commands_parse(const char *cmdline);
void show_help_message();

#endif
/* vim:set ts=4 sw=4 tw=80 foldmethod=marker expandtab: */
