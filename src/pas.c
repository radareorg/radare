/* radare @ pancake @ nopcode.org */

#if 0

PAS - parse assembly strings

Specifies a basic interface for parsing assembly strings
to prepare frontends on top of it for multiple architectures.

This way we reach a faster way to write a code analysis
engine without having to twice the job of parsing the
assembly which tends to bring errors.

#endif

#include "code.h"

void pas_set_arch(const char *str)
{
}

int pas_aop_aop(const char *w0, const char *w1, const char *w2, const char *w3)
{
	if (strstr(w0,"mov")) {
		eprintf("MOVE!!\n");
	} else {
		eprintf("UNKNOWN OPCODE: %s\n", w0);
		eprintf("   ARG 0: %s\n", w1);
		eprintf("   ARG 1: %s\n", w2);
		eprintf("   ARG 2: %s\n", w3);
	}
	return 0;
}

int udis_arch_string(int arch, char *buf, int endian, u64 seek, int bytes);
struct aop_t *pas_aop(int arch, u64 seek, const char *bytes, int len)
{
	char str[64];
	char w0[32];
	char w1[32];
	char w2[32];
	char w3[32];
	udis_arch_opcode(arch , config.endian, seek, 0, 0);//len);
	udis_arch_string(arch , &str, config.endian, seek, 0);//len);
eprintf("STR: (%s)\n", str);
	w0[0]='\0';
	w1[0]='\0';
	w2[0]='\0';
	w3[0]='\0';
	sscanf(str, "%s %s,%s,%s",w0,w1,w2,w3);
	pas_aop_aop(w0,w1,w2,w3);
	return NULL;
}
