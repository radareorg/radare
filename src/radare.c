/*
 * Copyright (C) 2006, 2007, 2008
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

#if __UNIX__
#include <sys/ioctl.h>
//#include <regex.h> // NOT PARSEABLE BY TCC :O
#include <termios.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "search.h"
#include "plugin.h"
#include "config.h"
#include "cmds.h"
#include "readline.h"
#include "flags.h"
#include "radare.h"

#include "dietline.h"

u64 tmpoff = -1;
int std = 0;

static int radare_close();
/* dummy callback for dl_hist_label */
static int cb(const char *str) { radare_cmd(str,0); }

#if !DEBUGGER
int debug_step(int x) { return 0; }
#endif

int radare_system(const char *cmd)
{
#if __FreeBSD__
	/* freebsd system() is broken */
	int fds[2];
	int st,pid;
	char *argv[] ={ "/bin/sh", "-c", cmd , NULL};
	pipe(fds);
	/* not working ?? */
	//pid = rfork(RFPROC|RFCFDG);
	pid = vfork();
	if (pid == 0) {
		dup2(1, fds[1]);
		execv(argv[0], argv);
		_exit(127); /* error */
	} else {
		dup2(1, fds[0]);
		waitpid(pid, &st, 0);
	}
	return WEXITSTATUS(st);
#else
	return system(cmd);
#endif
}

int radare_systemf(const char *format, ...)
{
	va_list ap;
	char buf[4096];

	va_start(ap, format);
	buf[0]='\0';
	snprintf(buf, 4095, format, ap);
	radare_system(buf);

	va_end(ap);
}

void radare_init()
{
	config_init(1);
	flags_setenv();
	plugin_init();
}

void radare_exit()
{
	int ret;

	ret = radare_close();

	if (ret==-2)
		return;
	if ( ret == 0) {
		dl_hist_save(".radare_history");
#if 0
	// already done
		/* save project : user confirmation */
		ptr = config_get("file.project");

		if (ptr && ptr[0] ) {
			cons_set_raw(1);
			printf("Save project? (Y/n) ");
			fflush(stdout);
			read(0,&ch, 1);
			write(1, &ch, 1);
			write(1, "\n", 1);
			if (ch != 'n') 
			{
				pch = strdup (ptr);
				project_save(pch);
				free (pch);
			}
			cons_set_raw(0);
		}
#endif
	}
	exit(0);
}

static void radare_interrupt(int sig)
{
	config.interrupted = 1;
#if DEBUGGER
/* XXX This is absolutely UGLY!!! */
	if (config.debug)
#if __UNIX__
		kill(ps.pid, SIGCONT);
#else
		debug_contp(ps.tid);
#endif
#endif
}

void radare_controlc()
{
	config.interrupted = 0;
#if __UNIX__
	signal(SIGINT, radare_interrupt);
#endif
}

void radare_controlc_end()
{
	config.interrupted = 0;
#if __UNIX__
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
#endif
}

unsigned char radare_get(int delta)
{
	if (delta<0)
		delta = 0;
	if (delta>config.block_size)
		delta = config.block_size-1;
	return config.block[delta];
}

static int radare_close()
{
	radare_changes_close();
	project_close();
	return io_close(config.fd);
}

void radare_sync()
{
	u64 base;
	int limit = DEFAULT_BLOCK_SIZE;

	if (config_get_i("cfg.sections")) {
		base = section_get_base(config.seek);
		if (base != -1)
			config_set_i("file.baddr", base);
	}

	if (config.debug||config.unksize)
		return;

	/* enlarge your integer overflows */
	if (config.seek>0xfffffffffffffffLL)
		config.seek = 0;

	if (config.size!=-1) {
		if (config.block_size > config.size)
			radare_set_block_size_i(config.size);
			//config.block_size = config.size;

		if (config.seek > config.size) {
			config.seek  = config.size;
			config.limit = config.size;
		}

		if (config.block_size == 0) {
			if ( config.size < limit )
				limit = config.size;
			config.block_size = limit;
		}
	}

}

int radare_strsearch(char *str)
{
	u64 i;
	int j, ret;
	u64 seek = config.seek;
	u64 size = config.size;
	int min = 3;
	int enc = resolve_encoding(config_get("cfg.encoding")); // ASCII

	// TODO: Move to stripstr_iterate as args or so
	//encoding = resolve_encoding(config_get("cfg.encoding"));
	//min = 5;

	if (str) str=str+1;
	if (size <=0)
		size=0xbfffffff;

	radare_controlc();
	for(i = (size_t)seek; !config.interrupted && config.seek < size; i++) {
		ret = radare_read(1);
		if (ret == -1) break;
		for(j=0;j<config.block_size;j++)
			stripstr_iterate(config.block, j, min, enc, config.seek+j, str);
	}
	config.seek = seek;
	radare_controlc_end();

	return 0;
}

