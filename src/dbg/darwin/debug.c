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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the * GNU General Public License for more details.
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

#define MACH_ERROR_STRING(ret) \
(mach_error_string (ret) ? mach_error_string (ret) : "(unknown)")

/* working task and threads */
static task_t inferior_task = 0;
static thread_array_t inferior_threads = NULL;
static unsigned int inferior_thread_count = 0;

static exception_type_t exception_type;
static exception_data_t exception_data;
static mach_port_t exception_port; 
static int task = 0;
extern int errno;

int debug_os_kill(int pid, int sig)
{
	/* prevent killall selfdestruction */
	if (pid < 1)
		return -1;
	return kill(pid, sig);
}

// TODO: to be removed
int debug_ktrace()
{
}

int arch_print_syscall()
{
	/* dummy */
}

inline int debug_detach()
{
	ptrace(PT_DETACH, ps.tid, 0, 0);
	//inferiorinferior_in_ptrace = 0;
	return 0;
}


void inferior_abort_handler(int pid)
{
	fprintf(stderr, "Inferior received signal SIGABRT. Executing BKPT.\n");
}

task_t pid_to_task(int pid)
{
	static task_t old_pid  = -1;
	static task_t old_task = -1;
	task_t task = 0;
	int err;

	/* xlr8! */
	if (old_pid != -1 && old_pid == pid)
		return old_task;

	if (task_for_pid(mach_task_self(), (pid_t)pid, &task) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to get task for pid %d.\n", (int)pid, task);
		fprintf(stderr, "Reason: 0x%x: %s\n", err, MACH_ERROR_STRING(err));
		fprintf(stderr, "You probably need to add user to procmod group.\n"
				" Or chmod g+s radare && chown root:procmod radare\n");
		fprintf(stderr, "FMI: http://developer.apple.com/documentation/Darwin/Reference/ManPages/man8/taskgated.8.html\n");
		return -1;
	}
	old_pid = pid;
	old_task = task;
	
	return task;
}

// s/inferior_task/port/
int debug_attach(int pid)
{
	kern_return_t err;

	inferior_task = pid_to_task(pid);
	if (inferior_task == -1)
		return -1;

	task = inferior_task; // ugly global asignation
	fprintf(stderr, "; pid = %d\ntask= %d\n", pid, inferior_task);
	//ps.pid = ps.tid = task; // XXX HACK

	if (task_threads(task, &inferior_threads, &inferior_thread_count) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to get list of task's threads.\n");
		return -1;
	}
	fprintf(stderr, "Thread count: %d\n", inferior_thread_count);

	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &exception_port) != KERN_SUCCESS) {
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
	fprintf(stderr, "debug_contp: program is running now...\n");
	task_resume(inferior_task);
	thread_resume(inferior_threads[0]);
	//wait_for_events();
	return 0;
}

inline int debug_os_steps()
{
	fprintf(stderr, "debug_os_steps: TODO\n");
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
		//waitpid(kid, &status, 0);
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
		debug_os_kill(pid, SIGKILL);
		exit(1);
		return -1;
	}

	ps.pid = ps.tid = pid;
	return pid;
}

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
	int err = vm_read_overwrite(pid_to_task(tid), (unsigned int)addr, len, (pointer_t)buff, &size);
	if (err == -1) {
		printf("Cannot read\n");
		return -1;
	}
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

int debug_list_threads(int pid)
{
	int i, err;
	int gp_count;
	i386_thread_state_t  state;
	TH_INFO *th;

	if (task_threads(pid_to_task(pid), &inferior_threads, &inferior_thread_count) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to get list of task's threads.\n");
		return;
	}
	for (i = 0; i < inferior_thread_count; i++) {
		th = malloc(sizeof(TH_INFO));
		memset(th, '\0', sizeof(TH_INFO));
		th->tid = inferior_threads[i];
		
		if ((err=thread_get_state(th->tid, i386_THREAD_STATE, (thread_state_t) &state, &gp_count)) != KERN_SUCCESS) {
			fprintf(stderr, "FUCK %s\n", MACH_ERROR_STRING(err));
		} else{
			th->status = 0; // XXX TODO
			th->addr = state.__eip;
			//printf("ADDR: %08x\n", state.__eip);
		}
		add_th(th);

	}
}

int debug_getregs(pid_t tid, regs_t *regs)
{
	//unsigned int gp_count, regs[17]; // FOR ARM // IPHONE
	unsigned int gp_count;
	kern_return_t err;
	i386_thread_state_t  state;

	thread_act_port_array_t thread_list;
	mach_msg_type_number_t thread_count;

	err = task_threads(pid_to_task(tid), &inferior_threads, &inferior_thread_count);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "FUCK\n");
	} else {
		//fprintf(stderr, "THREAD COUNT: %d\n", inferior_thread_count);
	}

	/* TODO: allow to choose the selected thread */
	if ((err = thread_get_state(inferior_threads[0], i386_THREAD_STATE, (thread_state_t) &state, &gp_count)) != KERN_SUCCESS) {
		fprintf(stderr, "getregs: Failed to get thread %d .error (%x).\n", (int)tid, (int)err);
		return -1;
	}
	// XXX USELESS COPY
	memcpy(regs, &state, sizeof(regs_t));

	return gp_count;
}

