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

#define __addr_t_defined

#include "../mem.h"
#include "../debug.h"
#include "../thread.h"
//#include "utils.h"

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

int debug_attach(int pid)
{
	kern_return_t err;

	signal(SIGCHLD, child_handler);

	if ((err = task_for_pid(mach_task_self(), (int)pid, &inferior_task) !=
				KERN_SUCCESS)) {
		fprintf(stderr, "Failed to get task for pid %d: %d.\n", (int)inferior,
				(int)err);
		return 1;
	}

	if (task_threads(inferior_task, &inferior_threads, &inferior_thread_count)
			!= KERN_SUCCESS) {
		fprintf(stderr, "Failed to get list of task's threads.\n");
		return 1;
	}

	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				&exception_port) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to create exception port.\n");
		return 1;
	}
	if (mach_port_insert_right(mach_task_self(), exception_port,
				exception_port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to acquire insertion rights on the port.\n");
		return 1;
	}
	if (task_set_exception_ports(inferior_task, EXC_MASK_ALL, exception_port,
				EXCEPTION_DEFAULT, THREAD_STATE_NONE) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to set the inferior's exception ports.\n");
		return 1;
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

        exit(0);
    }

    child_args = (char **)malloc(sizeof(char *) * (argc + 1));
    memcpy(child_args, argv, sizeof(char *) * argc);
    child_args[argc] = NULL;

    ptrace(PT_TRACE_ME, 0, 0, 0);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGABRT, inferior_abort_handler);

    execvp(argv[0], child_args);

    fprintf(stderr, "Failed to start inferior.\n");
    exit(1);

    return 0;   /* should never get here */
}

int debug_set_bp_mem(int pid, u64 addr, int len)
{
// TODO: not used
}

int debug_fork_and_attach()
{
	char **argv = { ps.
	/* TODO */
	start_inferior(
}

int debug_read_at(pid_t tid, void *buff, int len, u64 addr)
{
	int ret_len = 0;
	int err = vm_read_overwrite(inferior_task, addr, len, (pointer_t)buff, &ret_len);
	if (err == -1)
		return -1;
	return ret_len;
}

int debug_write_at(pid_t tid, void *buff, int len, u64 addr)
{
	int err = vm_write(inferior_task, addr, len, (pointer_t)buff, (mach_msg_type_number_t)4);
	if (err == -1)
		return -1;

	return ret_len;
}


int debug_getregs(pid_t tid, regs_t *regs)
{
	/* TODO */
	return 0;
}

int debug_setregs(pid_t tid, regs_t *regs)
{
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

	exc_server(&msg, &out_msg);

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

static inline int CheckValidPE(unsigned char * PeHeader)
{    	    

	return 0;
}


int debug_init_maps(int rest)
{

	return 0;
}

