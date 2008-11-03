/* radare @ pancake @ nopcode.org */

#if 0

PAS - parse assembly strings

Specifies a basic interface for parsing assembly strings
to prepare frontends on top of it for multiple architectures.

This way we reach a faster way to write a code analysis
engine without having to twice the job of parsing the
assembly which tends to bring errors.

#endif

#include "main.h"
#include "code.h"

void pas_set_arch(const char *str)
{
}

#if 0
BUGS:
 - needs to run 'pd' once before working :??? why?
 - current seek is not used properly or so
#endif

/* TODO: Use vm.c from here */
int pas_emul_init()
{
	vm_init();
}

int pas_emul(const char *str)
{
	const char *ptr;
	ptr = strchr(str, '=');
	if (ptr == str)
		return -1;
	if (ptr != NULL) {
		
		
	}
}

int pas_aop_x86(int argc, const char *argv[], struct aop_t *aop, char *newstr)
{
	int i,j,k;
	struct {
		char *op;
		char *str;
		int type;
	} ops[] = {
		{ "cmp",  "cmp 1,2", AOP_TYPE_CMP },
		{ "test", "cmp 1,2", AOP_TYPE_CMP },
		{ "lea",  "1=2",     AOP_TYPE_MOV },
		{ "mov",  "1=2",     AOP_TYPE_MOV },
		{ "add",  "1+=2",    AOP_TYPE_MOV },
		{ "sub",  "1-=2",    AOP_TYPE_MOV },
		{ "mul",  "1*=2",    AOP_TYPE_MOV },
		{ "div",  "1/=2",    AOP_TYPE_MOV },
		{ "jmp",  "goto 1",  AOP_TYPE_MOV },
		{ "je",   "je 1",    AOP_TYPE_CJMP },
		{ "push", "1",       AOP_TYPE_PUSH },
		{ "pop",  "1",       AOP_TYPE_POP },
		{ NULL }
	};

	for(i=0;ops[i].op != NULL;i++) {
		if (!strcmp(ops[i].op, argv[0])) {
			/* opcode matches */
			if (aop != NULL) {
				aop->type = ops[i].type;
			}
			if (newstr != NULL) {
				for(j=k=0;ops[i].str[j]!='\0';j++,k++) {
					if (ops[i].str[j]>='0' && ops[i].str[j]<='9') {
						const char *w = argv[ ops[i].str[j]-'0' ];
						if (w != NULL) {
							strcpy(newstr+k, w);
							k += strlen(w)-1;
						}
					} else newstr[k] = ops[i].str[j];
				}
				newstr[k]='\0';
			}
			return 1;
		}
	}

	if (aop != NULL) {
		/* XXX : we need bytes */
	}

	if (newstr != NULL) {
		if (argv[0]) strcpy(newstr, argv[0]);
		if (argv[1]) strcpy(newstr, " ");
		if (argv[1]) strcpy(newstr, argv[1]);
		if (argv[2]) strcpy(newstr, ",");
		if (argv[2]) strcpy(newstr, argv[2]);
		if (argv[3]) strcpy(newstr, ",");
		if (argv[3]) strcpy(newstr, argv[3]);
	}
	return 0;
}

int pas_aop_aop(int argc, const char *argv[], struct aop_t *aop, char *newstr)
{
	return pas_aop_x86(argc, argv, aop, newstr);
}

struct aop_t *pas_aop(int arch, u64 seek, const char *bytes, int len, struct aop_t *aop, char *newstr)
{
	int i;
	char str[64];
	char w0[32];
	char w1[32];
	char w2[32];
	char w3[32];
	char *ptr, *optr;

	udis_arch_string(arch, str, bytes, config.endian, seek, 0, 0);

	if (str[0]!='\0') {
		w0[0]='\0';
		w1[0]='\0';
		w2[0]='\0';
		w3[0]='\0';
		ptr = strchr(str, ' ');
		if (ptr == NULL)
			ptr = strchr(str, '\t');
		if (ptr) {
			ptr[0]='\0';
			for(ptr=ptr+1;ptr[0]==' ';ptr=ptr+1);
			strcpy(w0, str);
			strcpy(w1, ptr);

			optr=ptr;
			ptr = strchr(ptr, ',');
			if (ptr) {
				ptr[0]='\0';
				for(ptr=ptr+1;ptr[0]==' ';ptr=ptr+1);
				strcpy(w1, optr);
				strcpy(w2, ptr);
				ptr = strchr(ptr, ',');
				if (ptr) {
					strcpy(w3, ptr);
				}
			}
		}
		{
			const char *wa[] = { &w0, &w1, &w2, &w3 };
			struct aop_t *aop;
			int nw=0;

			for(i=0;i<4;i++) {
				if (wa[i][0] != '\0')
				nw++;
			}
			pas_aop_aop(nw, wa, aop, newstr);
		}
	}
	return NULL;
}
