/*
 * Copyright (C) 2008, 2009
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
#define EXCEPTION_PORT 0

#define __addr_t_defined

#include "../mem.h"
#include "../debug.h"
#include "../libps2fd.h"
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
static int inferior_in_ptrace = 1;

/* SIGCHLD handler */
void child_handler(int pid)
{
    int status;

printf("WAITIING IN CHILD HANDL\n");
fflush(stdout);
    wait(&status);
    if (WIFEXITED(status)) {
        fprintf(stderr, "Inferior %d exited with status %d.\n",
		pid, (int) WEXITSTATUS(status));
        exit(0);
    } else if (WIFSTOPPED(status)) {
        fprintf(stderr, "Inferior %d is stopped %d.\n",
		pid, (int) WEXITSTATUS(status));
	
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "Inferior %d exited with signal %d.\n", 
		pid, (int)WTERMSIG(status));
        exit(0);
    }
}

int debug_os_kill(int pid, int sig)
{
	/* prevent killall selfdestruction */
	if (pid < 1)
		return -1;
	return kill(pid, sig);
}

vm_prot_t unix_prot_to_darwin(int prot)
{
	return ((prot&1<<4)?VM_PROT_READ:0 |
		(prot&1<<2)?VM_PROT_WRITE:0 |
		(prot&1<<1)?VM_PROT_EXECUTE:0);
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

int debug_detach()
{
	ptrace(PT_DETACH, ps.tid, 0, 0);
	//inferiorinferior_in_ptrace = 0;
	return 0;
}

void inferior_abort_handler(int pid)
{
	fprintf(stderr, "Inferior received signal SIGABRT. Executing BKPT.\n");
	fflush(stderr);
}

task_t pid_to_task(int pid)
{
	static task_t old_pid  = -1;
	static task_t old_task = -1;
	task_t task = 0;
	int err;

	/* xlr8! */
	if (old_task!= -1) //old_pid != -1 && old_pid == pid)
		return old_task;

	err = task_for_pid(mach_task_self(), (pid_t)pid, &task);
	if ((err != KERN_SUCCESS) || !MACH_PORT_VALID(task)) {
		fprintf(stderr, "Failed to get task %d for pid %d.\n", (int)task, (int)pid);
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
#if SUSPEND
  if ( task_suspend ( this -> port ) != KERN_SUCCESS )
  {
    return ( FALSE );
  }
#endif
/* is this required for arm ? */
#if EXCEPTION_PORT
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
#endif
	return 0;
}

int debug_contp(int tid)
{
	fprintf(stderr, "debug_contp: program is now running...\n");

	if (inferior_in_ptrace) {
		/* only stopped with ptrace the first time */
		//ptrace(PT_CONTINUE, ps.tid, 0, 0);
		ptrace(PT_DETACH, ps.tid, 0, 0);
		inferior_in_ptrace = 1;
	} else {
		task_resume(inferior_task); // ???
		thread_resume(inferior_threads[0]);
	}

	return 0;
}

#if __arm || __arm__ || __POWERPC__
void debug_arch_x86_trap_set(int pid, int foo)
{
	/* do nothing here */
}

#else

// XXX intel specific
#define EFLAGS_TRAP_FLAG 0x100
void debug_arch_x86_trap_set(int pid, int foo)
{
	regs_t regs;
	debug_getregs(pid_to_task(pid), &regs);
	printf("trap flag: %d\n", (regs.__eflags&0x100));
	if ( foo ) regs.__eflags |= EFLAGS_TRAP_FLAG;
	else regs.__eflags &= ~EFLAGS_TRAP_FLAG;
	debug_setregs(pid_to_task(pid), &regs);
}
#endif

int QMACHINE_THREAD_STATE_COUNT = sizeof(regs_t)/4;
int debug_os_steps()
{
	int ret;
	regs_t regs;

	debug_arch_x86_trap_set(ps.tid, 1);

#define OLD_PANCAKE_CODE 1
#if OLD_PANCAKE_CODE
	printf("stepping from pc = %08x\n", (u32)get_offset("eip"));
	//ret = ptrace(PT_STEP, ps.tid, (caddr_t)get_offset("eip"), SIGSTOP);
	ret = ptrace(PT_STEP, ps.tid, (caddr_t)1, SIGTRAP); //SIGINT);
	if (ret != 0) {
		perror("ptrace-step");
		fprintf(stderr, "FUCK: %s\n", MACH_ERROR_STRING(ret));
		fprintf(stderr, "debug_os_steps: %d\n", ret);
		/* DO NOT WAIT FOR EVENTS !!! */
		return -1;
	}
#endif
#if 0
  [ESRCH]
           No process having the specified process ID exists.

     [EINVAL]
           oo   A process attempted to use PT_ATTACH on itself.
           oo   The request was not one of the legal requests.
           oo   The signal number (in data) to PT_CONTINUE was neither 0 nor a legal signal number.
           oo   PT_GETREGS, PT_SETREGS, PT_GETFPREGS, or PT_SETFPREGS was attempted on a process with no
               valid register set.  (This is normally true only of system processes.)

     [EBUSY]
           oo   PT_ATTACH was attempted on a process that was already being traced.
           oo   A request attempted to manipulate a process that was being traced by some process other than
               the one making the request.
           oo   A request (other than PT_ATTACH) specified a process that wasn't stopped.

     [EPERM]
           oo   A request (other than PT_ATTACH) attempted to manipulate a process that wasn't being traced
               at all.
           oo   An attempt was made to use PT_ATTACH on a process in violation of the requirements listed
               under PT_ATTACH above.
#endif
	return 0;
}

/* TODO: not work */
int debug_contfork(int tid) { eprintf("not work yet\n"); return -1; }
int debug_contscp() { eprintf("not work yet\n"); return -1; }
int debug_status() { eprintf("not work yet\n"); return -1; }

int debug_fork()
{
	int forksyscall = 2; // XXX must be collected from foo
	return arch_syscall(ps.tid, forksyscall);
	// XXX restore child process memory
}

int debug_ktrace()
{
	/* TODO ? */
}


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
	/* XXX this can be monitorized by the child */
	signal(SIGTRAP, SIG_IGN); // SINO NO FUNCIONA EL STEP
	signal(SIGABRT, inferior_abort_handler);

	execvp(argv[0], child_args);

	eprintf("Failed to start inferior.\n");
	return 0;
}

int debug_fork_and_attach()
{
	char **argv = { "/bin/ls", 0 };
	int err;
	pid_t pid;
	/* TODO */
	pid = start_inferior(1, &ps.argv); //&argv);
	if (pid == -1) {
		printf("inferior pid is -1??!?!\n");
		return -1;
	}

	err = debug_attach(pid);
	if (err == -1) {
		debug_os_kill(pid, SIGKILL);
		exit(1);
		return -1;
	}

	ps.pid = ps.tid = pid;
	return pid;
}

// XXX This must be implemented on all the os/debug.c instead of arch_mprotect
int debug_os_mprotect(u64 addr, int len, int perms)
{
	return vm_protect(pid_to_task(ps.tid),
		(vm_address_t)addr,
		(vm_size_t)len,
		(boolean_t)0, /* maximum protection */
		unix_prot_to_darwin(perms));
}

#if 0
debug_os_alloc()
debug_os_dealloc()
#endif
#if 0
kern_return_t   vm_allocate
                (vm_task_t                          target_task,
                 vm_address_t                           address,
                 vm_size_t                                 size,
                 boolean_t                             anywhere);

 anywhere
    [in scalar] Placement indicator. The valid values are:

    TRUE
        The kernel allocates the region in the next unused space that is sufficient within the address space. The kernel returns the starting address actually used in address.

    FALSE
        The kernel allocates the region starting at address unless that space is already allocated. 

---------------------

kern_return_t   vm_deallocate
                (vm_task_t                          target_task,
                 vm_address_t                           address,
                 vm_size_t                                 size);

/* MAP AND REMAP */

Function - Map memory objects in one task's address space to that of another task's. 

http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/vm_remap.html
http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/vm_map.html

#endif

int debug_os_read_at(pid_t tid, void *buff, int len, u64 addr)
{
	unsigned int size= 0;
	int err = vm_read_overwrite(pid_to_task(tid), (unsigned int)addr, len, (pointer_t)buff, &size);
	if (err == -1) {
		fprintf(stderr, "Cannot read\n");
		return -1;
	}
	return size;
}

int debug_os_write_at(pid_t tid, void *buff, int len, u64 addr)
{
	// XXX SHOULD RESTORE PERMS LATER!!!
	vm_protect(pid_to_task(tid), addr, len, 0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);

	kern_return_t err = vm_write(
		pid_to_task(tid),
		(vm_address_t)(unsigned int)addr, // XXX not for 64 bits
		(pointer_t)buff,
		(mach_msg_type_number_t)len);

	if (err == KERN_SUCCESS)
		return len;

	fprintf(stderr, "Oops (0x%llx) error (%s)\n", addr, MACH_ERROR_STRING(err));

	return -1;
}

int debug_list_threads(int pid)
{
	int i, err;
	int gp_count;
	regs_t state;
	TH_INFO *th;

	if (task_threads(pid_to_task(pid), &inferior_threads, &inferior_thread_count) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to get list of task's threads.\n");
		return;
	}
	for (i = 0; i < inferior_thread_count; i++) {
		th = malloc(sizeof(TH_INFO));
		memset(th, '\0', sizeof(TH_INFO));
		th->tid = inferior_threads[i];
		
		if ((err=thread_get_state(th->tid, THREAD_STATE, (thread_state_t) &state, &gp_count)) != KERN_SUCCESS) {
			//fprintf(stderr, "debug_list_threads: %s\n", MACH_ERROR_STRING(err));
		} else{
			th->status = 0; // XXX TODO
#if __arm__
			th->addr = state.r15;
#elif __POWERPC__
			th->addr = state.srr0;
#else
			th->addr = state.__eip;
#endif
			//printf("ADDR: %08llx\n", state.srr0);
		}
		add_th(th);

	}
}

int debug_getregs(pid_t tid, regs_t *regs)
{
	unsigned int gp_count = sizeof(regs_t);
	kern_return_t err;
	regs_t state;

	thread_act_port_array_t thread_list;
	mach_msg_type_number_t thread_count;
//fprintf(stderr, "getregs-TID: %d\n", tid);

	err = task_threads(pid_to_task(tid), &inferior_threads, &inferior_thread_count);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "debug_getregs\n");
		return -1;
	}

	if (inferior_thread_count>0) {
		/* TODO: allow to choose the thread */
		if ((err = thread_get_state(inferior_threads[0], THREAD_STATE, (thread_state_t) regs, &gp_count)) != KERN_SUCCESS) {
			fprintf(stderr, "debug_getregs: Failed to get thread %d %d.error (%x). (%s)\n", (int)tid, pid_to_task(tid), (int)err, MACH_ERROR_STRING(err));
			perror("thread_get_state");
			return -1;
		}
	} else {
		fprintf(stderr, "There are no threads!\n");
	}

	return gp_count;
}

