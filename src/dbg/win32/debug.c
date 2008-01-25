/*
 * Copyright (C) 2008
 *       th0rpe <nopcode.org>
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

#include "../libps2fd.h"
#include <tlhelp32.h>
#include "utils.h"
#include <stdio.h>

#if __GYGWIN__
#include <sys/errno.h>
#endif

BOOL WINAPI DebugActiveProcessStop(DWORD dwProcessId);

static BOOL WINAPI (*win32_detach)(DWORD);
static HANDLE WINAPI (*win32_openthread)(DWORD, BOOL, DWORD);
static HANDLE WINAPI (*win32_dbgbreak)(HANDLE);
static DWORD WINAPI (*win32_getthreadid)(HANDLE);

/* mark step flag (DEPRECATED) */
static int single_step;
static int stopped_thread;
static int exit_wait;

void debug_init_calls()
{
	win32_detach = GetProcAddress(GetModuleHandle("kernel32"),
		       	"DebugActiveProcessStop");

	win32_openthread = GetProcAddress(GetModuleHandle("kernel32"),
		       	"OpenThread");

	win32_dbgbreak = GetProcAddress(GetModuleHandle("kernel32"),
		       	"DebugBreakProcess");

	win32_getthreadid = GetProcAddress(GetModuleHandle("kernel32"),
		       	"GetThreadId");

	if(win32_detach == NULL || win32_openthread == NULL ||
	   win32_dbgbreak == NULL || win32_getthreadid == NULL) {
		print_lasterr(__FUNCTION__);
		exit(1);
	}
}

/* console events handler */
static BOOL dispatch_console(DWORD type)
{
	switch(type) {
		case CTRL_C_EVENT: 
			exit_wait = 1;
			break;

	}
}

static HANDLE tid2handler(pid_t tid)
{
	TH_INFO *th = get_th(tid);
	if(th == NULL)
		return NULL;

	//eprintf("thread handler: %d %d\n", th->tid, th->ht);

	return th->ht;
}

inline static int handler2tid(HANDLE h)
{
	return win32_getthreadid(h);
}

inline static int handler2pid(HANDLE h)
{
	return GetProcessId(h);
}

inline int debug_attach(pid_t pid) 
{
	OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
	if(DebugActiveProcess(pid)) {
		/* set process id */
		ps.pid = pid;
		/* set active tid */
		ps.tid = debug_load_threads();
		if (ps.tid<0) { // no default active thread
			eprintf("No default active thread. Taking root as default.\n");
			ps.tid = ps.pid;
		}
		return 0;
	}
	eprintf("Cannot attach\n");
	return -1;
}

inline int debug_detach(pid_t pid)
{
	return win32_detach(pid)? 0 : -1;
}

int debug_load_threads()
{
	HANDLE th;
	THREADENTRY32 te32;
	TH_INFO *th_i;
	int ret;

	ret = -1;
	te32.dwSize = sizeof(THREADENTRY32);

	th = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, ps.pid); 
	if(th == INVALID_HANDLE_VALUE || !Thread32First(th, &te32))
		goto err_load_th;

	/* TODO: free all threads */
	//free_th();

	do {
		/* get all threads of process */
		if( te32.th32OwnerProcessID == ps.pid ) {

			/* add a new thread at threads list */
			th_i = init_th(te32.th32ThreadID, te32.dwFlags);
			if(th_i == NULL)
				goto err_load_th;

			/* open a new handler */
			th_i->ht = win32_openthread(THREAD_ALL_ACCESS, 0,
					te32.th32ThreadID);

			if(th_i->ht == NULL)
				goto err_load_th;

			ret = te32.th32ThreadID;
			add_th(th_i);
		}
	} while(Thread32Next(th, &te32));

err_load_th:	

	if(ret) 
		print_lasterr(__FUNCTION__);

	if(th != INVALID_HANDLE_VALUE)
		CloseHandle(th);

	return ret;
}

int debug_contp(int tid)
{
	int ret = 0;

	if(stopped_thread) {

		if(ResumeThread(tid2handler(stopped_thread)) == -1) {
			print_lasterr(__FUNCTION__);
			ret = -1;
		} else{
			stopped_thread = 0;	
		}

	} else {
		if(ContinueDebugEvent(ps.pid, tid, DBG_CONTINUE) == 0) {
			print_lasterr(__FUNCTION__);
			ret = -1;
		}
	}

	return ret;
}

inline int debug_steps()
{
	regs_t regs;

	/* up TRAP flag */
	debug_getregs(ps.tid, &regs);
	regs.EFlags |= 0x100;
	debug_setregs(ps.tid, &regs);

	single_step = ps.tid;

	/* continue */
	debug_contp(ps.tid);
	 
}

