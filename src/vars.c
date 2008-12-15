/* Copyleft 2009 - pancake<AT>youterm.com */


#include "main.h"
#include "list.h"

// this can be nested inside the function_t which is not defined..

#if 0
struct function_t {
	char name[128];
	int framesize;
	struct list_head ranges;
	struct list_head vars;
};
#endif

static struct list_head vars;

/* variable types */
enum {
	VAR_T_GLOBAL,
	VAR_T_LOCAL,
	VAR_T_ARG,
	VAR_T_ARGREG
};

struct var_xs_t {
	u64 addr;
	int set;
	struct list_head list;
};

struct var_t {
	int type;      /* global, local... */
	u64 addr;      /* address where it is used */
	u64 eaddr;      /* address where it is used */
	u64 delta;     /* */
	char name[128];
	char vartype[128];
	int arraysize; /* size of array var in bytes , 0 is no-array */
	struct list_head access; /* list of accesses for this var */
	struct list_head list;
};

int var_add(u64 addr, u64 eaddr, int delta, int type, const char *vartype, const char *name, int arraysize)
{
	struct var_t *var = (struct var_t *)malloc(sizeof(struct var_t));
	/* TODO: check of delta inside funframe */
	strncpy(var->name, name, sizeof(var->name));
	strncpy(var->vartype, vartype, sizeof(var->vartype));
	var->delta = delta;
	var->type = type;
	var->addr = addr;
	var->eaddr = eaddr;
	var->arraysize = arraysize;
	INIT_LIST_HEAD(&(var->access));
	list_add(&(var->list), &vars);
	return 0;
}

int var_add_access(u64 addr, int delta, int set)
{
	struct list_head *pos;
	struct var_t *v;

	list_for_each(pos, &vars) {
		v = (struct var_t *)list_entry(pos, struct var_t, list);
		if (addr>v->addr) {
			//if (!strcmp(name, v->name)) {
			if (delta == v->delta) {
				struct var_xs_t *xs = (struct var_xs_t *)malloc(sizeof(struct var_xs_t));
				xs->addr = addr;
				xs->set = set;
				/* add var access here */
				list_add(&(v->access), &xs);
				return 1;
			}
		}
	}
	return 0;
}

/* 0,0 to list all */
int var_list(u64 addr, int delta)
{
	struct list_head *pos;
	struct var_t *v;
	struct var_xs_t *x;

	list_for_each(pos, &vars) {
		v = (struct var_t *)list_entry(pos, struct var_t, list);
		if (addr == 0 || (addr >= v->addr && addr <= v->eaddr)) {
			cons_printf("0x%08llx - %0x%08llx %s %s %d\n",
				v->addr, v->eaddr, v->vartype, v->name, v->delta);
			list_for_each(pos, &v->access) {
				x = (struct var_xs_t *)list_entry(pos, struct var_xs_t, list);
				cons_printf("  0x%08llx %s\n", x->addr, x->set?"set":"get");
			}
		}
	}

	return 0;
}

int var_init()
{
	INIT_LIST_HEAD(&vars);
}

int var_cmd(const char *str)
{
	switch(*str) {
	case 'v': // frame variable
		break;
	case 'a': // stack arg
		break;
	case 'A': // fastcall arg
		break;
	}
	return 0;
}

#if 0
 -- function boundaries are used to limit variables life-cycle --
 // global vars are handled as flags??
  "CV 0x8049200 x global_counter
 // local vars
 // types: glar: g=global, l=local, a=arg, r=argreg
  Cv l d i @ 0x8048200
   /* using code analysis we can identify local var accesses */

 f.ex:
 ; Set var0 
  0x4a13c014,  mov [ebp-0x34], eax

 ; set name for variable accessed.
 Cvn counter @ 0x4a13c014

 stack frame {
   var1 { offset, size, type, name }
   var2 { offset, size, type, name }
 }

// how to track a variable 

#endif
