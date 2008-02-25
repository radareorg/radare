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
#include <regex.h>
#include <termios.h>
#include <sys/wait.h>
#include <netdb.h>
#endif

#include <stdio.h>
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

u64 tmpoff = -1;
int std = 0;

void radare_init()
{
	config_init();
	flags_setenv();
	plugin_init();
}

static int radare_interrupt(int sig)
{
	config.interrupted = 1;
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

int radare_close()
{
	return io_close(config.fd);
}

void radare_sync()
{
	int limit = DEFAULT_BLOCK_SIZE;

	if (config.debug||config.unksize)
		return;

	/* enlarge your integer overflows */
	if (config.seek>0xfffffffffffffff)
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
	int ran;
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
		printf("Message of the day:\n  %s\n", ptr);
		free(str);
	}
}

int radare_cmd_raw(char *tmp, int log)
{
	FILE *fd;
	int i,f,fdi = 0;
	char *eof;
	char *eof2;
	char *piped;
	char file[1024], buf[1024];
	char *input, *oinput;
	char *str, *st;
	char *next = NULL;

	if (strnull(tmp))
		return 0;

	tmp = strclean(tmp);

	if (strchr(tmp,'\n')) {
		eprintf("Multiline command not yet supported\n");
		return 0;
	}

	input = oinput = strdup(tmp);

	if (input[0] == ':') {
		config.verbose = ((int)config_get("cfg.verbose"))^1;
		config_set("cfg.verbose", (config.verbose)?"true":"false");
		input = input+1;
	}

 	eof = input+strlen(input)-1;

	if (input[0]!='%') {
		next = strstr(input, "&&");
		if (next) next[0]='\0';
	}

	/* interpret stdout of a process executed */
	if (input[0]=='.')
	{
		radare_controlc();
		switch(input[1]) {
		case '%': {
			char *cmd = getenv(input+2);
			if (cmd)
				radare_cmd(cmd, 0);
			free(oinput);
			return 1;
			}
		case '?':
			cons_printf(
			"Usage [.][command| file]\n"
			"- Interpret radare commands from file or command\n"
			"  > .!regs*\n"
			"  > . /tmp/flags-saved\n");
			break;
		case ' ':
			fd = fopen(input+2,"r");
			if (fd) {
				while(!feof(fd)) {
					buf[0]='\0';
					fgets(buf, 1000, fd);
					if (buf[0]!='\0')
						radare_cmd(buf, 0);
				}
				fclose(fd);
			} else
				cons_printf("oops (%s)\n", input+2);
			break;
		default:
			/* hack to parse .!regs* on w32 */
			/* maybe everything but debug commands must be in this way */
			if (strstr(input,"regs")) {
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
				return;
			}
			pipe_stdout_to_tmp_file(file, input+1);
			f = open(file, O_RDONLY);
			if (f == -1) {
				eprintf("radare_cmd_raw: Cannot open.\n");
				config_set_i("cfg.verbose", 1);
				free(oinput);
				return 0;
			}
			for(;!config.interrupted;) {
				int v = config_get("cfg.verbose");
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
		if (piped) {
			char tmp[1024];
			char cmd[1024];
			piped[0]='\0';
			pipe_stdout_to_tmp_file(tmp, input);
			snprintf(cmd, 1023 ,"cat '%s' | %s", tmp, piped+1);
			io_system(cmd);
			unlink(tmp);
			free(oinput);
			return 1;
		}
		if (input[0]!='%' && input[0]!='!' && input[0]!='_' && input[0]!=';' && input[0]!='?') {
			/* inline pipe */
			piped = strchr(input, '`');
			if (piped) {
				int len;
				int dif = input - oinput;
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
				free(oinput);
				input = oinput = malloc(len);
				sprintf(oinput, "wx %s", filebuf);
			}

			/* temporally offset */
			eof2 = strchr(input, '@');
			if (eof2 && input && input[0]!='e') {
				char *ptr = eof2+1;
				eof2[0] = '\0';
				tmpoff = config.seek;
				for(;*ptr==' ';ptr=ptr+1);
				if (*ptr=='+'||*ptr=='-')
					config.seek = config.seek + get_math(ptr);
				else	config.seek = get_math(ptr);
				radare_read(0);
			}

			if (input[0] && input[0]!='>' && input[0]!='/') { // first '>' is '!'
				char *pos = strchr(input+1, '>');
				char *file = pos + 1;
				char *end;
				if (pos != NULL) {
					for(pos[0]='\0';iswhitespace(file[0]); file = file + 1);
					if (*file == '\0') {
						eprintf("No target file\n");
					} else {
						for(end = file+strlen(file);
							end[0]&&!iswhitespace(end[0]);
							end = end-1); end[0]='\0';
						if (pos[1] == '>')
							fd = io_open(file, O_APPEND|O_WRONLY, 0644);
						else	fd = io_open(file, O_TRUNC|O_WRONLY|O_CREAT, 0644);
						if (fd == -1) {
							eprintf("Cannot open '%s' for writing\n", file);
							config_set("cfg.write", "false");
							tmpoff = config.seek;
							free(oinput);
							return 1;
						}
						std = dup(1); // store stdout
						dup2(fd, 1);
					}
				}
			}
		}

		// XXX fuckmenot
		//for(;eof!=input && eof>1 && eof[0] && iswhitespace(eof[0]); eof=eof-1) eof[0]='\0';

		if (input[strlen(input)]=='\n')
			input[strlen(input)] = '\0';
		for(eof=input;eof[0];eof=eof+1)
			if (eof[0]=='\n') eof[0]=' ';
		commands_parse(input);

		if (fdi!=0) {
			fflush(stdout);
			close(fd);
			dup2(std, 1);
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
	return 0; /* error */
}

char *radare_cmd_str(const char *cmd)
{
	char *buf;
	//cons_flush();
	cons_reset();
	radare_cmd(cmd, 0);
//printf("RUN(%s)\n", cmd);
	buf = cons_get_buffer();
//printf("RET(%s)\n", buf);
	if (buf)
		buf = strdup(buf);
	//buf = cons_get_buffer();
	//cons_flush();
	cons_reset();
	return buf;
}

int radare_cmd(char *tmp, int log)
{
	const char *ptr;
	int repeat;
	int p,i;
	char buf[128];

	/* silently skip lines begginging with 0 */
	if(tmp==NULL || (log&&tmp==NULL) || (tmp&&tmp[0]=='0'))
		return 0;

	// TODO : move to a dbg specific func outside here
	if (config.debug && tmp && tmp[0]=='\0') {
		radare_read(0);
		ptr = config_get("cmd.visual");
		if (!strnull(ptr)) {
			char *ptrcmd = strdup(ptr);
			radare_cmd_raw(ptrcmd, 0);
			free(ptrcmd);
		}
		config_set("cfg.verbose", "false");
		p = last_print_format;

		terminal_get_real_columns();

		/* NOT REQUIRED update flag registers NOT REQUIRED */
		//radare_cmd(":.!regs*", 0);

		if (config_get("dbg.stack")) {
			C cons_printf(C_RED"Stack:\n"C_RESET);
			else cons_printf("Stack:\n");
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

		//config.verbose = 1; //t;
		config_set("cfg.verbose", "true");
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

		//radare_cmd("s eip", 0);
		{
			u64 pc = flag_get_addr("eip");
			if (pc<config.seek || pc > config.seek+config.block_size)
				radare_cmd("s eip",0);
		}		
		config.height-=10;
		config_set("cfg.verbose", "true");
		config.verbose=1;
		/* TODO: chose pd or pD by eval */
		if (config.visual)
			radare_cmd("pD", 0);
		else	radare_cmd("pD 50", 0);
		//radare_cmd("pd 100", 0);

		config_set("cfg.verbose", "1");
		last_print_format = p;
		config.height+=14;
		cons_flush();
		return 0;
	}

	/* history stuff */
	if (tmp[0]=='!') {
		p = atoi(tmp+1);
		if (tmp[0]=='0'||p>0)
			return radare_cmd(hist_get_i(p), 0);
	}
	hist_add(tmp, log);
	dl_hist_add(tmp);
	if (config.skip) return 0;

	if (tmp[0] == ':') {
		config.verbose = ((int)config_get("cfg.verbose"))^1;
		config_set("cfg.verbose", (config.verbose)?"true":"false");
		tmp = tmp+1;
	}

	/* repeat stuff */
	if (tmp)
		repeat = atoi(tmp);
	if (repeat<1)
		repeat = 1;
	for(;tmp&&(tmp[0]>='0'&&tmp[0]<='9');)
		tmp=tmp+1;

	for(i=0;i<repeat;i++)
		radare_cmd_raw(tmp, log);

	return 0;
}

int radare_interpret(char *file)
{
	int len;
	char buf[1024];
	FILE *fd;
	
	if (file==NULL || file[0]=='\0')
		return 0;
	
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
		//cons_flush();
		hist_add(buf, 0);
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

void radare_move(char *arg)
{
	unsigned char *buf;
	char *str = strchr(arg, ' ');
	u64 len =  0;
	u64 pos = -1;
	u64 src = config.seek;

	if (!config_get("cfg.write")) {
		eprintf("You are not in read-write mode.\n");
		return;
	}

	if (str) {
		str[0]='\0';
		len = get_math(arg);
		pos = get_math(str+1);
		str[0]=' ';
	}
	if ( (str == NULL) || (pos == -1) || (len == 0) ) {
		printf("Usage: move [len] [dst-addr]\n");
		return;
	}

	buf = (unsigned char *)malloc( len );
	radare_seek(config.seek, SEEK_SET);
	read(config.fd, buf, len);
	radare_seek(pos, SEEK_SET);
	write(config.fd, buf, len);
	free(buf);

	config.seek = src;
	radare_read(0);
	radare_open(1);
}

void radare_prompt_command()
{
	DIR *dir;
	char path[1024];
	int i;
	FILE *fd;
	char file[1024];
	char *ptr;
	int tmp; /* preserve print format */
	struct dirent *de;

	/* user defined command */
	ptr = config_get("cmd.prompt");
	if (ptr&&ptr[0]) {
		int tmp = last_print_format;
		radare_cmd_raw(ptr, 0);
		last_print_format = tmp;
	}

	if (config.debug)
		radare_cmd(".!regs*", 0);

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
			while((de = readdir(dir))) {
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

extern char *dl_prompt;
extern int dl_disable;
int radare_prompt()
{
	char input[BUFLEN];
	const char *ptr;
	char prompt[1024]; // XXX avoid 1024 limit
	int t, ret;

	config.interrupted = 0;

	/* run the visual command */
	if ( (ptr = config_get("cmd.visual")) && ptr[0])
		radare_cmd( ptr, 0 );
	cons_flush();
	radare_prompt_command();

#if __UNIX__
	C	sprintf(prompt, C_YELLOW"["OFF_FMT"]> "C_RESET,
			(offtx)config.seek+config.baddr); 
	else
	sprintf(prompt, "["OFF_FMT"]> ",
			(offtx)config.seek+config.baddr); 
#else
	sprintf(prompt, "["OFF_FMT"]> ",
			(offtx)config.seek+config.baddr); 
#endif

	memset(input, 0, BUFLEN);

	t = (int) config_get("cfg.verbose");

#if HAVE_LIB_READLINE
	D {
		ptr = readline(prompt);
		if (ptr == NULL)
			return 0;

		strncpy(input, ptr, sizeof(input));

		if (config.width>0) { // fixed width
			fixed_width = 0;
			config.width = terminal_get_columns();
			//rl_get_screen_size(NULL, &config.width);
			radare_cmd(input, 1);
		} else { // dinamic width
			fixed_width = 1;
			config.width=-config.width;	
			radare_cmd(input, 1);
			config.width=-config.width;	
		}
		if (ptr && ptr[0]) free(ptr);
	} else {
		dl_disable=1;
#endif
		//D { printf(prompt); fflush(stdout); }
		dl_prompt = &prompt;
		ret = cons_fgets(input, BUFLEN-1);
		if (ret == -1)
			exit(0);
		radare_cmd(input, 1);
#if HAVE_LIB_READLINE
		dl_disable=0;
	}
#endif
	
	if (input[0]!='%')  {
		config_set("cfg.verbose", t?"true":"false");
		config.verbose = t;
	}
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
		eprintf("Cannot allocate %d bytes\n", config.block_size+4);
		radare_set_block_size_i(DEFAULT_BLOCK_SIZE);
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
		size = get_offset(arg+i);
		if (size<1) size = 1;
		radare_set_block_size_i(size);
	}
	D printf("bsize = %d\n", config.block_size);
}

int radare_open(int rst)
{
	char *ptr;
	char buf[4096];
	char buf2[255];
	struct config_t ocfg;
	int wm = config_get("cfg.write");
	int fd_mode = wm?O_RDWR:O_RDONLY;
	u64 seek_orig = config.seek;

	if (config.file == NULL)
		return;

	memcpy(&ocfg, &config, sizeof(struct config_t));
	prepare_environment("");

	ptr = strrchr(config.file,'/');
	if (ptr == NULL) ptr = config.file; else ptr = ptr +1;
	strncpy(buf, ptr, 4000);
	ptr = strchr(buf, ' ');
	if (ptr) ptr[0] = '\0';
	snprintf(buf2, 255, "%s.rdb", buf);
	config_set("file.rdb", buf2);

	D if (wm)
		eprintf("warning: Opening file in read-write mode\n");

	ptr = config_get("file.project");
	if (ptr)
		project_open(ptr);

	config.fd = io_open(config.file, fd_mode, 0);

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
				config_set("cfg.write", "false");
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
	|| (strstr(config.file, "gdb://")))
		config.debug = 1;
	else	config.debug = 0;

	if (config.debug) {
		config_set("cfg.write", "true");
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
	int i,eq = 0;

	for(i=0;i<len;i++)
		if (f[i]==d[i])
			eq++;

	eprintf("Compare %d/%d equal bytes\n", eq, len);
	return len-eq;
}

int radare_go()
{
	u64 tmp;
	int t = (int)config_get("cfg.verbose");

	radare_controlc_end();

	if (config.file == NULL) {
		eprintf("radare [-fhnuLvVwc] [-s #] [-b #] [-S #] [-i f] [-P f] [-e k=v] [file]\n");
		return 1;
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

#if HAVE_LIB_READLINE
	D rad_readline_init();
#endif
	radare_controlc();

	if (!config.noscript) {
		char path[1024];
		int t = config_get("cfg.verbose");
		config.verbose = 0;
		snprintf(path, 1000, "%s/.radarerc", config_get("dir.home"));
		radare_interpret(path);
		config_set("cfg.verbose", t?"true":"false");
	}

	if (config_get("cfg.verbose") && config_get("cfg.fortunes"))
		radare_fortunes();

	/* load rabin stuff here */
	if (config_get("file.identify"))
		rabin_load();

	/* flag all syms and strings */
	if (config_get("file.flag"))
		radare_cmd(".!rsc flag $FILE", 0);

	switch(config.debug) { // old config.debug value
	case 1:
		t = config.verbose;
		config.verbose = 0;
		config.endian = !LIL_ENDIAN;
		radare_cmd(":.!regs*", 0);
		radare_cmd("s eip", 0);
		/* load everything */
		if (config_get("dbg.syms"))
			radare_cmd("!syms", 0);
		if (config_get("dbg.maps"))
			radare_cmd("!maps", 0);
		if (config_get("dbg.strings")) {
			eprintf("Loading strings...press ^C when tired\n");
			radare_cmd(".!rsc strings-flag $FILE", 0);
		}
		radare_set_block_size_i(100); // 48 bytes only by default in debugger
		config_set("cfg.write", "true"); /* write mode enabled for the debugger */
		config_set("cfg.verbose", "true"); /* write mode enabled for the debugger */
		config.verbose = 1; // ?
		break;
	case 2:
		radare_seek(config.seek, SEEK_SET);
		radare_read(0);
		data_print(config.seek, "", config.block, config.block_size, FMT_HEXB, 0);
		exit(0);
	}

	if (io_isdbg(config.fd)) {
		radare_cmd(":.!regs*", 0);
		radare_cmd(".!info*", 0);
		radare_cmd(":.!maps*", 0);
		radare_cmd("s eip", 0);
	}

	if (config.script)
		radare_interpret(config.script);

	config_set("cfg.verbose", t?"true":"false");

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

int pipe_stdout_to_tmp_file(char *tmp, char *cmd)
{
	int fd = make_tmp_file(tmp);
	cons_flush();
	if (fd == -1) {
		eprintf("pipe: Cannot open '%s' for writing\n", tmp);
		tmpoff = config.seek;
		return 0;
	}
	std = dup(1); // store stdout
	dup2(fd, 1);

	if (cmd[0])
		radare_cmd(cmd, 0);

	cons_flush();
	fflush(stdout);
	fflush(stderr);
	close(fd);
	dup2(std, 1);
	close(std);

	return 1;
}

#define BLOCK 1024

// innecessary
char *pipe_command_to_string(char *cmd)
{
	char *buf = NULL;
	char msg[1024];
	unsigned int size = BLOCK;
	int fd;

	buf = (char *)malloc(size);
	memset(buf, '\0', size);
	fd = make_tmp_file(msg);
	cons_set_fd(fd);

	radare_cmd(cmd, 0);
	
	close(fd);

	fd = open(msg, O_RDONLY);
	size = (unsigned int)lseek(fd,0,SEEK_END);
	buf = (char *)malloc(size+1);
	memset(buf, '\0', size+1);
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, size);
	
	close(fd);

	unlink(msg);
	return buf;
}

#if 0
	int child;
	int io[2];
	int i, ret;
/* on memory */
// XXX DISABLED ATM XXX
//printf("Execute(%s)\n", cmd);
	
	int j;
	char tmp[1024];
	char *argv[10];
	argv[0]=argv[1]=argv[2]=argv[3]='\0';
	strcpy(tmp, cmd);
	for(j=0,i=strlen(tmp)-1;i>0;i--) {
		if (tmp[i]==' ') {
			tmp[i]='\0';
			argv[j++] = tmp+i+1;
			printf("arg: %s\n", tmp+i+1);
		}
	}
	argv[j++] = tmp;

	buf = (char *)malloc(size);
	pipe(io);
//printf("CMD: %s\n", cmd);
//printf("ARG: %s, %s, %s\n", argv[0], argv[1], argv[2]);
	child = fork();
	if (child) {
		// pare
		fcntl(io[0], F_SETFL, O_NONBLOCK);
		for(i=0;;) {
			if (i==size) {
				size += BLOCK;
				buf = realloc(buf, size);
			}
			ret = read(io[0], buf+i, 1);
			if (ret == 1)
				i++;
			if (ret<0)
				if (-1==waitpid(child, NULL, WNOHANG))
					break;
		}
		buf[i]='\0';
		close(io[0]);
		close(io[1]);
	} else {
		// fill
		close(0); // no stdin
		dup2(io[1], 1); // stdout to pipe
		radare_cmd(cmd, 0);
	//	io_system(cmd);
	//	execv(argv[0], argv);
	//	perror("execv");
		// execl failed
		close(io[0]);
		close(io[1]);
		exit(1);
	}

	if (buf[0]=='\0') {
		free(buf);
		buf = NULL;
	}
	return buf;
}
#endif

#if 0
char *pipe_command_to_string(char *cmd)
{
	int pid, ret;
	int limit = 1024;
	int strlen = 0;
	char *str;
	char buf[2];
	int pee[2];

	if (!cmd[0])
		return NULL;
	pipe(pee);

	str = (char *)malloc(limit);
	if (str == NULL)
		return NULL;

	if (!(pid = fork())) {
		close(1);
		dup2(pee[1], 1);
		radare_cmd(cmd, 0);
		fflush(stdout);
		fflush(stderr);
		exit(0);
	}

	while((ret = read(pee[0], buf+strlen,1))==1) {
		if (strlen>limit) {
			str = realloc(str, strlen+1024);
			limit += 1024;
		}
		str[strlen++] = buf[0];
		str[strlen] = '\0';
	}

	close(pee[0]);
	close(pee[1]);

	return str;
}
#endif
