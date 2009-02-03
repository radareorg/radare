#include <stdio.h>
//#include <sys/linux.h>
#include <sys/signal.h>
#include <sys/ptrace.h>
#include <asm/user.h>

int main()
{
	int ret, pid = fork();
	if (pid ==0) {
		ptrace(PTRACE_TRACEME, 0 ,0,0);
perror("ptrace");
		execl("/bin/ls", "ls", NULL);
	} else {
	int st;
		unsigned char buf[1024];
		struct user_regs_struct regs;
		wait(&st);
printf("ST = %d\n", st);
ptrace(PTRACE_ATTACH, pid, 0 , 0);
perror("attach");
		kill(pid, SIGSTOP);
		memset(buf, '\0', 4);
		memset(&regs, '\0', sizeof(regs));

		ret = ptrace(PTRACE_GETREGS, pid, 0, &regs);
		if (ret == -1) {
			printf("ret = %d\n", ret);
			perror("ptrace");
		}
printf("EIP = 0x%08x\n", regs.eip);
		ret = ptrace(PTRACE_CONT, pid, 0,0);
		printf("%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
	}
}
