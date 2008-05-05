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
    int (*hook) (char *input);
} command_t;

#define CMD_NAME(hook) cmd_ ## hook
#define CMD_DECL(hook) int cmd_ ## hook (char *input)
#define CMD_CALL(hook,input) cmd_ ## hook(input);
#define COMMAND(sn,opt,help,hook) {sn, (char*)opt, help, cmd_ ## hook}

int commands_parse(const char *cmdline);
void show_help_message();

CMD_DECL(gotoxy);
CMD_DECL(menu);
CMD_DECL(baddr);
CMD_DECL(seek0);
CMD_DECL(hack);
CMD_DECL(store);
CMD_DECL(blocksize);
CMD_DECL(count);
CMD_DECL(code);
CMD_DECL(show_info);
CMD_DECL(envvar);
CMD_DECL(compare);
CMD_DECL(dump);
CMD_DECL(endianess);
CMD_DECL(limit);
CMD_DECL(move);
CMD_DECL(print);
CMD_DECL(quit);
CMD_DECL(resize);
CMD_DECL(seek);
CMD_DECL(undoseek);
CMD_DECL(status);
CMD_DECL(rdb);
CMD_DECL(project);
CMD_DECL(yank);
CMD_DECL(yank_paste);
CMD_DECL(visual);
CMD_DECL(write);
CMD_DECL(examine);
CMD_DECL(prev);
CMD_DECL(next);
CMD_DECL(prev_align);
CMD_DECL(next_align);
CMD_DECL(search);
CMD_DECL(shell); 
CMD_DECL(cmd);
CMD_DECL(help);
CMD_DECL(flag);
CMD_DECL(interpret);
CMD_DECL(interpret_perl);
CMD_DECL(echo);
CMD_DECL(open);
CMD_DECL(math);
CMD_DECL(width);
CMD_DECL(hash);
CMD_DECL(config_eval);
CMD_DECL(default);

#endif
/* vim:set ts=4 sw=4 tw=80 foldmethod=marker expandtab: */