void radare_cmd_foreach(const char *cmd, const char *each)
{
	int i=0,j;
	char ch;
	char *word = NULL;
	char *str = strdup(each);
	struct list_head *pos;

	radare_controlc();
	while(str[i] && !config.interrupted) {
		j = i;
		for(;str[j]&&str[j]==' ';j++); // skip spaces
		for(i=j;str[i]&&str[i]!=' ';i++); // find EOS
		ch = str[i];
		str[i] = '\0';
		word = strdup(str+j);
		str[i] = ch;
		if (strchr(word, '*')) {

			/* for all flags in current flagspace */
			list_for_each(pos, &flags) {
				flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
				if (config.interrupted)
					break;

				/* filter per flag spaces */
//				if ((flag_space_idx != -1) && (flag->space != flag_space_idx))
//					continue;

				config.seek = flag->offset;
				radare_read(0);
				cons_printf("; @@ 0x%08llx (%s)\n", config.seek, flag->name);
				radare_cmd_raw(cmd,0);
			}
		} else {
			/* for all flags in current flagspace */
			list_for_each(pos, &flags) {
				flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
				if (config.interrupted)
					break;

#if 0
				/* filter per flag spaces */
				if ((flag_space_idx != -1) && (flag->space != flag_space_idx))
					continue;
#endif
//eprintf("polla(%s)(%s)\n", flag->name, word);
				if (word[0]=='\0' || strstr(flag->name, word) != NULL) {

					config.seek = flag->offset;
					radare_read(0);
					cons_printf("; @@ 0x%08llx (%s)\n", config.seek, flag->name);
					radare_cmd_raw(cmd,0);
				}
			}
#if 0
			/* ugly copypasta from tmpseek .. */
			if (strstr(word, each)) {
				if (word[i]=='+'||word[i]=='-')
					config.seek = config.seek + get_math(word);
				else	config.seek = get_math(word);
				radare_read(0);
				cons_printf("; @@ 0x%08llx\n", config.seek);
				radare_cmd(cmd,0);
			}
#endif
		}
		radare_controlc();

		free(word);
		word = NULL;
	}
	radare_controlc_end();

	free(str);
}

void radare_fortunes()
{
#if __WINDOWS__
	char *str = slurp("doc/fortunes");
#else
	char *str = slurp(DOCDIR"/fortunes");
#endif
	int lines = 0;
	char *ptr;
	int i;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	srand(getpid()+tv.tv_usec);
	if (str) {
		for(i=0;str[i];i++)
			if (str[i]=='\n')
				lines++;
		lines = (rand()%lines);
		for(i=0;str[i]&&lines;i++)
			if (str[i]=='\n')
				lines--;
		ptr = str+i;
		for(i=0;ptr[i];i++) if (ptr[i]=='\n') { ptr[i]='\0'; break; }
		eprintf("Message of the day:\n  %s\n", ptr);
		free(str);
	}
}

