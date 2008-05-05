/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * radare is part of the radare project
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


#define USE_PTRACE 0
#define __addr_t_defined

#include "../mem.h"
#include "../debug.h"
#include "../thread.h"
//#include "utils.h"
#include <mach/exception_types.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_traps.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/thread_act.h>
#include <mach/thread_info.h>
#include <mach/vm_map.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

static thread_array_t inferior_threads = NULL;

// TODO: to be removed
int debug_ktrace()
{
}

inline int debug_detach()
{
	ptrace(PT_DETACH, ps.tid, 0, 0);
	//inferiorinferior_in_ptrace = 0;
	return 0;
}

static task_t inferior_task = 0;
static unsigned int inferior_thread_count = 0;
static exception_type_t exception_type;
static exception_data_t exception_data;
static mach_port_t exception_port; 

void inferior_abort_handler(int pid)
{
	fprintf(stderr, "Inferior received signal SIGABRT. Executing BKPT.\n");
}

#define MACH_ERROR_STRING(ret) \
(mach_error_string (ret) ? mach_error_string (ret) : "[UNKNOWN]")

int debug_attach(int pid)
{
	kern_return_t err;

	signal(SIGCHLD, pid);

	if ((err = task_for_pid(mach_task_self(), (int)pid, &inferior_task) !=
				KERN_SUCCESS)) {
		fprintf(stderr, "Failed to get task for pid %d: %d.\n", (int)pid, (int)err);
		fprintf(stderr, "Reason: %s\n", MACH_ERROR_STRING(err));
		fprintf(stderr, "You probably need to add user to procmod group.\n"
				" Or chmod g+s radare && chown root:procmod radare\n");
		return -1;
	}

	if (task_threads(inferior_task, &inferior_threads, &inferior_thread_count)
			!= KERN_SUCCESS) {
		fprintf(stderr, "Failed to get list of task's threads.\n");
		return -1;
	}

	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				&exception_port) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to create exception port.\n");
		return -1;
	}
	if (mach_port_insert_right(mach_task_self(), exception_port,
				exception_port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to acquire insertion rights on the port.\n");
		return -1;
	}
	if (task_set_exception_ports(inferior_task, EXC_MASK_ALL, exception_port,
				EXCEPTION_DEFAULT, THREAD_STATE_NONE) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to set the inferior's exception ports.\n");
		return -1;
	}
	return 0;
}

int debug_contp(int tid)
{
	thread_resume(inferior_threads[0]);
	//wait_for_events();
	return 0;
}

inline int debug_os_steps()
{
	return 0;
}

/* TODO: not work */
int debug_contfork(int tid) { eprintf("not work yet\n"); return -1; }
int debug_contscp() { eprintf("not work yet\n"); return -1; }
int debug_status() { eprintf("not work yet\n"); return -1; }

int debug_pstree(char *input)
{ 
	int tid = atoi(input);
	if (tid != 0) {
		ps.tid = tid;
		eprintf("Current selected thread id (pid): %d\n", ps.tid);
		// XXX check if exists or so
	}

	printf(" pid : %d\n", ps.pid);
	printf(" tid : %d\n", ps.tid);

	eprintf("more work to do\n"); return -1; 
}

static pid_t start_inferior(int argc, char **argv)
{
	char **child_args;
	int status;
	pid_t kid;

	fprintf(stderr, "Starting process...\n");

	if ((kid = fork())) {
		wait(&status);
		if (WIFSTOPPED(status)) {
			fprintf(stderr, "Process with PID %d started...\n", (int)kid);
			return kid;
		}
		return -1;
	}

	child_args = (char **)malloc(sizeof(char *) * (argc + 1));
	memcpy(child_args, argv, sizeof(char *) * argc);
	child_args[argc] = NULL;

	ptrace(PT_TRACE_ME, 0, 0, 0);
	/* why altering these signals? */
	signal(SIGTRAP, SIG_IGN);
	signal(SIGABRT, inferior_abort_handler);

	execvp(argv[0], child_args);

	fprintf(stderr, "Failed to start inferior.\n");
	return 0;
}

int debug_set_bp_mem(int pid, u64 addr, int len)
{
	// TODO: not used
}

int debug_fork_and_attach()
{
	char **argv = { "/bin/ls", 0 };
	int err;
	pid_t pid;
	/* TODO */
	pid = start_inferior(1, &argv);

	err = debug_attach(pid);
	if (err == -1) {
		kill(pid, SIGKILL);
exit(1);
		return -1;
	}

	ps.pid = ps.tid = pid;
	return pid;
}