/* TODO: not work */
int debug_contfork(int tid) { eprintf("not work yet\n"); return -1; }
int debug_init_maps(int rest) { eprintf("debug_init_maps : not work yet\n"); return -1; }
int debug_contscp() { eprintf("not work yet\n"); return -1; }
int debug_status() { eprintf("not work yet\n"); return -1; }
int debug_pstree() { eprintf("not work yet\n"); return -1; }

int debug_fork_and_attach()
{
	STARTUPINFO si = { sizeof(si) };
	DEBUG_EVENT de;
	int i;

	/* TODO: with args */
	if( !CreateProcess(ps.argv[0], NULL,
		       	NULL, NULL, FALSE, 
			CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS,
	       		NULL, NULL, &si, &ps.pi ) ) {

		print_lasterr(__FUNCTION__);
		return -1;
	}

	/* set process id and thread id */
	ps.pid = WIN32_PI(dwProcessId);
	ps.tid = WIN32_PI(dwThreadId);

	debug_environment();

	if (config_get("cfg.verbose")) {
		printf("argv = ");
		for(i=0;ps.argv[i];i++)
			printf("'%s', ", ps.argv[i]);
		printf("\n");
	}

	/* restore breakpoints */
	debug_reload_bps();

	/* load thread list */
	debug_load_threads();

	/* continue process */
	debug_contp(ps.tid);

	/* catch create process event */
	if(!WaitForDebugEvent(&de, 10000)) 
		goto err_fork;

	/* check if is a create process debug event */
	if(de.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT) {
		eprintf("exception code %d\n",
			       	de.dwDebugEventCode);
		goto err_fork;
	}

	ps.opened = 1;
	ps.steps = 1;

	return 0;

err_fork:

	TerminateProcess(WIN32_PI(hProcess), 1);
	return -1;

#if 0
	/* ignore exception */
	debug_contp(ps.tid);

	/* handle new exceptions */
	debug_dispatch_wait();
#endif

	return 0;
}

int debug_read_at(pid_t tid, void *buff, int len, off_t addr)
{
	int ret_len = 0;

/*
	HANDLE hp;
	eprintf("addr: 0x%x\n", addr);
        eprintf("Marcial len: %d\n", len);
	eprintf("handler: 0x%x %d %d\n", hp, WIN32_PI(dwProcessId), ps.pid);
	*/

	ReadProcessMemory(WIN32_PI(hProcess), addr,
		       	(LPVOID)buff, len, (SIZE_T *)&ret_len);

	return ret_len;
}

int debug_write_at(pid_t tid, void *buff, int len, off_t addr)
{
	int ret_len = -1;
	DWORD old;

	/* change page permissions */
	/*
	if(VirtualProtectEx(WIN32_PI(hProcess), addr, 
				len, PAGE_READWRITE, &old) == 0)

		return -1;
		*/

	/* write memory */
	WriteProcessMemory(WIN32_PI(hProcess), (LPVOID)addr, (LPVOID)buff,
		       	len, (SIZE_T *)&ret_len);

	/* restore page permissions */
	/*
	VirtualProtectEx(WIN32_PI(hProcess), addr, 
				len, old, NULL);
				*/

	return ret_len;
}


inline int debug_getregs(pid_t tid, regs_t *regs)
{
	regs->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	return GetThreadContext(tid2handler(tid), regs)? 0 : -1; 
}

inline int debug_setregs(pid_t tid, regs_t *regs)
{
	regs->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	return SetThreadContext(tid2handler(tid), regs)? 0 : -1;
}

inline int debug_single_setregs(pid_t tid, regs_t *regs)
{
	regs->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	return SetThreadContext(tid2handler(tid), regs)? 0 : -1;
}

static int debug_enable(int enable)
{
	HANDLE tok;  
	TOKEN_PRIVILEGES tp;
	DWORD err;

	tok = NULL;
	err = -1;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
	   &tok))
		goto err_enable;

	tp.PrivilegeCount = 1;
	if(!LookupPrivilegeValue(NULL,  SE_DEBUG_NAME, 
				&tp.Privileges[0].Luid))
		goto err_enable;

	tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

	if(!AdjustTokenPrivileges(tok, 0, &tp, sizeof(tp), NULL, NULL)) 
		goto err_enable;

	err = 0;

err_enable:

	if(tok != NULL)
		CloseHandle(tok);

	if(err)
		print_lasterr((char *)__FUNCTION__);

	return err;
}

int debug_print_wait(char *act)
{
	if (act)
	switch(WS(event)) {
	case BP_EVENT:
		eprintf("%s: breakpoint stop (0x%x)\n", act, WS(bp)->addr);
		break;

	case INT3_EVENT:
		eprintf("%s: int3 stop at (0x%x)\n", act, arch_pc());
		break;
	default:
		if(WS(event) != EXIT_EVENT ) {
			/* XXX: update thread list information here !!! */
			eprintf("=== %s: (%d) stop at 0x%x(%s)\n",
				act, ps.tid, (unsigned int)arch_pc(), //WS_PC() is not portable
				stopped_thread? "INT" : "UNKNOWN");
		}
	}
	return 0;
}

