/*
 * Copyright (C) 2008
 *       th0rpe <nopcode.org>
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

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <winbase.h>
#include <psapi.h>

#if __CYGWIN__

/* CYGWIN declare only GetProcessID() 
 * if _WIN32_WINNT >= 0x0501 
 * */
DWORD WINAPI GetProcessId(HANDLE);
#endif

#include "../mem.h"
#include "../debug.h"
#include "../thread.h"
#include "utils.h"

int debug_os_kill(int pid, int sig)
{
	/* prevent oops */
	if (pid < 1)
		return -1;
	/* XXX: this is not ok, but.. can w32 send other signals? */
	TerminateProcess(WIN32_PI(hProcess), 1);
}


BOOL WINAPI DebugActiveProcessStop(DWORD dwProcessId);

static void (*gmbn)(HANDLE, HMODULE, LPTSTR, int);
static int (*gmi)(HANDLE, HMODULE, LPMODULEINFO, int);
static BOOL WINAPI (*win32_detach)(DWORD);
static HANDLE WINAPI (*win32_openthread)(DWORD, BOOL, DWORD);
static HANDLE WINAPI (*win32_dbgbreak)(HANDLE);
static DWORD WINAPI (*win32_getthreadid)(HANDLE); // Vista
static DWORD WINAPI (*win32_getprocessid)(HANDLE); // XP

/* mark step flag (DEPRECATED) */
static int single_step;
static int exit_wait;

int arch_print_syscall()
{
	/* dummy */
}

void debug_init_calls()
{
	HANDLE lib;

	win32_detach = (BOOL WINAPI (*)(DWORD))
		GetProcAddress(GetModuleHandle("kernel32"),
				"DebugActiveProcessStop");

	win32_openthread = (HANDLE WINAPI (*)(DWORD, BOOL, DWORD))
		GetProcAddress(GetModuleHandle("kernel32"), "OpenThread");

	win32_dbgbreak = (HANDLE WINAPI (*)(HANDLE))
		GetProcAddress(GetModuleHandle("kernel32"),
				"DebugBreakProcess");

	// only windows vista :(
	win32_getthreadid = (DWORD WINAPI (*)(HANDLE))
		GetProcAddress(GetModuleHandle("kernel32"), "GetThreadId");
	// from xp1
	win32_getprocessid = (DWORD WINAPI (*)(HANDLE))  
		GetProcAddress(GetModuleHandle("kernel32"), "GetProcessId");

	lib = LoadLibrary("psapi.dll");

	if(lib == NULL) {
		eprintf("Cannot load psapi.dll!!\n");
		exit(1);
	}

	gmbn = (void (*)(HANDLE, HMODULE, LPTSTR, int))
		GetProcAddress(lib, "GetModuleBaseNameA");
	gmi = (int (*)(HANDLE, HMODULE, LPMODULEINFO, int))
		GetProcAddress(lib, "GetModuleInformation");

	if(win32_detach == NULL || win32_openthread == NULL || win32_dbgbreak == NULL || 
	   gmbn == NULL || gmi == NULL) {
		// OOPS!
		eprintf("debug_init_calls:\n"
			"DebugActiveProcessStop: 0x%x\n"
			"OpenThread: 0x%x\n"
			"DebugBreakProcess: 0x%x\n"
			"GetThreadId: 0x%x\n",
			win32_detach, win32_openthread, win32_dbgbreak, win32_getthreadid);
		exit(1);
	}
}

/* console events handler */
static BOOL dispatch_console(DWORD type)
{
	switch(type) {
		case CTRL_C_EVENT: 
			if (config_get("dbg.controlc")) {
				/* stop process */
				win32_dbgbreak(WIN32_PI(hProcess));
				exit_wait = 1;
				return 1;
			}
			break;
	}

	return 0;
}

static HANDLE tid2handler(pid_t tid)
{
	TH_INFO *th = get_th(tid);
	if(th == NULL) {
		/* refresh thread list */
		debug_load_threads();

		/* try to search thread */
		if((th = get_th(tid)) == NULL)
			return NULL;
	}

	return th->ht;
}

inline static int handler2tid(HANDLE h)
{
	if (win32_getthreadid != NULL) // >= Windows Vista
		return win32_getthreadid(h);
	return ps.tid;
	if (win32_getprocessid != NULL) // >= Windows XP1
		return win32_getprocessid(h);
	return (int)h; // XXX broken
}

inline static int handler2pid(HANDLE h)
{
	return GetProcessId(h);
}