int debug_setregs(pid_t tid, regs_t *regs)
{
	//unsigned int gp_count, regs[17];
	unsigned int gp_count;
	kern_return_t err;

	/* thread_id, flavor, old_state, old_state_count */
	if ((err = thread_set_state(inferior_threads[0], i386_THREAD_STATE, (thread_state_t) regs, &gp_count)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to get thread %d state (%d).\n", (int)tid, (int)err);
		return -1;
	}

	return gp_count;
}

/* XXX this must be structures in arrays or so */
const char * unparse_exception_type (unsigned int i)
{
	switch (i) {
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
	}
	return "???";
}

const char * unparse_protection (vm_prot_t p)
{
	switch (p) {
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

int darwin_prot_to_unix(int prot)
{
	switch(prot) {
	case VM_PROT_NONE:
		return REGION_NONE;
	case VM_PROT_READ:
		return REGION_READ;
	case VM_PROT_WRITE:
		return REGION_WRITE;
	case VM_PROT_READ | VM_PROT_WRITE:
		return REGION_READ | REGION_WRITE;
	case VM_PROT_EXECUTE:
		return REGION_EXEC;
	case VM_PROT_EXECUTE | VM_PROT_READ:
		return REGION_EXEC | REGION_READ;
	case VM_PROT_EXECUTE | VM_PROT_WRITE:
		return REGION_EXEC | REGION_WRITE;
	case VM_PROT_EXECUTE | VM_PROT_WRITE | VM_PROT_READ:
		return REGION_READ | REGION_WRITE | REGION_EXEC;
	default:
		return 7; // UNKNOWN
	}
}

const char * unparse_inheritance (vm_inherit_t i)
{
	switch (i) {
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

void macosx_debug_region (task_t task, mach_vm_address_t address)
{
	macosx_debug_regions (task, address, 1);
}

void macosx_debug_regions (task_t task, mach_vm_address_t address, int max, int rest)
{
	char buf[128];
	int i;
	kern_return_t kret;
	vm_region_basic_info_data_64_t info, prev_info;
	mach_vm_address_t prev_address;
	mach_vm_size_t size, prev_size;
	mach_port_t object_name;
	mach_msg_type_number_t count;
	int nsubregions = 0;
	int num_printed = 0;
	MAP_REG *mr;

	if(ps.map_regs_sz > 0)
		free_regmaps(rest);

	count = VM_REGION_BASIC_INFO_COUNT_64;
	kret = mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO_64,
			(vm_region_info_t) &info, &count, &object_name);
	if (kret != KERN_SUCCESS) {
		printf("No memory regions.\n");
		return;
	}
	memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_64_t));
	prev_address = address;
	prev_size = size;
	nsubregions = 1;

	for (i=0;;i++) {
		int print = 0;
		int done = 0;

		address = prev_address + prev_size;

		/* Check to see if address space has wrapped around. */
		if (address == 0)
			print = done = 1;

		if (!done) {
			count = VM_REGION_BASIC_INFO_COUNT_64;
			kret = mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO_64,
					(vm_region_info_t) &info, &count, &object_name);
			if (kret != KERN_SUCCESS) {
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

		mr = malloc(sizeof(MAP_REG));
		mr->ini = prev_address;
		mr->end = prev_address + prev_size;
		mr->size = prev_size;
		
		sprintf(buf, "unk%d-%s-%s-%s", i,
			   unparse_inheritance (prev_info.inheritance),
			   prev_info.shared ? "shar" : "priv",
			   prev_info.reserved ? "reserved" : "not-reserved");
		mr->bin = strdup(buf);
		mr->perms = darwin_prot_to_unix(prev_info.protection); // XXX is this ok?
		//mr->flags = // FLAG_NOPERM  // FLAG_USERCODE ...
		//mr->perms = prev_info.max_protection;

		add_regmap(mr);

		if (1==0 && rest) { /* XXX never pritn this info here */
			if (num_printed == 0)
				fprintf(stderr, "Region ");
			else
				fprintf(stderr, "   ... ");
			   fprintf(stderr, " 0x%08llx - 0x%08llx %s (%s) %s, %s, %s",
			   (u64)prev_address, (u64)(prev_address + prev_size),
			   unparse_protection (prev_info.protection),
			   unparse_protection (prev_info.max_protection),
			   unparse_inheritance (prev_info.inheritance),
			   prev_info.shared ? "shared" : " private",
			   prev_info.reserved ? "reserved" : "not-reserved");

			if (nsubregions > 1)
				fprintf(stderr, " (%d sub-regions)", nsubregions);

			fprintf(stderr, "\n");

			prev_address = address;
			prev_size = size;
			memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_64_t));
			nsubregions = 1;

			num_printed++;
		} else {
#if 0
			prev_size += size;
			nsubregions++;
#else
			prev_address = address;
			prev_size = size;
			memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_64_t));
			nsubregions = 1;

			num_printed++;
#endif
		}

		if ((max > 0) && (num_printed >= max))
			done = 1;

		if (done)
			break;
	}
}

inline int debug_single_setregs(pid_t tid, regs_t *regs)
{
	fprintf(stderr, "debug_single_setregs: TODO\n");
	return 0;
}

int debug_print_wait(char *act)
{
	fprintf(stderr, "debug_print_wait: TODO\n");
	return 0;
}

int debug_os_init()
{
	return 0;
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

	printf("debug_dispatch_wait: TODO\n");
	return 0;

	fprintf(stderr, "Waiting for events... (kill -STOP %d to get prompt)\n",ps.tid);

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

#if 0
	gp_count = 17;
	thread_get_state(inferior_threads[0], 0, (thread_state_t)regs, &gp_count);
#endif

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
	//task_t task, mach_vm_address_t address, int max)
	macosx_debug_regions(task, 0, 999, rest);
	return 0;
}
