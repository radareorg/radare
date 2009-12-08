/* trampoline to the gnu disassembler for sparc */
#include <utils.h>
#include <string.h>
#include <radare.h>
#include <dis-asm.h>
#include "opcode/sparc.h"
#include <stdarg.h>

int sparc_mode = 32;

extern int cons_fprintf(void *stream, const char *format, ...);
static unsigned long Offset = 0;
//unsigned char *bytes = "\xe1\x2f\xff\x32";
static unsigned char bytes[4];// = "\xe1\x2f\xff\x32";

static int sparc_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	memcpy (myaddr, bytes, length);
	return 0;
}

static int symbol_at_address(bfd_vma addr, struct disassemble_info * info)
{
	return 0;
}


static void hoho  (int status, bfd_vma memaddr, struct disassemble_info *info)
{

}

static char *buf_global = NULL;
static void print_address(bfd_vma address, struct disassemble_info *info)
{
	va_list ap;
	char tmp[32];
	if (buf_global == NULL)
		return;
	sprintf(tmp, "0x%08llx", (ut64)address);
	strcat(buf_global, tmp);
}
static int buf_fprintf(void *stream, const char *format, ...)
{
	int ret = 0;
	va_list ap;
	char *tmp , *tmp2;
	if (buf_global != NULL) {
		va_start(ap, format);
		tmp2 = alloca(strlen(format)+strlen(buf_global)+2);
		sprintf(tmp2, "%s%s", buf_global, format);
		ret = vsprintf(buf_global, tmp2, ap);
		va_end(ap);
	}
	return ret;
}

/* Disassembler entry point */
int gnu_disparc_str(char *str, const u8 *inst, ut64 offset)
{
	struct disassemble_info info;

	str[0] = '\0';

	Offset = (unsigned long)offset;
	//endian_memcpy(bytes, inst, 4); //
	endian_memcpy(bytes, inst, 4); // TODO handle thumb
	buf_global = str;

	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	//info.arch = CPU_LOONGSON_2F; //ARM_EXT_LOONGSON2F|ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	info.buffer = bytes; //bytes; //&bytes;
	info.read_memory_func = &sparc_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.buffer_vma = offset;
	info.print_address_func = &print_address;
	info.endian = 0;//config_get_i("cfg.bigendian");
	info.fprintf_func = &buf_fprintf;
	info.stream = stdout;

	//print_sparc_disassembler_options(stdout);
	//parse_sparc_dis_options("arch=loongson2f");

	// endian is controlled by radare
	if (print_insn_sparc (offset, &info) == -1) {
		cons_strcat("  (data)");
		return 1;
	}

	return 0;
}