int radare_cmd_raw(const char *tmp, int log)
{
	int fd;
	FILE* filef;
	int i,f,fdi = 0;
	char *eof;
	char *eof2;
	char *piped;
	char file[1024], buf[1024];
	char *input, *oinput;
	char *next = NULL;
	int ret = 0;

	if (strnull(tmp))
		return 0;

	eof = strchr(tmp,'\n');
	if (eof) {
		*eof = '\0';
		if (eof[1]!='\0') { // OOPS :O
			eprintf("Multiline command not yet supported (%s)\n", tmp);
			return 0;
		}
	}

	input = oinput = strdup(tmp);
	input = strclean(input);

	if (input[0] == ':') {
		config.verbose = config_get_i("cfg.verbose")^1;
		config_set("cfg.verbose", (config.verbose)?"true":"false");
		input = input+1;
	}

//	if (dl_hist_label(input, &cb)) {
//		return 0;
//	}

 	eof = input+strlen(input)-1;

	next = strstr(input, "&&");
	if (next) next[0]='\0';

	/* interpret stdout of a process executed */
	if (input[0]=='.') {
		radare_controlc();
		switch(input[1]) {
#if 0
		case '%': {
			char *cmd = getenv(input+2);
			if (cmd)
				radare_cmd(cmd, 0);
			free(oinput);
			return 1;
			}
#endif
		case '?':
			cons_printf(
			"Usage [.][command| file]\n"
			"- Interpret radare commands from file or command\n"
			"  > .!regs*               ; interpret the output of a command\n"
			"  > . /tmp/flags-saved    ; load radare script file\n"
			"  > . my-script.py        ; depends on file extension\n");
			break;
		case ' ':
			filef = fopen(input+2,"r");
			if (filef) {
				while(!feof(filef)) {
					buf[0]='\0';
					fgets(buf, 1000, filef);
					if (buf[0]!='\0')
						radare_cmd(buf, 0);
				}
				fclose(filef);
			} else
				cons_printf("oops (%s)\n", input+2);
			break;
		default:
			#if __WINDOWS__
			/* XXX hack to parse .!regs* on w32 */
			/* XXX w32 port needs a system replacement */
			/* maybe everything but debug commands must be in this way */
			/* radare_cmd_str doesn't handle system() output :( */
			if( (strstr(input,"!regs")) ||(strstr(input,"!maps"))) {
			//if (1) {
				str = radare_cmd_str(input+1);
				st = str;
				for(i=0;str && str[i];i++) {
					if (str[i]=='\n') {
						str[i]='\0';
						radare_cmd(st, 0);
						st = str+i+1;
					}
				}
				config_set_i("cfg.verbose", 1);
				free(oinput);
				return 0;
			}
			#endif
			// this way doesnt workz
			pipe_stdout_to_tmp_file(file, input+1);
			f = open(file, O_RDONLY);
			if (f == -1) {
				eprintf("radare_cmd_raw: Cannot open.\n");
				config_set_i("cfg.verbose", 1);
				free(oinput);
				return 0;
			}
			for(;!config.interrupted;) {
				buf[0]='\0';
				for(i=0;i<1000;i++) {
					if (read(f, &buf[i], 1)<=0) {
						i = -1;
						break;
					}
					if (buf[i]=='\n') {
						buf[i]='\0';
						break;
					}
				}
				if (i==-1) break;
				if (buf[0])
					radare_cmd(buf, 0);
				//cons_flush();
				config_set_i("cfg.verbose", 1);
			}
			close(f);

			unlink(file);
			break;
		}
		radare_controlc_end();
	/* other commands */
	} else {
		/* pipe */
		piped = strchr(input, '|');
		if (piped && input != piped && piped[-1]!='\\') {
			char tmp[1024];
			char cmd[1024];
			piped[0]='\0';
			pipe_stdout_to_tmp_file(tmp, input);
			snprintf(cmd, 1023 ,"cat '%s' | %s", tmp, piped+1);
			ret = io_system(cmd);
			unlink(tmp);
			piped[0]='|';
			free(oinput);
			return 1;
		}
		fd = -1;
		fdi = -1;
		std = -1;
		if (input[0]!='%' && input[0]!='!' && input[0]!='_' && input[0]!=';' && input[0]!='?') {
			/* inline pipe */
			piped = strchr(input, '`');
			if (piped) {
				int len;
				char tmp[128];
				char filebuf[4096];
				piped[0]='\0';

				pipe_stdout_to_tmp_file(tmp, piped+1);
				fdi = open(tmp, O_RDONLY);
				if (fdi == -1) {
					perror("open");
					free(oinput);
					return 0;
				}

				memset(filebuf, '\0', 2048);
				len = read(fdi, filebuf, 1024);
				if (len<1) {
					eprintf("cannot read?\n");
					return 0;
				}
				len += strlen(input) + 5;
				oinput = malloc(len);
				sprintf(oinput, "%s %s", input, filebuf);
				free(input);
				input = oinput;
			}

			if (input[0] && input[0]!='>' && input[0]!='/') { // first '>' is '!'
				char *pos = strchr(input+1, '>');
				char *file = pos + 1;
				char *end;
				if (pos != NULL) {
					for(pos[0]='\0';iswhitespace(file[0]); file = file + 1);
					if (*file == '\0') {
						eprintf("No target file (%s)\n", input);
					} else {
						for(end = file+strlen(file);
							end[0]&&!iswhitespace(end[0]);
							end = end-1); end[0]='\0';
						if (pos[1] == '>')
							fd = io_open(file, O_APPEND|O_WRONLY, 0644);
						else	fd = io_open(file, O_TRUNC|O_WRONLY|O_CREAT, 0644);
						if (fd == -1) {
							eprintf("Cannot open '%s' for writing\n", file);
							config_set("file.write", "false");
							tmpoff = config.seek;
							free(oinput);
							return 1;
						}
						std = dup(1); // store stdout
						dup2(fd, 1);
					}
				}
			}

			/* temporally offset */
			eof2 = strchr(input, '@');
			if (eof2 && input && input[0]!='e') {
				char *ptr = eof2+1;
				eof2[0] = '\0';

				if (eof2[1]=='@') {
					/* @@ is for foreaching */
					tmpoff = config.seek;
					radare_cmd_foreach(input ,eof2+2);
					//config.seek = tmpoff;
					radare_seek(tmpoff, SEEK_SET);
					
					return 0;
				} else {
					tmpoff = config.seek;
					for(;*ptr==' ';ptr=ptr+1);
					if (*ptr=='+'||*ptr=='-')
						config.seek = config.seek + get_math(ptr);
					else	config.seek = get_math(ptr);
					radare_read(0);
				}
			}

		}

		// XXX fuckmenot
		//for(;eof!=input && eof>1 && eof[0] && iswhitespace(eof[0]); eof=eof-1) eof[0]='\0';

		if (input[strlen(input)]=='\n')
			input[strlen(input)] = '\0';
		for(eof=input;eof[0];eof=eof+1)
			if (eof[0]=='\n') eof[0]=' ';
		ret = commands_parse(input);
		cons_flush();

		if (fdi!=-1) {
			fflush(stdout);
			if (std)
			dup2(std, 1);
			if (fdi != 0)
				close(fdi);
		} else
		if (fd!=-1) {
			fflush(stdout);
			if (std)
			dup2(std, 1);
			if (fd != 0)
				close(fd);
		}

		if (std!=-1) {
			fflush(stdout);
			dup2(std, 1);
			std = 0;
		}

		/* restore seek */
		if (tmpoff != -1) {
			config.seek = tmpoff;
			tmpoff = -1;
		}
	}
	
	if (next && next[1]=='&') {
		int ret;
		next[0] = '&';
		for(next=next+2;*next==' ';next=next+1);

		//free(oinput);
		ret = radare_cmd(next, 0);
	//	cons_flush();
		return ret; 
	}

	free(oinput);
	return ret; /* error */
}

