/* trampoline to the gnu disassembler for mips */
#include <utils.h>
#include <string.h>
#include <radare.h>
#include <dis-asm.h>
#include <stdarg.h>
#include "opcode/mips.h"

int mips_mode = 32;

extern int cons_fprintf(void *stream, const char *format, ...);
static unsigned long Offset = 0;
//unsigned char *bytes = "\xe1\x2f\xff\x32";
static unsigned char bytes[4];// = "\xe1\x2f\xff\x32";

static int mips_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
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
static void buf_fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	char *tmp , *tmp2;
	if (buf_global == NULL)
		return;
	va_start(ap, format);
 	tmp2 = alloca(strlen(format)+strlen(buf_global)+2);
	sprintf(tmp2, "%s%s", buf_global, format);
	vsprintf(buf_global, tmp2, ap);
	va_end(ap);
}

/* Disassembler entry point */
int gnu_dismips_str(char *str, const u8 *inst, ut64 offset)
{
	struct disassemble_info info;
#if 0
	struct mips_cpu_info *arch_info = {
		"loongson2f",     0,      ISA_MIPS3,      CPU_LOONGSON_2F
	};
#endif

	str[0] = '\0';
	buf_global = str;

	Offset = (unsigned long)offset;
	endian_memcpy(bytes, inst, 4); // TODO handle thumb

	//set_default_mips_dis_options(&info);
	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	info.arch = CPU_LOONGSON_2F; //ARM_EXT_LOONGSON2F|ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	info.buffer = bytes;
	info.read_memory_func = &mips_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.buffer_vma = Offset;
	info.buffer_length = 4;
	info.endian = 0;//config_get_i("cfg.bigendian");
	info.fprintf_func = &buf_fprintf;
	//info.fprintf_func = &cons_fprintf;
	info.stream = stdout;

	//print_mips_disassembler_options(stdout);
	//parse_mips_dis_options("arch=loongson2f");

	// endian is controlled by radare
	//if (print_insn_big_mips((bfd_vma)offset-4, &info) == -1)
	if (print_insn_big_mips((bfd_vma)Offset, &info) == -1) {
		strcpy(str, " (data)");
		return 1;
	}

	return 0;
}
