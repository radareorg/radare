/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
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

#include "radare.h"
#include "code.h"
#include "list.h"
#include "vm.h"

struct list_head vm_regs;
struct vm_cpu_t vm_cpu;

struct vm_reg_type vm_reg_types[] = {
	{ VMREG_BIT, "bit" },
	{ VMREG_INT64, "int64" },
	{ VMREG_INT32, "int32" },
	{ VMREG_INT16, "int16" },
	{ VMREG_INT8, "int8" },
	{ VMREG_FLOAT32, "float32" },
	{ VMREG_FLOAT64, "float64" },
	{ 0, NULL }
};

static char *unkreg="(unk)";
const char *vm_reg_type(int type)
{
	struct vm_reg_type *p = vm_reg_types;
	while(p) {
		if (p->type == type)
			return p->str;
		p++;
	}
	return unkreg;
}

const int vm_reg_type_i(const char *str)
{
	struct vm_reg_type *p = vm_reg_types;
	while(p) {
		if (!strcmp(str, p->str))
			return p->type;
		p++;
	}
	return -1;
}

void vm_print()
{
	int i;
	struct list_head *pos;

	list_for_each(pos, &vm_regs) {
		struct vm_reg_t *r = list_entry(pos, struct vm_reg_t, list);
		cons_printf("%s %s 0x%08llx\n",
			r->name, vm_reg_type(r->type), r->value);
	}
}

int vm_mmu_read(u64 off, u8 *data, int len)
{
	return radare_read_at(off, data, len);
}

int vm_mmu_write(u64 off, u8 *data, int len)
{
	return radare_write_at(off, data, len);
}

int vm_reg_add(const char *name, int type, u64 value)
{
	struct vm_reg_t *r;

	r = (struct vm_reg_t *)malloc(sizeof (struct vm_reg_t));
	if (r == NULL)
		return 0;
	strncpy(r->name, name, 15);
	r->type = type;
	r->value = value;
	list_add_tail(&(r->list), &vm_regs);
	return 1;
}


u64 vm_reg_get(const char *name)
{
	struct list_head *pos;

	list_for_each(pos, &vm_regs) {
		struct vm_reg_t *r = list_entry(pos, struct vm_reg_t, list);
		if (!strcmp(name, r->name))
			return r->value;
	}
	return -1LL;
}

int vm_reg_del(const char *name)
{
	struct list_head *pos;

	list_for_each(pos, &vm_regs) {
		struct vm_reg_t *r = list_entry(pos, struct vm_reg_t, list);
		if (!strcmp(name, r->name)) {
			list_del(&r->list);
			return 0;
		}
	}
	return 1;
}

int vm_reg_set(const char *name, u64 value)
{
	struct list_head *pos;

	list_for_each(pos, &vm_regs) {
		struct vm_reg_t *r = list_entry(pos, struct vm_reg_t, list);
		if (!strcmp(name, r->name)) {
			r->value = value;
			return 1;
		}
	}
	return 0;
}

int vm_import()
{
	struct list_head *pos;

	list_for_each(pos, &vm_regs) {
		struct vm_reg_t *r = list_entry(pos, struct vm_reg_t, list);
		r->value = get_offset(r->name); // XXX doesnt work for eflags and so
	}
	return 0;
}


void vm_configure_cpu(const char *eip, const char *esp, const char *ebp)
{
	vm_cpu.pc = strdup(eip);
	vm_cpu.sp = strdup(esp);
	vm_cpu.bp = strdup(ebp);
}

void vm_configure_fastcall(const char *eax, const char *ebx, const char *ecx, const char *edx)
{
	vm_cpu.a0 = strdup(eax);
	vm_cpu.a1 = strdup(ebx);
	vm_cpu.a2 = strdup(ecx);
	vm_cpu.a3 = strdup(edx);
}

void vm_configure_ret(const char *eax)
{
	vm_cpu.ret = strdup(eax);
}

void vm_cpu_call(u64 addr)
{
	/* x86 style */
}

static u64 vm_stack_base = 0;
static u8 *vm_stack = NULL;

void vm_stack_push(u32 val)
{
	// XXX do not write while emulating zomfg
	vm_reg_set(vm_cpu.sp, vm_reg_get(vm_cpu.sp)+4);
	vm_mmu_write(vm_reg_get(vm_cpu.sp), &val, 4);
}

void vm_stack_pop(const char *reg)
{
	u32 val = 0;
	vm_mmu_read(vm_reg_get(vm_cpu.sp), &val, 4);
	vm_reg_set(reg, val);
	vm_reg_set(vm_cpu.sp, vm_reg_get(vm_cpu.sp)-4);
}