int radare_cmdf(const char *cmd, ...)
{
	va_list ap;
	int ret;
	char buf[1024];
	va_start(ap, cmd);
	vsnprintf(buf, 1023, cmd, ap);
	ret = radare_cmd_raw(buf, 0);
	va_end(ap);
	return ret;
}

/* XXX this is more portable and faster than the solution pipe_to_tmp_file and so */
/* but doesnt supports system() stuff. i have to split the use of both functions */
char *radare_cmd_str(const char *cmd)
{
	char *buf;
	char *dcmd;
	int scrbuf = config_get_i("scr.buf");
	int cfgver = config_get_i("cfg.verbose");

	cons_reset();
	config_set_i("scr.buf", 1);

	dcmd = strdup ( cmd );
	radare_cmd( dcmd, 0);
	free ( dcmd);
	buf = cons_get_buffer();
	if (buf)
		buf = strdup(buf);
	config_set_i("scr.buf", scrbuf);
	config_set_i("cfg.verbose", cfgver);
	cons_reset();

	return buf;
}

/* default debugger interface */
void radare_nullcmd()
{
	char buf[128];
	const char *ptr;
	int p,i;
	int verborig;

	if (!config.debug)
		return;

	i = radare_read(0);
	if (i<0)
		return;
	ptr = config_get("cmd.visual");
	if (!strnull(ptr)) {
		char *ptrcmd = strdup(ptr);
		radare_cmd_raw(ptrcmd, 0);
		free(ptrcmd);
	}
	verborig = config_get_i("cfg.verbose");
	config_set("cfg.verbose", "false");
	p = last_print_format;

	/* NOT REQUIRED update flag registers NOT REQUIRED */
	//radare_cmd(":.!regs*", 0);

	if (config_get("dbg.stack")) {
		C cons_printf(C_RED"Stack: "C_RESET);
		else cons_printf("Stack: ");
		radare_cmd("?x ebp-esp", 0); //":px 66@esp", 0);
		sprintf(buf, "%spx %d @ %s",
			(config_get("dbg.vstack"))?":":"",
			(int)config_get_i("dbg.stacksize"),
			config_get("dbg.stackreg"));
		radare_cmd(buf, 0); //":px 66@esp", 0);
	}

	if (config_get("dbg.regs")) {
		C cons_printf(C_RED"Registers:\n"C_RESET);
		else cons_printf("Registers:\n");
		radare_cmd("!regs", 0);
	}

	if (config_get("dbg.fpregs")) {
		C cons_printf(C_RED"Floating registers:\n"C_RESET);
		else cons_printf("Floating registers:\n");
		radare_cmd("!fpregs", 0);
	}

	config_set_i ("cfg.verbose", 1);
	if (config_get("dbg.bt")) {
		if (config_get("dbg.fullbt")) {
			C cons_printf(C_RED"Full Backtrace:\n" C_YELLOW C_RESET);
			else cons_printf("Full Backtrace:\n");
			radare_cmd(":!bt", 0);
		} else {
			C cons_printf(C_RED"User Backtrace:\n" C_YELLOW C_RESET);
			else cons_printf("User Backtrace:\n");
			radare_cmd("!bt", 0);
		}
	}

	C cons_printf(C_RED"Disassembly:\n"C_RESET);
	else cons_printf("Disassembly:\n");
	if (config_get("dbg.dwarf"))
		radare_cmd("pR @ eip",0);
	{
		u64 pc = flag_get_addr("eip");
		if (pc<config.seek || pc > config.seek+config.block_size)
			radare_seek(pc, SEEK_SET);
	}
	config_set("cfg.verbose", "true");
	config.verbose=1;
	/* TODO: chose pd or pD by eval */
	radare_cmd("pD", 0);

	config_set_i ("cfg.verbose", verborig);
	last_print_format = p;
}


int radare_cmd(char *input, int log)
{
	const char *ptr;
	int repeat;
	int p,i;
	char buf[128];
	char *next = NULL;
	int ret = 0;

	/* silently skip lines begginging with 0 */
	if(input==NULL || (log&&input==NULL) || (input&&input[0]=='0'))
		return 0;

	if (dl_hist_label(input, &cb))
		return 0;
//fprintf(stderr, "CMD: (%s)\n", input);
	if (log) {
		dl_hist_add(input);
	}

	next = strstr(input, "&&");
	if (next) next[0]='\0';

// XXX not handled !?!?
	if (input[0]=='!'&&input[1]=='!')
		return radare_system(input+2);

	// bypass radare inputline hack ;D
	if (!memcmp(input, "[0x", 3)) {
		char *foo = strchr(input, '>');
		if (foo)
			input = foo+2;
	}

	if (input && input[0]=='\0') {
		radare_nullcmd();
		return 0;
	}

	if (config.skip) return 0;

	if (input[0] == ':') {
		config.verbose = config_get_i("cfg.verbose")^1;
		config_set("cfg.verbose", (config.verbose)?"true":"false");
		input = input+1;
	}

	/* repeat stuff */
	if (input)
		repeat = atoi(input);
	if (repeat<1)
		repeat = 1;
	for(;input&&(input[0]>='0'&&input[0]<='9');)
		input=input+1;

	/* no command found? */
	if (input[0]=='\0')
		return 0;

	radare_controlc();
	for(i=0;!config.interrupted&&i<repeat;i++) {
		ret = radare_cmd_raw(input, log);
		/* TODO: do something with ret */
	}
	radare_controlc_end();

	if (next && next[1]=='&') {
		int ret;
		next[0] = '&';
		for(next=next+2;*next==' ';next=next+1);

		//free(oinput);
		ret = radare_cmd(next, 0);
	//	cons_flush();
		return ret; 
	}

	return ret;
}