extern int errno;

#define debug_read_raw(x,y) ptrace(PTRACE_PEEKTEXT, x, y, 0)
static int ReadMem(int pid,  addr_t addr, size_t sz, void *buff)
{
	unsigned long words = sz / sizeof(long) ;
	unsigned long last = sz % sizeof(long) ;
	long x, lr ;
	int ret ;

	if (addr==-1)
		return 0;

	for(x=0;x<words;x++) {
		((long *)buff)[x] = debug_read_raw(pid, (void *)(&((long*)(long )addr)[x]));

		if (((long *)buff)[x] == -1 && errno)
			goto err;
	}

	if (last) {
		//lr = ptrace(PTRACE_PEEKTEXT,pid,&((long *)addr)[x],0) ;
		lr = debug_read_raw(pid, &((long*)(long)addr)[x]);

		if (lr == -1 && errno)
			goto err;

		memcpy(&((long *)buff)[x],&lr,last) ;
	}

	return sz;
err:
	ret = --x * sizeof(long);

	return ret ;
}

int debug_read_at(pid_t tid, void *buff, int len, u64 addr)
{
#if USE_PTRACE
	int ret = ReadMem(tid, (addr_t)addr, (size_t)len, (void *)buff);
	if (ret>0)return ret;
	return 0;
#else
	unsigned int size= 0;
	int err = vm_read_overwrite(inferior_task, (unsigned int)addr, 4, (pointer_t)buff, &size);
	if (err == -1)
		return -1;
	return size;
#endif
}

int debug_write_at(pid_t tid, void *buff, int len, u64 addr)
{
	kern_return_t err = vm_write(tid, addr, (pointer_t)buff, (mach_msg_type_number_t)len);
	if (err == KERN_SUCCESS)
		return len;

	return -1;
}


int debug_getregs(pid_t tid, regs_t *regs)
{
	printf("TODO: GETREGS\n");
	/* TODO */
	return 0;
}

	const char *
unparse_exception_type (unsigned int i)
{
	switch (i)
	{
		case EXC_BAD_ACCESS:
			return "EXC_BAD_ACCESS";
		case EXC_BAD_INSTRUCTION:
			return "EXC_BAD_INSTRUCTION";
		case EXC_ARITHMETIC:
			return "EXC_ARITHMETIC";
		case EXC_EMULATION:
			return "EXC_EMULATION";
		case EXC_SOFTWARE:
			return "EXC_SOFTWARE";
		case EXC_BREAKPOINT:
			return "EXC_BREAKPOINT";
		case EXC_SYSCALL:
			return "EXC_SYSCALL";
		case EXC_MACH_SYSCALL:
			return "EXC_MACH_SYSCALL";
		case EXC_RPC_ALERT:
			return "EXC_RPC_ALERT";

		default:
			return "???";
	}
}

	const char *
unparse_protection (vm_prot_t p)
{
	switch (p)
	{
		case VM_PROT_NONE:
			return "---";
		case VM_PROT_READ:
			return "r--";
		case VM_PROT_WRITE:
			return "-w-";
		case VM_PROT_READ | VM_PROT_WRITE:
			return "rw-";
		case VM_PROT_EXECUTE:
			return "--x";
		case VM_PROT_EXECUTE | VM_PROT_READ:
			return "r-x";
		case VM_PROT_EXECUTE | VM_PROT_WRITE:
			return "-wx";
		case VM_PROT_EXECUTE | VM_PROT_WRITE | VM_PROT_READ:
			return "rwx";
		default:
			return "???";
	}
}

	const char *
unparse_inheritance (vm_inherit_t i)
{
	switch (i)
	{
		case VM_INHERIT_SHARE:
			return "share";
		case VM_INHERIT_COPY:
			return "copy";
		case VM_INHERIT_NONE:
			return "none";
		default:
			return "???";
	}
}

	void
macosx_debug_region (task_t task, mach_vm_address_t address)
{
	macosx_debug_regions (task, address, 1);
}

	void
