/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include "r_vm.h"

static char *unkreg = "(unk)";

/* static */
static struct r_vm_reg_type r_vm_reg_types[] = {
	{ VMREG_BIT, "bit" },
	{ VMREG_INT64, "int64" },
	{ VMREG_INT32, "int32" },
	{ VMREG_INT16, "int16" },
	{ VMREG_INT8, "int8" },
	{ VMREG_FLOAT32, "float32" },
	{ VMREG_FLOAT64, "float64" },
	{ 0, NULL }
};

void r_vm_reg_type_list()
{
	struct r_vm_reg_type *p = r_vm_reg_types;
	while(p) {
		if (p->str==NULL)
			break;
		r_cons_printf(" .%s\n", p->str);
		p++;
	}
}

const char *r_vm_reg_type(int type)
{
	struct r_vm_reg_type *p = r_vm_reg_types;
	while(p) {
		if (p->type == type)
			return p->str;
		p++;
	}
	return unkreg;
}

const int r_vm_reg_type_i(const char *str)
{
	struct r_vm_reg_type *p = r_vm_reg_types;
	while(p) {
		if (!strcmp(str, p->str))
			return p->type;
		p++;
	}
	return -1;
}

int r_vm_reg_del(struct r_vm_t *vm, const char *name)
{
	struct list_head *pos;

	list_for_each(pos, &vm->regs) {
		struct r_vm_reg_t *r = list_entry(pos, struct r_vm_reg_t, list);
		if (!strcmp(name, r->name)) {
			list_del(&r->list);
			return 0;
		}
	}
	return 1;
}

int r_vm_reg_set(const char *name, u64 value)
{
	struct list_head *pos;

	list_for_each(pos, &vm->regs) {
		struct r_vm_reg_t *r = list_entry(pos, struct r_vm_reg_t, list);
		if (!strcmp(name, r->name)) {
			r->value = value;
			if (rec == NULL && r->set != NULL) {
				rec = r;
				vm_eval(r->set);
				rec = NULL;
			}
			return 1;
		}
	}
	return 0;
}