/* TODO: move to cmds.c */
int radare_interpret(char *file)
{
	int len;
	char buf[1024];
	FILE *fd;

	if (file==NULL || file[0]=='\0')
		return 0;
	
	/* check for perl/python/lua/ */
	buf[0]='\0';
	if (strstr(file, ".rb"))
		snprintf(buf, 1012, "H ruby %s", file);
	else
	if (strstr(file, ".pl"))
		snprintf(buf, 1012, "H perl %s", file);
	else
	if (strstr(file, ".py"))
		snprintf(buf, 1012, "H python %s", file);
	else
	if (strstr(file, ".lua"))
		snprintf(buf, 1012, "H lua %s", file);

	if (buf[0])
		return radare_cmd(buf, 0);

	/* failover to simple radare script */
	fd = fopen(file, "r");
	if (fd == NULL)
		return 0;

	while(!feof(fd) && !config.interrupted) {
		buf[0]='\0';
		fgets(buf, 1024, fd);
		if (buf[0]=='\0') break;
		len = strlen(buf);
		if (len>0) buf[strlen(buf)-1]='\0';
		radare_cmd(buf, 0);
		config.verbose = 0;
		config_set("cfg.verbose", "false");
	}
	fclose(fd);

	return 1;
}

int stdout_fd = 6676;
int stdout_file = -1;
void stdout_open(char *file)
{
	int fd = open(file, O_RDONLY);
	if (fd==-1)
		return;
	stdout_file = fd;
	dup2(1, stdout_fd);
	//close(1);
	dup2(fd, 1);
}

void stdout_close()
{
	dup2(stdout_fd, 1);
	//close(stdout_file);
}

int radare_move(char *arg)
{
	u64 src = config.seek;
	u64 len =  0;
	u64 pos = -1;
	char *str;
	u8 *buf;

	while(*arg==' ')arg=arg+1;
	str = strchr(arg, ' ');
	if (str) {
		str[0]='\0';
		len = get_math(arg);
		pos = get_math(str+1);
		str[0]=' ';
	}
	if ( (str == NULL) || (pos == -1) || (len == 0) ) {
		printf("Usage: yf [len] [dst-addr]\n");
		return 1;
	}

	if (!config_get("file.write")) {
		eprintf("You are not in read-write mode.\n");
		return 1;
	}

	buf = (u8*)malloc( len );
	radare_read_at(src, buf, len);
	radare_write_at(pos, buf, (int)len);
	free(buf);

	config.seek = src;
	radare_read(0);
	return 0;
}

void radare_prompt_command()
{
	const char *ptr;
	char* aux;

	if (config_get("cfg.vbsze_enabled")) {
		struct list_head *pos;
		u64 uh = config.seek + config_get_i("cfg.vbsize");
		u64 old = uh;
		list_for_each(pos, &flags) {
			flag_t *flag = (flag_t *)list_entry(pos, flag_t, list);
			if (flag->offset > config.seek && (flag->offset < old))
				old = flag->offset;
		}
		if (old)
			radare_set_block_size_i(old-config.seek);
	}

	/* define cursor flag */
	if (config.cursor_mode)
		flag_set("cursor", config.cursor, 0);
	else flag_set("cursor", 0, 0);

	/* user defined command */
	ptr = config_get("cmd.prompt");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		aux = strdup ( ptr );
		radare_cmd_raw(aux, 0);
		free ( aux );
		last_print_format = tmp;
	}

	if (config.debug)
		radare_cmd(".!regs*", 0);

	monitors_run();
}

