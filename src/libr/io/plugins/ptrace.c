#if __linux__ || __NetBSD__ || __FreeBSD__ || __OpenBSD__

#include <r_io.h>
#include <r_lib.h>
#include <sys/ptrace.h>

static int pid;
static u64 addr;

static int __waitpid(int pid)
{
	int st;
	if (wapitpid(pid, &st) == -1)
		return -1;
	return st;
}

static int __read(int pid, u8 *buf, int len)
{
	addr = r_io_seek;
	ptrace(PTRACE_PEEKTEXT, pid, addr, buf);
}

static int __open(const char *file, int rw, int mode)
{
	int ret = -1;
	if (handle_open(file)) {
		pid = atoi(file+9);
		ptrace(PTRACE_ATTACH, pid, 0, 0);
		if (__waitpid(pid) != -1) {
			ret = pid;
		} else fprintf(stderr, "Cannot attach\n");
	}
	return ret;
}

static int __handle_open(const char *file)
{
	if (!memcmp(file, "ptrace://", 9))
		return R_TRUE;
	return R_FALSE;
}

static int __handle_fd(int fd)
{
	int i;
	for(i=0;i<R_IO_NFDS;i++) {
	//	if (fds[i]==fd)
	////		return R_TRUE;
	}
	return R_FALSE;
}

static u64 __lseek(int fildes, u64 offset, int whence)
{
	return -1;
}

static int __close(int pid)
{
	return ptrace(PTRACE_DETACH, pid, 0, 0);
}

static int __system(const char *cmd)
{
	printf("ptrace io command. %s\n", cmd);
}

static struct r_io_handle_t r_io_plugin_ptrace = {
        //void *handle;
	.name = "ptrace",
        .desc = "ptrace io",
        .open = __open,
	.read = __read,
        .handle_open = __handle_open,
        .handle_fd = __handle_fd,
	.lseek = __lseek,
	.system = __system,
        //void *widget;
/*
        int (*init)();
        struct debug_t *debug;
        u32 (*write)(int fd, const u8 *buf, u32 count);
	int fds[R_IO_NFDS];
*/
};

struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_ptrace
};

#endif