int vm_arch = -1;
int vm_init(int init)
{
	if (config.arch != vm_arch)
		init = 1;

	if (init) {
		INIT_LIST_HEAD(&vm_regs);
		memset(&vm_cpu, '\0', sizeof(struct vm_cpu_t));
	}

	/* vm_dbg_arch_x86_nregs */
//	const char *arch = config_get("asm.arch");
	switch (config.arch) {
	case ARCH_X86:
		//eprintf("VM: Initialized\n");
		vm_reg_add("eax", VMREG_INT32, 0);
		vm_reg_add("ebx", VMREG_INT32, 0);
		vm_reg_add("ecx", VMREG_INT32, 0);
		vm_reg_add("edx", VMREG_INT32, 0);
		vm_reg_add("esi", VMREG_INT32, 0);
		vm_reg_add("edi", VMREG_INT32, 0);
		vm_reg_add("eip", VMREG_INT32, 0);
		vm_reg_add("esp", VMREG_INT32, 0);
		vm_reg_add("ebp", VMREG_INT32, 0);
		vm_reg_add("zf",  VMREG_BIT, 0);
		vm_reg_add("cf",  VMREG_BIT, 0); // ...
		vm_reg_add("cf",  VMREG_BIT, 0); // ...

		vm_configure_cpu("eip", "esp", "ebp");
		//vm_configure_call("[ebp-4]", "[ebp-8]", "[ebp-12]", "edx");
		vm_configure_fastcall("eax", "ebx", "ecx", "edx");
		//vm_configure_loop("ecx");
		//vm_configure_copy("esi", "edi");
		vm_configure_ret("eax");
		// TODO: do the same for fpregs and mmregs
		if (init) // XXX
			vm_arch_x86_init();
		break;
	case ARCH_MIPS:
#if 0
		vm_nregs    = vm_arch_mips_nregs;
		vm_regs     = vm_arch_mips_regs;
		vm_regs_str = vm_arch_mips_regs_str;
#endif
		// TODO: do the same for fpregs and mmregs
		if (init)
			vm_arch_mips_init();
		break;
		//vm_regs = NULL;
	}
	return 0;
}

static u64 vm_get_value(const char *str)
{
	u64 ret = 0LL;
	for(;*str&&*str==' ';str=str+1);

	if (str[0]=='0' && str[1]=='x')
		sscanf(str, "0x%llx", &ret);
	else
	if (str[0]>='0' && str[0]<='9')
		sscanf(str, "%lld", &ret);
	else ret = vm_reg_get(str);
	return ret;
}

static u64 vm_get_math(const char *str)
{
	int len;
	char *p,*a;

	len = strlen(str)+1;
	p=alloca(len);
	memcpy(p, str, len);
	a = strchr(p,'+');
	if (a) {
		*a='\0';
		return vm_get_value(str) + vm_get_value(a+1);
	}
	a = strchr(p,'-');
	if (a) {
		*a='\0';
		return vm_get_value(str) - vm_get_value(a+1);
	}
	a = strchr(p,'*');
	if (a) {
		*a='\0';
		return vm_get_value(str) * vm_get_value(a+1);
	}
	a = strchr(p,'/');
	if (a) {
		*a='\0';
		return vm_get_value(str) / vm_get_value(a+1);
	}
	a = strchr(p,'&');
	if (a) {
		*a='\0';
		return vm_get_value(str) & vm_get_value(a+1);
	}
	a = strchr(p,'|');
	if (a) {
		*a='\0';
		return vm_get_value(str) | vm_get_value(a+1);
	}
	a = strchr(p,'^');
	if (a) {
		*a='\0';
		return vm_get_value(str) ^ vm_get_value(a+1);
	}
	a = strchr(p,'%');
	if (a) {
		*a='\0';
		return vm_get_value(str) % vm_get_value(a+1);
	}
	return vm_get_value(str);
}

int vm_eval_eq(const char *str, const char *val)
{
	for(;*str==' ';str=str+1);
	for(;*val==' ';val=val+1);

	if (*str=='[') {
		// USE MMU
		// [foo] = 33, [reg] = 33
		if (*val=='[') {
			// TODO: support for size 8:addr
			// if (strchr(val, ':')) ..
			u8 data[8];
			u64 off = vm_get_math(val+1);
			u32 _int32 = 0;

			vm_mmu_read(off, (u8*)&_int32, 4);
			off = vm_get_math(str+1);
			vm_mmu_write(off, (u8*)&_int32, 4);
		} else {
			// use ssssskcvtgvmu
			u64 off = vm_get_math(str+1);
			u32 v = (u32)vm_get_math(val); // TODO control endian
			vm_mmu_write(off, (u8*)&v, 4);
		}
	} else {
		// USE REG
		// reg = [foo] , reg = 33
		if (*val=='[') {
			// use mmu
			u8 data[8];
			u64 off = vm_get_math(val+1);
			u32 _int32 = 0;
			vm_mmu_read(off, (u8*)&_int32, 4);
			vm_reg_set(str, (u64)_int32);
		} else {
			vm_reg_set(str, vm_get_math(val));
		}
	}
	return 0;
}

