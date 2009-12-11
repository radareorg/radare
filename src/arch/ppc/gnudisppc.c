/* trampoline to the gnu disassembler for mips */
#include <utils.h>
#include <string.h>
#include <radare.h>
#include <dis-asm.h>
#include <stdarg.h>
#include "opcode/ppc.h"

extern int cons_fprintf(void *stream, const char *format, ...);
static unsigned long Offset = 0;
//unsigned char *bytes = "\xe1\x2f\xff\x32";
static unsigned char bytes[4];// = "\xe1\x2f\xff\x32";

static int ppc_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	memcpy (myaddr, bytes, length);
	return 0;
}

static int symbol_at_address(bfd_vma addr, struct disassemble_info * info)
{
	eprintf("symataddr%llx\n",(ut32)addr);
	return 0;
}

static void hoho  (int status, bfd_vma memaddr, struct disassemble_info *info)
{
	eprintf("hoho%llx\n",(ut32)memaddr);
}

static char *buf_global = NULL;
static void print_address(bfd_vma address, struct disassemble_info *info)
{
	char tmp[32];
	if (buf_global == NULL)
		return;
	sprintf(tmp, "0x%08llx", (ut64)address-8); /* WTF ?!?! why gnu disasm doesnt do this well? */
	strcat(buf_global, tmp);
}
static int buf_fprintf(void *stream, const char *format, ...)
{
	int ret = 0;
	va_list ap;
	char *tmp;
	if (buf_global != NULL) {
		va_start(ap, format);
		tmp = alloca(strlen(format)+strlen(buf_global)+2);
		sprintf(tmp, "%s%s", buf_global, format);
		ret = vsprintf(buf_global, tmp, ap);
		va_end(ap);
	}
	return ret;
}

/* Disassembler entry point */
int gnu_disppc_str(char *str, const u8 *inst, ut64 offset)
{
	struct disassemble_info info;

	str[0] = '\0';
	buf_global = str;

	Offset = (unsigned long)offset;
	endian_memcpy(bytes, inst, 4); // TODO handle thumb

	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	info.buffer = bytes;
	info.read_memory_func = &ppc_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.buffer_vma = Offset;
	info.buffer_length = 4;
	info.endian = 0;//config_get_i("cfg.bigendian");
	info.fprintf_func = buf_fprintf;
	//info.fprintf_func = &cons_fprintf;
	info.stream = stdout;


	// endian is controlled by radare
	if (print_insn_big_powerpc((bfd_vma)Offset, &info) == -1) {
		strcpy(str, " (data)");
		return 1;
	}

	return 0;
}
