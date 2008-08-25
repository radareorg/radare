/*
 * Copyright (C) 2007,2008
 *       pancake <youterm.com>
 *
 * libps2fd is part of the radare project
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
#include "../mem.h"
#include "../arch/i386.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#if __UNIX__
#include <sys/wait.h>
#include <sys/syscall.h>
//#include <sys/ptrace.h>
#endif

#if __linux__
#include <sys/prctl.h>
#endif

#include "../debug.h"

u64 debug_fd_seek(int pid, int fd, u64 addr, int whence)
{
	u64 ret = 0;
#if __UNIX__
	ret = arch_syscall(pid, SYS_lseek, fd, addr, whence);
#endif
	//printf("seek(%d,%d,%d)->%06x\n", fd, (int)addr, whence, ret);
	return ret;
}

int debug_fd_list(int pid)
{
	char path[1024];
	char buf[1024];
	char buf2[1024];
	struct stat st;
	int r,w, t;
	struct dirent *de;
	DIR *dd;

	sprintf(path, "/proc/%d/fd/", pid);
	dd = opendir(path);
	if (dd==NULL) {
		printf("Cannot open /proc\n");
		return -1;
	}
	while((de = (struct dirent *)readdir(dd))) {
		if (de->d_name[0]=='.')
			continue;
		sprintf(path, "/proc/%d/fd/", pid);
		strncat(path, de->d_name, de->d_reclen);
		memset(buf, 0, 1024);
		readlink(path, buf, 1023);
		if (stat(path, &st) != -1) {
			t = st.st_mode & S_IFIFO  ? 'P':
			    st.st_mode & S_IFSOCK ? 'S':
			    st.st_mode & S_IFCHR  ? 'C':'-';
		} else t = '-';
		if (lstat(path, &st) != -1) {
			r = st.st_mode & S_IRUSR  ? 'r':'-';
			w = st.st_mode & S_IWUSR  ? 'w':'-';
		} else r = w = '-';
		if (readlink(buf, buf2, 1023) != -1) /* never happens ? */
		printf("%2s 0x%08x %c%c%c %s -> %s \n", de->d_name, 
			(unsigned int)debug_fd_seek(pid,atoi(de->d_name),0,1),r,w,t,buf,buf2);
		else
		printf("%2s 0x%08x %c%c%c %s\n", de->d_name, 
			(unsigned int)debug_fd_seek(pid,atoi(de->d_name),0,1),r,w,t,buf);
	}
	closedir(dd);

	return 0;
}

int debug_fd_dup2(int pid, int oldfd, int newfd)
{
#if __UNIX__
#ifdef SYS_dup2
	return arch_syscall(pid, SYS_dup2, oldfd, newfd);
#else
#warning No dup2 here? Solaris?
#endif
#else
#warning Implement fd debugger stuff for w32
	return -1;
#endif
}

int debug_fd_open(int pid, char *file, int mode)
{
#if __UNIX__
	return arch_syscall(pid, SYS_open, file, mode);
#endif
	return -1;
#if 0
#if __i386__
        regs_t   reg, reg_saved;
	u64	file_addr;
        int     status;
	char	bak[128];
        void*   ret = (void *)-1;

	/* save old registers */
        debug_getregs(pid, &reg_saved);
	memcpy(&reg, &reg_saved, sizeof(reg));

	/* read stack values */
        debug_read_at(pid, bak, 128, R_EIP(reg));

	file[120]='\0'; // truncate
	file_addr = R_EIP(reg)+4;
	debug_write_at(pid, file, strlen(file)+4, file_addr);

	/* close call */
        R_EAX(reg) = SYS_open;
        R_EBX(reg) = file_addr;
        R_ECX(reg) = mode;
        R_EDX(reg) = 0x0;

        /* write syscall interrupt code */
        R_EIP(reg) = R_ESP(reg) - 4;

	/* write SYSCALL OPS */
	debug_write_at(pid, (long *)SYSCALL_OPS, 4, R_EIP(reg));

        /* set new registers value */
        debug_setregs(pid, &reg);

        /* continue */
        debug_contp(pid);

        /* wait to stop process */
        waitpid(ps.tid, &status, 0);

	if(WIFSTOPPED(status)) {
        	/* get new registers value */
        	debug_getregs(ps.tid, &reg);

        	/* read allocated address */
        	ret = (void *)R_EAX(reg);
	}

        /* restore registers */
        debug_setregs(ps.tid, &reg_saved);

        /* restore memory */
	debug_write_at(ps.tid, (long *)bak, 128, R_ESP(reg_saved) - 4);
	return ret;
#else
	fprintf(stderr, "TODO: fd_open\n");
	return -1;
#endif
#endif
}

int debug_fd_close(int pid, int fd)
{
#if __UNIX__
	//return arch_syscall(pid, SYS_close, 'i', fd);
	return arch_syscall(pid, SYS_close, fd);
#endif
	return 0;
}