int vm_eval_single(const char *str)
{
	char *ptr, *eq;
	char buf[128];
	int i, len;

	for(;str&&str[0]==' ';str=str+1);
	len = strlen(str)+1;
	ptr = alloca(len);
	memcpy(ptr, str, len);
	
	eq = strchr(ptr, '=');
	if (eq) {
		eq[0]='\0';
		switch(eq[-1]) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '&':
		case '^':
		case '%':
		case '|':
			snprintf(buf, 127, "%s%c%s", ptr, eq[-1], eq+1);
			vm_eval_eq(ptr, buf);
			break;
		case ' ':
			i=-1;
			do { eq[i--]='\0';
			} while(eq[i]==' ');
		default:
			vm_eval_eq(ptr, eq+1);
		}
		eq[0]='=';
	} else {
		if (!memcmp(ptr, "syscall", 6)) {
			eprintf("TODO\n");
		} else
		if((!memcmp(ptr, "call ", 4))
		|| (!memcmp(ptr, "jmp ", 4))){
			if (ptr[0]=='c')
				vm_stack_push(vm_cpu.pc);
			printf("CALL(%s)\n", ptr+4);
			vm_reg_set(vm_cpu.pc, vm_get_value(ptr+4));
		} else
		if (!memcmp(ptr, "push ", 5)) {
			vm_stack_push(str+5);
		} else
		if (!memcmp(str, "pop ", 4)) {
			vm_stack_pop(str+5);
		} else
			eprintf("Unknown opcode\n");
	}
	return 0;
}

int vm_eval(const char *str)
{
	char *next, *ptr, *p;
	int ret, len = strlen(str)+1;

	ptr = alloca(len);
	memcpy(ptr, str, len);

	next = strchr(ptr, ',');
	if (next) {
		next[0]='\0';
		ret = vm_eval_single(str);
		if (ret == -1)
			return 0;
		vm_eval(ptr+1);
		next[0]=',';
	} else
		return vm_eval_single(str);
	return 1;
}

/* emulate n opcodes */
int vm_emulate(int n)
{
	u64 pc;
	char str[128];
	u8 buf[128];
	int opsize;
	int op = config_get_i("asm.pseudo");
	struct aop_t aop;

	printf("Emulating %d opcodes\n", n);
	radare_cmd("avi", 0);
	config_set("asm.pseudo", "true");
	config_set("asm.syntax", "intel");
	udis_init();
	while(n--) {
		pc = vm_reg_get(vm_cpu.pc);
		udis_set_pc(pc);
		vm_mmu_read(pc, buf, 32);
		pas_aop(config.arch, pc, buf, 32, &aop, str, 1);
		opsize = aop.length;
		printf("=> 0x%08llx '%s' (%d)\n", vm_reg_get(vm_cpu.pc), str, opsize);
		vm_reg_set(vm_cpu.pc, vm_reg_get(vm_cpu.pc)+opsize);
		vm_eval(str);
	}
	config_set("asm.pseudo", op?"true":"false");
	
#if 0
	eprintf("TODO: vm_emulate\n");
	vm_init(1);
	vm_print();
#endif
// TODO: perform asm-to-pas-eval
// TODO: evaluate string
	return n;
}

int vm_cmd_reg(const char *_str)
{
	struct list_head *pos;
	char *str, *ptr;
	int len;

	for(;_str&&*_str==' ';_str=_str+1);
	len = strlen(_str)+1;
	str = alloca(len);
	memcpy(str, _str, len);

	if (str==NULL ||str[0]=='\0') {
		/* show all registers */
		vm_print();
	} else {
		switch(str[0]) {
		case '+':
			// add register
			// avr+ eax int32
			for(str=str+1;str&&*str==' ';str=str+1);
			ptr = strchr(str, ' ');
			if (ptr) {
				ptr[0]='\0';
				vm_reg_add(str, vm_reg_type_i(ptr+1), 0);
			} else vm_reg_add(str, VMREG_INT32, 0);
			break;
		case '-':
			// rm register
			// avr- eax
			// avr-*
			for(str=str+1;str&&*str==' ';str=str+1);
			if (str[0]=='*')
				INIT_LIST_HEAD(&vm_regs); // XXX Memory leak
			else vm_reg_del(str);
			break;
		default:
			ptr = strchr(str, '=');
			if (ptr) {
				vm_eval(str);
	#if 0
				/* set register value */
				ptr[0]='\0';
				vm_eval_eq(str, ptr+1);
				ptr[0]='=';
	#endif
			} else {
				/* show single registers */
				cons_printf("%s = 0x%08llx\n", str, vm_reg_get(str));
			}
		}
	}
	return 0;
}