void monitors_run()
{
	char file[1024];
	char path[1024];
	int i;
	FILE *fd;
	const char *ptr;
	int tmp;
	struct dirent *de;
	DIR *dir;

	/* run the commands found in the monitor path directory */
	*path='\0';
	if ( (ptr = config_get("dir.monitor")) ) {
		strncpy(path, ptr, 1023);
	} else {
		ptr = config_get("dir.home");
		if (ptr) {
			sprintf(path, "%s/.radare/monitor", ptr);
		} /* else silently unexistence of HOME */
	}
	if (*path) {
		tmp = last_print_format;
		dir = opendir(path);
		if (dir) {
			while((de = (struct dirent *)readdir(dir))) {
				if (de->d_name[0] != '.' && !strstr(de->d_name, ".txt")) {
					sprintf(file, "%s/%s", path, de->d_name);
					fd = fopen(file, "r");
					if (fd) {
						strcat(file, ".txt");
						_print_fd = open(file, O_RDWR|O_TRUNC);
						if (_print_fd == -1)
							_print_fd = open(file, O_RDWR|O_CREAT|O_TRUNC, 0644);
						if (_print_fd == -1) {
							_print_fd = 1;
							continue;
						}
						while(1) {
							file[0]='\0';
							fgets(file, 1023, fd);
							if (file[0]=='\0') break;
							file[strlen(file)-1]='\0';
							for(i=strlen(file);i;i--) {
								if (file[i]==' ')
									file[i]='\0';
							}
							radare_cmd(file, 0);
						}
		//				cons_flush();
						if (_print_fd != 1) // XXX stdout
							close(_print_fd);
						fclose(fd);
					}
					_print_fd = 1;
				}
			}
			closedir(dir);
		}
		last_print_format = tmp;
		_print_fd = 1;
	}
}

int radare_prompt()
{
	char input[BUFLEN];
	const char *ptr;
	char* aux;
	char prompt[1024]; // XXX avoid 1024 limit
	int t, ret;

	config.interrupted = 0;

	/* run the visual command */
	if ( (ptr = config_get("cmd.visual")) && ptr[0])
	{
		aux = strdup ( ptr );
		radare_cmd( aux, 0 );
		free ( aux );
	}
	cons_flush();
	radare_prompt_command();

#if __UNIX__
	C	sprintf(prompt, "%s["OFF_FMT"]> "C_RESET,
			cons_palette[PAL_PROMPT], (offtx)config.seek+config.baddr); 
	else
	sprintf(prompt, "["OFF_FMT"]> ",
			(offtx)config.seek+config.baddr); 
#else
	sprintf(prompt, "["OFF_FMT"]> ",
			(offtx)config.seek+config.baddr); 
#endif

	memset(input, 0, BUFLEN);

	t = (int) config_get_i("cfg.verbose");
	if (!t) prompt[0]='\0';

#if HAVE_LIB_READLINE
	D {
		aux = readline(prompt);
		if (aux == NULL) {
			printf("\n");
			return 0;
		}

		strncpy(input, aux, sizeof(input));

		flag_space_pop();

		radare_cmd(input, 1);

		flag_space_push();
		if (aux && aux[0]) free(aux);
	} else {
		dl_disable=1;
#endif
		//D { printf(prompt); fflush(stdout); }
		dl_prompt = prompt;
		ret = cons_fgets(input, BUFLEN-1, 0, NULL);
		if (ret == -1)
			exit(0);
		radare_cmd(input, 1);
#if HAVE_LIB_READLINE
		dl_disable=0;
	}
#endif
	config_set_i("cfg.verbose", t);
	return 1;
}

void radare_set_block_size_i(size_t i)
{
	if (i<1) i = 1;
	if (((int)i)<0) i=1;

	config.block_size = i;
	free(config.block);
	config.block = (unsigned char *)malloc(config.block_size + 4);
	if (config.block == NULL) {
		if (i == DEFAULT_BLOCK_SIZE) {
			eprintf("Oops malloc error\n");
			return;
		} else {
			eprintf("Cannot allocate %d bytes\n", config.block_size+4);
			radare_set_block_size_i(DEFAULT_BLOCK_SIZE);
		}
	}
	radare_read(0);
}

void radare_set_block_size(char *arg)
{
	int i;
	size_t size = 0;

	for(i=0;arg[i]&&!iswhitespace(arg[i]);i++);
	for(;arg[i]&&iswhitespace(arg[i]);i++);

	if ( arg[i] != '\0' ) {
		size = get_math(arg+i);
		if (size<1) size = 1;
		if (arg[i]=='+') size += config.block_size;
		else
		if (arg[i]=='-') size = config.block_size - size;
		radare_set_block_size_i(size);
	}
	config_set_i("cfg.bsize", config.block_size);
	//D printf("bsize = %d\n", config.block_size);
}

