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

struct vm_reg_type vm_reg_types[] = {
	{ VMREG_BIT, "bit" },
	{ VMREG_INT64, "int64" },
	{ VMREG_INT32, "int32" },
	{ VMREG_INT16, "int16" },
	{ VMREG_INT8, "int8" },
	{ VMREG_FLOAT32, "float32" },
	{ VMREG_FLOAT64, "float64" },
	{ NULL }
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
	// TODO
}

int vm_mmu_write(u64 off, u8 *data, int len)
{
	// TODO
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

int vm_arch = -1;
int vm_init(int init)
{
	if (config.arch != vm_arch)
		init = 1;

	if (init)
		INIT_LIST_HEAD(&vm_regs);

	/* vm_dbg_arch_x86_nregs */
//	const char *arch = config_get("asm.arch");
	switch (config.arch) {
	case ARCH_X86:
		eprintf("VM: Initialized\n");
		vm_reg_add("eax", VMREG_INT32, 0);
		vm_reg_add("ebx", VMREG_INT32, 0);
		vm_reg_add("ecx", VMREG_INT32, 0);
		vm_reg_add("edx", VMREG_INT32, 0);
		vm_reg_add("esi", VMREG_INT32, 0);
		vm_reg_add("edi", VMREG_INT32, 0);
		vm_reg_add("esp", VMREG_INT32, 0);
		vm_reg_add("ebp", VMREG_INT32, 0);
		vm_reg_add("zf",  VMREG_BIT, 0);
		vm_reg_add("cf",  VMREG_BIT, 0); // ...
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
	if (str[0]=='0' && str[1]=='x')
		sscanf(str, "0x%llx", &ret);
	else
	if (str[0]>='0' && str[0]<='9')
		sscanf(str, "%lld", &ret);
	else ret = vm_reg_get(str);
	return ret;
}

int vm_eval_eq(const char *str, const char *val)
{
	for(;*str==' ';str=str+1);
	for(;*val==' ';val=val+1);

	if (*str=='[') {
		// USE MMU
		// [foo] = 33, [reg] = 33
		if (*val=='[') {
			// use mmu
		} else {
			// use mmu
		}
	} else {
		// USE REG
		// reg = [foo] , reg = 33
		if (*val=='[') {
			// use mmu
		} else {
			vm_reg_set(str, vm_get_value(val));
		}
	}
	return 0;
}

int vm_eval_single(const char *str)
{
	char *ptr, *eq;
	int len = strlen(str)+1;

	ptr = alloca(len);
	memcpy(ptr, str, len);
	
	eq = strchr(ptr, '=');
	if (eq) {
		eq[0]='\0';
		// TODO: handle +=, -=, /=, */, ...
		vm_eval_eq(ptr, eq+1);
		eq[0]='=';
	} else {
		eprintf("Unknown\n");
	}
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
#if 0
	eprintf("TODO: vm_emulate\n");
	vm_init(1);
	vm_print();
#endif
// TODO: perform asm-to-pas-eval
// TODO: evaluate string
	return n;
}

int vm_cmd_reg(const char *str)
{
	struct list_head *pos;
	for(;str&&*str==' ';str=str+1);
	if (str==NULL ||str[0]=='\0') {
		/* show all registers */
		vm_print();
	} else {
		char *ptr = strchr(str, '=');
		if (ptr) {
			/* set register value */
			ptr[0]='\0';
			vm_eval_eq(str, ptr+1);
			ptr[0]='=';
		} else {
			/* show single registers */
			cons_printf("%s = 0x%08llx\n", str, vm_reg_get(str));
		}
	}
	return 0;
}
