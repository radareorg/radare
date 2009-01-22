/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include "r_vm.h"

void r_vm_setup_flags(const char *zf)
{
	vm_cpu.zf = strdup(zf);
}

void r_vm_setup_cpu(const char *eip, const char *esp, const char *ebp)
{
	vm_cpu.pc = strdup(eip);
	vm_cpu.sp = strdup(esp);
	vm_cpu.bp = strdup(ebp);
}

void r_vm_setup_fastcall(const char *eax, const char *ebx, const char *ecx, const char *edx)
{
	vm_cpu.a0 = strdup(eax);
	vm_cpu.a1 = strdup(ebx);
	vm_cpu.a2 = strdup(ecx);
	vm_cpu.a3 = strdup(edx);
}

void r_vm_setup_ret(const char *eax)
{
	vm_cpu.ret = strdup(eax);
}