void radare_resize(const char *arg)
{
	int fd_mode = O_RDONLY;
	u64 size  = get_math(arg);

	// XXX move this check into a only one function for all write-mode functions
	// or just define them as write-only. and activate/deactivate them from
	// the readline layer.

	if ( arg[0] == '\0' || arg[0] == '?') {
		D cons_printf("Usage: r[?] [#|-#]\n");
		D cons_printf("  positive value means resize\n");
		D cons_printf("  negative value is used to remove N bytes from the current seek\n");
		D cons_printf("size:  "OFF_FMTd"\n", config.size);
		D cons_printf("limit: "OFF_FMTd"\n", config.limit);
		return;
	}

	if (!config_get("file.write")) {
		eprintf("Only available for write mode. (-w)\n");
		return;
	}

	if (config.size == -1) {
		eprintf("Sorry, this file cannot be resized.\n");
		return;
	}

	if (arg[0]=='-') {
		u64 rest;
		size = -size; // be positive
		D eprintf("stripping %lld bytes\n", size);
		rest = config.size - (config.seek -size);
		if (rest > 0) {
			char *str = malloc(rest);
			io_lseek(config.fd, config.seek+size, SEEK_SET);
			io_read(config.fd, str, rest);
			io_lseek(config.fd, config.seek, SEEK_SET);
			io_write(config.fd, str, rest);
			free(str);
			io_lseek(config.fd, config.seek, SEEK_SET);
			config.size -= size;
			ftruncate(config.fd, (off_t)config.size);
		}
		return;
	}
	if (arg[1]=='x') sscanf(arg, OFF_FMTx, &size);

	printf("resize "OFF_FMTd" "OFF_FMTd"\n", config.size, size);
	if (size < config.size) {
		D printf("Truncating...\n");
		ftruncate(config.fd, (off_t)size);
		close(config.fd);
		if (config.limit > size)
			config.limit = size;
		config.size = size;
	}
	if (size > config.size ) {
		char zero = '\0';
		D printf("Expanding...\n");
		radare_seek(size, SEEK_SET);
		write(config.fd, &zero, 1);
		close(config.fd);
		config.limit = size;
		config.size  = size;
	}

	if (config_get("file.write"))
		fd_mode = O_RDWR;

	radare_open(1);
}

/* XXX rst flag is ignored :O , we should pass a file name only */
int radare_open(int rst)
{
	char *ptr;
	const char *cptr;
	char buf[4096];
	char buf2[255];
	struct config_t ocfg;
	int wm = (int)config_get("file.write");
	int fd_mode = wm?O_RDWR:O_RDONLY;
	u64 seek_orig = config.seek;

	if (config.file == NULL)
		return 0;

	memcpy(&ocfg, &config, sizeof(struct config_t));

	ptr = strrchr(config.file,'/');
	if (ptr == NULL) ptr = config.file; else ptr = ptr +1;
	strncpy(buf, ptr, 4000);
	ptr = strchr(buf, ' ');
	if (ptr) ptr[0] = '\0';
	//snprintf(buf2, 255, "%s.rdb", buf);
	//config_set("file.rdb", buf2);

	cptr = config_get("file.project");
	if (cptr)
	{
		ptr = strdup ( cptr );
		project_open(ptr);
		free ( ptr );
	}
	config.fd = io_open(config.file, fd_mode, 0);
	if (config.block_size==1)
		radare_set_block_size_i(DEFAULT_BLOCK_SIZE);

	if (config.fd == -1) {
		if (wm) {
			config.fd = io_open(config.file, fd_mode|O_CREAT, 0644);
			if (config.fd == -1) {
				config.fd = io_open(config.file, O_RDONLY, 0644);
				if (config.fd == -1 ) {
					D eprintf("error: cannot create file\n");
					memcpy(&config, &ocfg, sizeof(struct config_t));
					return 1;
				}
				config_set("file.write", "false");
			} else {
				D printf("new file\n");
			}
		} else {
			struct stat st;
			D { if (stat(config.file, &st)==0)
				eprintf("error: %s: Permission denied as %s\n",
					config.file, wm?"rw":"ro");
			else	eprintf("error: Cannot open '%s'. Use -w to create\n",
					config.file); }
			memcpy(&config, &ocfg, sizeof(struct config_t));
			return 1;
		}
	}

	/* handles all registered debugger prefixes */
	// ugly hack? :) a plugin should have a field specifying
	// if it's for debug or not
	if((strstr(config.file, "dbg://"))
	|| (strstr(config.file, "pid://"))
	|| (strstr(config.file, "winedbg://"))
	|| (strstr(config.file, "gxemul://"))
	|| (strstr(config.file, "gdb://"))
	|| (strstr(config.file, "gdbx://")))
		config.debug = 1;
	else	config.debug = 0;

	if (config.debug) {
		config_set("file.write", "true");
		rdb_init();
		config.file = strstr(config.file, "://") + 3;
	}

	config.size = io_lseek(config.fd, (u64)0, SEEK_END);
	io_lseek(config.fd, (u64)seek_orig, SEEK_SET);

	if (config.size == -1 || config.unksize) {
		config.size  = -1;
		config.limit =  0;
	} else {
		if (config.size == -1) {
			eprintf("warning: unknown file size."
				" Use 'l'imit to define boundaries.\n");
			config.size = -1;
			config.limit = 0;
		} else
			config.limit = config.size;
	}

	if (config.block_size == 0)
		radare_set_block_size_i(config.size);

	radare_sync();

	init_environment();

	radare_seek(config.seek, SEEK_SET);

	D printf("open %s%s %s\n",
		config.debug?"debugger ":"",
		wm?"rw":"ro",
		config.file);

	if (ocfg.fd != -1)
		io_close(ocfg.fd); // close old filedescriptor

	config.zoom.size   = config.size;
	config.zoom.from   = 0;
	config.zoom.piece  = config.size/config.block_size;

	return 0;
}

int radare_compare(unsigned char *f, unsigned char *d, int len)
{
	int i, eq = 0;

	for(i=0;i<len;i++)
		if (f[i]==d[i]) {
			eq++;
		} else {
			D cons_printf("0x%08llx (byte=%.2d)   %02x '%c'  ->  %02x '%c'\n",
				config.seek+i, i+1,
				f[i], (is_printable(f[i]))?f[i]:' ',
				d[i], (is_printable(d[i]))?d[i]:' ');
		}

	eprintf("Compare %d/%d equal bytes\n", eq, len);
	return len-eq;
}