inline int debug_attach(pid_t pid) 
{
	WIN32_PI(hProcess) = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if(WIN32_PI(hProcess) != (HANDLE)NULL && DebugActiveProcess(pid)) {

		/* set process id */
		ps.pid = pid;

		/* get last thread id */
		ps.tid = debug_load_threads();

		/* fill PROCESS_INFORMATION struct */
		WIN32_PI(dwProcessId) = ps.pid;
		WIN32_PI(dwThreadId) = ps.tid;

		return 0;
	}
	eprintf("Cannot attach\n");
	return -1;
}

inline int debug_detach()
{
	return win32_detach(ps.pid)? 0 : -1;
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

	free_th();

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

	if(ret == -1) 
		print_lasterr((char *)__FUNCTION__);

	if(th != INVALID_HANDLE_VALUE)
		CloseHandle(th);

	return ret;
}

int debug_contp(int tid)
{
	if(ContinueDebugEvent(ps.pid, tid, DBG_CONTINUE) == 0) {
		print_lasterr((char *)__FUNCTION__);
		return -1;
	}

	return 0;
}

inline int debug_os_steps()
{
	regs_t regs;

	/* up TRAP flag */
	debug_getregs(ps.tid, &regs);
	regs.EFlags |= 0x100;
	debug_setregs(ps.tid, &regs);

	single_step = ps.tid;

	/* continue */
	debug_contp(ps.tid);

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

		print_lasterr((char *)__FUNCTION__);
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
}

int debug_read_at(pid_t tid, void *buff, int len, u64 addr)
{
	int ret_len = 0;

	ReadProcessMemory(WIN32_PI(hProcess), (PCVOID)(ULONG)addr,
		       	buff, len, (PDWORD)&ret_len);

	return ret_len;
}

int debug_write_at(pid_t tid, void *buff, int len, u64 addr)
{
	int ret_len = -1;

	/* change page permissions */
	/*
	if(VirtualProtectEx(WIN32_PI(hProcess), addr, 
				len, PAGE_READWRITE, &old) == 0)

		return -1;
		*/

	/* write memory */
	WriteProcessMemory(WIN32_PI(hProcess), (LPVOID)(ULONG)addr, (LPCVOID)buff,
		       	len, (SIZE_T *)&ret_len);

	/* restore page permissions */
	/*
	VirtualProtectEx(WIN32_PI(hProcess), addr, 
				len, old, NULL);
				*/

	return ret_len;
}


int debug_getregs(pid_t tid, regs_t *regs)
{
	regs->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	return GetThreadContext(tid2handler(tid), regs)? 0 : -1; 
}

int debug_setregs(pid_t tid, regs_t *regs)
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

	case FATAL_EVENT:
		eprintf("=== %s: (%d) stop at 0x%x(FATAL)\n",
				act, ps.tid, (unsigned int)arch_pc());
		break;

	default:
		if(WS(event) != EXIT_EVENT ) {
			/* XXX: update thread list information here !!! */
			eprintf("=== %s: (%d) stop at 0x%x(%s)\n",
				act, ps.tid, (unsigned int)arch_pc(), //WS_PC() is not portable
				exit_wait? "INT" : "UNKNOWN");
		}
	}
	return 0;
}

static void debug_init_console()
{
	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)dispatch_console,
			       	TRUE ) == 0) {
		print_lasterr((char *)__FUNCTION__);
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
	struct bp_t *bp;
	int next_event;

	next_event = 0;

	switch(code) {

		/* software breakpoint or int3 */
		case EXCEPTION_BREAKPOINT:

			bp = arch_stopped_bp();
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

			bp = arch_stopped_bp();
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

		/* fatal exceptions */
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_STACK_OVERFLOW:
			WS(event) = FATAL_EVENT;
			break;

		default:
			eprintf("exception 0x%x uncatched\n", code);

	}

	return next_event;
}

int debug_dispatch_wait() 
{
	DEBUG_EVENT de;
	int next_event = 0;
	unsigned int code;

	do {
		exit_wait = 0;

		/* handle debug events */
		if(WaitForDebugEvent(&de, INFINITE) == 0) {
			print_lasterr((char *)__FUNCTION__);
			return -1;
		}

		/* save thread id */
		ps.tid = de.dwThreadId;

		/* save registers */
		debug_getregs(ps.tid, &WS(regs));

		/* get exception code */
		code = de.dwDebugEventCode;

		/* Ctrl-C? */
		if(exit_wait == 1 && code == 0x2) {
			WS(event) = INT_EVENT;
			break;
		}

		/* set state */
		WS(event) = UNKNOWN_EVENT;

		/* get kind of event */
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
			eprintf("\n\n______________[ process finished ]_______________\n\n");
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
			eprintf("Unknown event: %d\n", code);
			return -1;
		}
	} while(next_event);

	return 0;
}