macosx_debug_regions (task_t task, mach_vm_address_t address, int max)
{
	kern_return_t kret;
	vm_region_basic_info_data_64_t info, prev_info;
	mach_vm_address_t prev_address;
	mach_vm_size_t size, prev_size;

	mach_port_t object_name;
	mach_msg_type_number_t count;

	int nsubregions = 0;
	int num_printed = 0;

	count = VM_REGION_BASIC_INFO_COUNT_64;
	kret = mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO_64,
			(vm_region_info_t) &info, &count, &object_name);
	if (kret != KERN_SUCCESS)
	{
		printf("No memory regions.\n");
		return;
	}
	memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_64_t));
	prev_address = address;
	prev_size = size;
	nsubregions = 1;

	for (;;)
	{
		int print = 0;
		int done = 0;

		address = prev_address + prev_size;

		/* Check to see if address space has wrapped around. */
		if (address == 0)
			print = done = 1;

		if (!done)
		{
			count = VM_REGION_BASIC_INFO_COUNT_64;
			kret =
				mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO_64,
						(vm_region_info_t) &info, &count, &object_name);
			if (kret != KERN_SUCCESS)
			{
				size = 0;
				print = done = 1;
			}
		}

		if (address != prev_address + prev_size)
			print = 1;

		if ((info.protection != prev_info.protection)
				|| (info.max_protection != prev_info.max_protection)
				|| (info.inheritance != prev_info.inheritance)
				|| (info.shared != prev_info.reserved)
				|| (info.reserved != prev_info.reserved))
			print = 1;

		if (print)
		{
			if (num_printed == 0)
				printf("Region ");
			else
				printf("   ... ");

			/*
			   printf("from 0x%s to 0x%s (%s, max %s; %s, %s, %s)",
			   paddr_nz (prev_address),
			   paddr_nz (prev_address + prev_size),
			   unparse_protection (prev_info.protection),
			   unparse_protection (prev_info.max_protection),
			   unparse_inheritance (prev_info.inheritance),
			   prev_info.shared ? "shared" : "private",
			   prev_info.reserved ? "reserved" : "not-reserved");
			 */

			if (nsubregions > 1)
				printf(" (%d sub-regions)", nsubregions);

			printf("\n");

			prev_address = address;
			prev_size = size;
			memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_64_t));
			nsubregions = 1;

			num_printed++;
		} else {
			prev_size += size;
			nsubregions++;
		}

		if ((max > 0) && (num_printed >= max))
			done = 1;

		if (done)
			break;
	}
}

int debug_setregs(pid_t tid, regs_t *regs)
{
	printf("TODO: SETREGS\n");
	/* TODO */
	return 0;
}

inline int debug_single_setregs(pid_t tid, regs_t *regs)
{
	return 0;
}

static int debug_enable(int enable)
{

	return 0;
}

int debug_print_wait(char *act)
{
	return 0;
}

static void debug_init_console()
{
}

int debug_os_init()
{
	return debug_enable(1);
}

static int debug_exception_event(unsigned long code)
{

	return 0;
}

int debug_dispatch_wait() 
{
	char data[1024];
	unsigned int gp_count, regs[17];
	kern_return_t err;
	mach_msg_header_t msg, out_msg;
	struct weasel_breakpoint *bkpt;

	fprintf(stderr, "Listening for exceptions.\n");

	err = mach_msg(&msg, MACH_RCV_MSG, 0, sizeof(data), exception_port, 0,
			MACH_PORT_NULL);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "Event listening failed.\n");
		return 1;
	}

	fprintf(stderr, "Exceptional event received.\n");

	//exc_server(&msg, &out_msg);

	/* if (mach_msg(&out_msg, MACH_SEND_MSG, sizeof(out_msg), 0, MACH_PORT_NULL,
	   0, MACH_PORT_NULL) != KERN_SUCCESS) {
	   fprintf(stderr, "Failed to send exception reply!\n");
	   exit(1);
	   } */

	fprintf(stderr, "Inferior received exception %x, %x.\n", (unsigned int)
			exception_type, (unsigned int)exception_data);

	gp_count = 17;
	thread_get_state(inferior_threads[0], 1, (thread_state_t)regs, &gp_count);

#if 0
	bkpt = breakpoints;
	while (bkpt) {
		if (bkpt->addr == regs[15]) {
			fprintf(stderr, "Breakpoint %d hit at $%08x.\n", bkpt->id,
					bkpt->addr);
			delete_breakpoint(bkpt->id);
			break;
		}

		bkpt = bkpt->next;
	}
}
#endif

return 0;
}


int debug_init_maps(int rest)
{

	macosx_debug_regions (0,0,999);//task_t task, mach_vm_address_t address, int max)
	return 0;
}