static void debug_init_console()
{
	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)dispatch_console,
			       	TRUE ) == 0) {
		print_lasterr(__FUNCTION__);
		return;
	}
}


int debug_os_init()
{
	WS(regs).ContextFlags = CONTEXT_FULL;

	debug_init_calls();
	debug_init_console();

	/* enable debug privilegies */
	return debug_enable(1);
}

static int debug_exception_event(unsigned long code)
{
	struct bpt_t *bp;
	int next_event;

	next_event = 0;

	switch(code) {

		/* software breakpoint or int3 */
		case EXCEPTION_BREAKPOINT:

			bp = (struct bp_t*) arch_stopped_bp();
			if(bp) {
				WS(event) = BP_EVENT;
				WS(bp) = bp;

			} else if(/* TODO: ignore int3 stop */ 1) {
				//eprintf("IGNORE INT 3 EXCEPTION\n");
				next_event = 1;
				debug_contp(ps.tid);
			}

			break;

		/* hardware breakpoint or single step exception */
		case EXCEPTION_SINGLE_STEP: 

			bp = (struct bp_t*) arch_stopped_bp();
			if(bp) {
				WS(event) = BP_EVENT;
				WS(bp) = bp;
			} else if(single_step == ps.tid) {
				single_step = 0;
			} else if(/* TODO: ignore step in excep */ 1) {
				//eprintf("IGNORE SINGLE STEP EXCEPTION\n");
				next_event = 1;
				debug_contp(ps.tid);
			}

			break;

		default:
			eprintf("exceptio 0x%x uncatched\n", code);

	}

	return next_event;
}

int debug_dispatch_wait() 
{
	DEBUG_EVENT de;
	int next_event = 0;
	unsigned int code;

	do {
		/* handle debug events */
		/* TODO: exit_wait is DEPRECATED */
		exit_wait = 0;

		while(WaitForDebugEvent(&de, 0) == 0) {

			/* interrupible call */
			usleep(1000);

			if(exit_wait) {

				if(SuspendThread(tid2handler(ps.tid)) == -1) {
					print_lasterr(__FUNCTION__);
					return -1;
				}

				stopped_thread = ps.tid;
				exit_wait = 0;
				return 0;
			}
		}

		/* set state */
		WS(event) = UNKNOWN_EVENT;

		/* save thread id */
		ps.tid = de.dwThreadId;


		/* save registers */
		debug_getregs(ps.tid, &WS(regs));

		/*
		eprintf("get regs: %d\n", debug_getregs(ps.tid, &WS(regs)));
		eprintf("Dr3: 0x%x\n", WS(regs).Dr3);
		*/
		/* get kind of event */
		code = de.dwDebugEventCode;

		switch(code) {

			case CREATE_PROCESS_DEBUG_EVENT:
				eprintf("(%d) created process (%d:0x%x)\n",
					    ps.tid, 
					    handler2tid(de.u.CreateProcessInfo.
						    hProcess),
					 de.u.CreateProcessInfo.lpStartAddress);

				debug_contp(ps.tid);
				next_event = 1;
				break;

			case EXIT_PROCESS_DEBUG_EVENT:

				eprintf("\n\n______________[ process "
					"finished ]_______________\n\n");

				ps.opened = 0;
				debug_load();
				next_event = 0;
				break;

			case CREATE_THREAD_DEBUG_EVENT:

				eprintf("(%d) created thread (0x%x)\n",
					    ps.tid, 
					    de.u.CreateThread.lpStartAddress);

				debug_contp(ps.tid);
				next_event = 1;
				break;

			case EXIT_THREAD_DEBUG_EVENT:

				eprintf("EXIT_THREAD\n");
				debug_contp(ps.tid);
				next_event = 1;
				break;

			case LOAD_DLL_DEBUG_EVENT:



				eprintf("(%d) Loading %s library at 0x%x\n",
					ps.tid,
					"",
					de.u.LoadDll.lpBaseOfDll);

				debug_contp(ps.tid);
				next_event = 1;
				break;

			case UNLOAD_DLL_DEBUG_EVENT:

				eprintf("UNLOAD_DLL\n");
				debug_contp(ps.tid);
				next_event = 1;
				break;

			case OUTPUT_DEBUG_STRING_EVENT:

				eprintf("OUTPUT_DBUG_STING\n");
				debug_contp(ps.tid);
				next_event = 1;
				break;

			case RIP_EVENT:

				eprintf("RIP_EVENT\n");
				debug_contp(ps.tid);
				next_event = 1;

				break;
			case EXCEPTION_DEBUG_EVENT:

				next_event = debug_exception_event(
				de.u.Exception.ExceptionRecord.ExceptionCode);
				break;

			default:

			print_lasterr(__FUNCTION__);
			return -1;
		}

	} while(next_event);

	return 0;
}

