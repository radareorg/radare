/* Copyleft 2009 - pancake<AT>youterm.com */

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

struct var_t {
	int type;      /* global, local... */
	u64 addr;      /* address where it is used */
	u64 delta;     /* */
	int size;      /* size of var in bytes */
	char name[128];
	char fmtstr[128];  /* format string to be used with pm */
	char grepstr[128]; /* [ebp-0x34] */
	struct list_head list;
};

struct var_t var_new()
{
}

int var_init()
{
	INIT_LIST_HEAD(&vars);
}

int var_add(struct var_t *var)
{
	
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