static inline int CheckValidPE(unsigned char * PeHeader)
{    	    
	IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)PeHeader;
	IMAGE_NT_HEADERS *nt_headers;

	if (dos_header->e_magic==IMAGE_DOS_SIGNATURE) {
		nt_headers = (IMAGE_NT_HEADERS *)((char *)dos_header
				+ dos_header->e_lfanew);
		if (nt_headers->Signature==IMAGE_NT_SIGNATURE) 
			return 1;
	}

	return 0;

}


int debug_init_maps(int rest)
{
	SYSTEM_INFO SysInfo;
	MEMORY_BASIC_INFORMATION mbi;
	LPBYTE CurrentPage;
	char *ModuleName = NULL;
	/* DEPRECATED */
	char PeHeader[1024];
	MODULEINFO ModInfo;
	IMAGE_DOS_HEADER *dos_header;
	IMAGE_NT_HEADERS *nt_headers;
	IMAGE_SECTION_HEADER *SectionHeader;
	int NumSections, i;
	DWORD ret_len;
	MAP_REG *mr;

	if(ps.map_regs_sz > 0) 
		free_regmaps(rest);


	GetSystemInfo(&SysInfo);

	for (CurrentPage = (LPBYTE) SysInfo.lpMinimumApplicationAddress ;
		       	CurrentPage < (LPBYTE) SysInfo.lpMaximumApplicationAddress;) {

		if (!VirtualQueryEx (WIN32_PI(hProcess),CurrentPage, &mbi, sizeof (mbi)))  {
			eprintf("VirtualQueryEx ERROR, address = 0x%08X\n", CurrentPage );
			return -1;
		}

		if(mbi.Type == MEM_IMAGE) {

			 ReadProcessMemory(WIN32_PI(hProcess), (const void *)CurrentPage,
					(LPVOID)PeHeader, sizeof(PeHeader), &ret_len);

			if(ret_len == sizeof(PeHeader) && CheckValidPE(PeHeader)) {

				dos_header = (IMAGE_DOS_HEADER *)PeHeader;
				nt_headers = (IMAGE_NT_HEADERS *)((char *)dos_header
						+ dos_header->e_lfanew);    	    		    	
				NumSections = nt_headers->FileHeader.NumberOfSections;    	        	        	    

				SectionHeader = (IMAGE_SECTION_HEADER *) ((char *)nt_headers
					+ sizeof(IMAGE_NT_HEADERS));

				if(NumSections > 0) {

					ModuleName = (char *)malloc(MAX_PATH);
					if(!ModuleName) {
						perror(":map_reg alloc");
						return -1;
					}

					gmbn(WIN32_PI(hProcess), (HMODULE) CurrentPage,
						(LPTSTR)ModuleName, MAX_PATH);

					i = 0;

					do {

						mr = (MAP_REG *)malloc(sizeof(MAP_REG));
						if(!mr) {
							perror(":map_reg alloc");
							free(ModuleName);
							return -1;
						}

						mr->ini = SectionHeader->VirtualAddress
							+ (unsigned long)CurrentPage;
						mr->size = SectionHeader->Misc.VirtualSize;
						mr->end = mr->ini + mr->size;
						mr->perms = SectionHeader->Characteristics;
						mr->bin = ModuleName;
						mr->flags = 0;

						add_regmap(mr);


						SectionHeader++;
						i++;

					} while(i < NumSections);
				}    	        	        	        	    
			}

			if(gmi(WIN32_PI(hProcess), (HMODULE) CurrentPage,
					(LPMODULEINFO) &ModInfo, sizeof(MODULEINFO)) == 0)
				return 0;

			CurrentPage += ModInfo.SizeOfImage;
		} else {
			mr = (MAP_REG *)malloc(sizeof(MAP_REG));
			if(!mr) {
				perror(":map_reg alloc");
				free(mr->bin);
				return -1;
			}

			mr->ini = (unsigned long)CurrentPage;
			mr->end = mr->ini + mbi.RegionSize;
			mr->size = mbi.RegionSize;
			mr->perms = mbi.Protect;
			mr->bin = (char *)NULL;
			mr->flags = 0;

			add_regmap(mr);

			CurrentPage +=  mbi.RegionSize; 
		}
	}

	return 0;
}