int debug_setregs(pid_t tid, regs_t *regs)
{
	//unsigned int gp_count, regs[17];
	kern_return_t err;

	err = task_threads(pid_to_task(tid), &inferior_threads, &inferior_thread_count);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "debug_getregs\n");
		return -1;
	} 

	/* thread_id, flavor, old_state, old_state_count */
	if (inferior_thread_count>0) {
		if ((err = thread_set_state(inferior_threads[0], THREAD_STATE, (thread_state_t) regs, sizeof(regs_t)/4)) != KERN_SUCCESS) { /* regs_sizeof */
			fprintf(stderr, "setregs: Failed to get thread %d %d.error (%x). (%s)\n", (int)tid, pid_to_task(tid), (int)err, MACH_ERROR_STRING(err));
			perror("thread_set_state");
			return -1;
		}
	} else {
		// XXX
	}

	return 0;
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
			else	fprintf(stderr, "   ... ");
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

int debug_single_setregs(pid_t tid, regs_t *regs)
{
	fprintf(stderr, "debug_single_setregs: TODO\n");
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

int debug_print_wait(char *act)
{
	fprintf(stderr, "debug_print_wait: TODO\n");
	return 0;
}

#if 0
static enum bp_ss_or_other
get_event_type (struct macosx_exception_thread_message *msg)
{
  if (msg->exception_type == EXC_BREAKPOINT)
    {
      if (msg->data_count == 2
          && msg->exception_data[0] == SINGLE_STEP)
        return ss_event;
      else
        return bp_event;
    }
  else if (msg->exception_type == EXC_SOFTWARE
      && (msg->data_count == 2)
      && (msg->exception_data[0] == EXC_SOFT_SIGNAL))
    return sig_event;
  else
    return other_event;
}
#endif

int debug_dispatch_wait() 
{
	char data[1024];
	unsigned int gp_count, regs[17];
	kern_return_t err;
	mach_msg_header_t msg, out_msg;
	struct weasel_breakpoint *bkpt;

	//printf("debug_dispatch_wait: TODO\n");
	//return 0;

	fprintf(stderr, "Waiting for events... (kill -STOP %d to get prompt)\n",ps.tid);

#if EXCEPTION_PORT
	err = mach_msg(&msg, MACH_RCV_MSG, 0, sizeof(data), exception_port, 1, MACH_PORT_NULL);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "Event listening failed.\n");
		return 1;
	}
#endif

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