int radare_go()
{
	u64 tmp;
	int t = (int)config_get("cfg.verbose");

	radare_controlc_end();

	if (config.file == NULL) {
		const char *project = config_get("file.project");
		if (project != NULL)
			config.file = estrdup(config.file, project_get_file(project));

		if (strnull(config.file)) {
			help_message_short();
			return 1;
		}
	}

	/* open file */
	tmp = config.block_size;
	if (radare_open(0))
		return 1;
	if (tmp)
		radare_set_block_size_i(tmp);

	/* hexdump mode (-x) */
	if (config.mode == MODE_HEXDUMP) {
		radare_cmd("x", 0);
		return 0;
	}

	radare_controlc();

	if (!config.noscript) {
		char path[1024];
		config.verbose = 0;
		snprintf(path, 1000, "%s/.radarerc", config_get("dir.home"));
		radare_interpret(path);
		config_set_i("cfg.verbose", t);
	}

	if (config_get_i("cfg.verbose") && config_get_i("cfg.fortunes"))
		radare_fortunes();

	/* load rabin stuff here */
	//rabin_load();
	if (config_get("file.id"))
		rabin_id();

	/* flag all syms and strings */
	//radare_cmd(".!rsc flag $FILE", 0);
	if (strnull(config_get("file.project"))) {
		if (config_get("file.flag"))
			rabin_flag();

		if (config_get("file.analyze")) {
			eprintf("Analyzing program...\n");
			radare_cmd(".af* @ entrypoint",0);
			radare_cmd(".af* @@ sym_",0);
			radare_cmd_raw("Ci", 0);
		}
	}

	switch(config.debug) { // old config.debug value
	case 1:
		t = config.verbose;
		config.verbose = 0;
		config.endian = !LIL_ENDIAN;
		radare_cmd(":.!regs*", 0);
		radare_cmd("s eip", 0);

		/* load everything */
		if (config_get("dbg.syms") && !config_get("file.flag"))
			radare_cmd("!syms", 0);
		if (config_get("dbg.maps")) {
			radare_cmd("!maps", 0);
			radare_cmd(".!!rsc maps ${DPID}", 0);
		}

		if (config_get("dbg.sections") && !config_get("file.flag"))
			//radare_cmd(":.!rsc flag-sections $FILE", 0);
			radare_cmd(".!!rabin -rS $FILE",0);

		if (config_get("dbg.strings") && !config_get("file.flag")) {
			//eprintf("Loading strings...press ^C when tired\n");
			//radare_cmd(".!rsc strings-flag $FILE", 0);
			radare_cmd(".!!rabin -rz $FILE",0);
		}

		radare_set_block_size_i(100); // 48 bytes only by default in debugger
		config_set("file.write", "true"); /* write mode enabled for the debugger */

		config_set("cfg.verbose", "true"); /* write mode enabled for the debugger */
		//config.verbose = 1; // ?
		break;
	case 2:
		radare_seek(config.seek, SEEK_SET);
		radare_read(0);
		data_print(config.seek, "", config.block, config.block_size, FMT_HEXB);
		exit(0);
	}

	if (io_isdbg(config.fd)) {
		radare_cmd(":.!regs*", 0);
		radare_cmd(".!info*", 0);
		radare_cmd(":.!maps*", 0);
		radare_cmd("s eip", 0);
	}

	config_set_i("cfg.verbose", t);

	if (config.script)
		radare_interpret(config.script);

	config_set_i("cfg.verbose", t);

	radare_controlc_end();

	do {
		do {
			cons_flush();
			if (config.debug)
				radare_cmd(".!regs*", 0);
			update_environment();
			radare_sync();
		} while( radare_prompt() );
	} while ( io_close(config.fd) );

	return 0;
}

// TODO: move to cons.c
//static int pipe_fd = -1;
int pipe_stdout_to_tmp_file(char *tmpfile, const char *cmd)
{
	/* WORKS BUT IT IS UGLY */
	int fd = make_tmp_file(tmpfile);
	int std;
	cons_reset();
	//cons_flush();
	if (fd == -1) {
		eprintf("pipe: Cannot open '%s' for writing\n", tmpfile);
		tmpoff = config.seek;
		return 0;
	}
	std = dup(1); // store stdout
	dup2(fd, 1);

	if (cmd[0]) {
		char *ptr = strdup(cmd);
		radare_cmd_raw(ptr, 0);
		free(ptr);
	}

	cons_reset();
	//cons_flush();
	fflush(stdout);
	fflush(stderr);
	close(fd);
	if (std!=0) {
		dup2(std, 1);
		close(std);
	}

	return 1;
}

char *pipe_command_to_string(char *cmd)
{
	char *buf = NULL;
	char tmpfile[1024];

	if (pipe_stdout_to_tmp_file(tmpfile, cmd)) {
		buf = slurp(tmpfile);
		unlink(tmpfile);
	}

	return buf;
}
