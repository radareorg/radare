/* trampoline to the gnu disassembler for arm */
#include <radare.h>
#include <cons.h>
#include <dis-asm.h>
#include <stdarg.h>
#include <string.h>
#include "main.h"

int arm_mode = 32;

static char str[128];
extern void cons_fprintf(FILE *stream, const char *format, ...);
static unsigned long Offset = 0;
//unsigned char *bytes = "\xe1\x2f\xff\x32";
static unsigned char bytes[4];// = "\xe1\x2f\xff\x32";

static int arm_buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	memcpy (myaddr, bytes, length);
	return 0;
}
static char *str_data = " (data)";

static int symbol_at_address(bfd_vma addr, struct disassemble_info * info)
{
	return 0;
}

static int hoho(int a, int b, int c)
{
	return 0;
}

static char *buf_global = NULL;
static void print_address(bfd_vma address, struct disassemble_info *info)
{
	char tmp[32];
	if (buf_global == NULL)
		return;
	sprintf(tmp, "0x%08llx", (u64)address);
	strcat(buf_global, tmp);
}

static void buf_fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	char *tmp;
	if (buf_global == NULL)
		return;
	va_start(ap, format);
 	tmp = alloca(strlen(format)+strlen(buf_global)+2);
	sprintf(tmp, "%s%s", buf_global, format);
	vsprintf(buf_global, tmp, ap);
	va_end(ap);
}

/* Disassembler entry point */
int gnu_disarm_str(char *str, const u8 *inst, u64 offset)
{
	struct disassemble_info info;

	str[0]='\0';
	buf_global = str;

	offset-=4; /* hacky workaround */

	Offset = (unsigned long)offset;
	//endian_memcpy(bytes, inst, 4); //
	memcpy(bytes, inst, 4); // TODO handle thumb

	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	//info.arch = ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	info.buffer = bytes; //bytes; //&bytes;
	info.read_memory_func = &arm_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.endian = config_get_i("cfg.bigendian")?0:1;
	info.fprintf_func = &buf_fprintf;
	info.stream = stdout;

	//if (print_insn_arm((unsigned long)offset, &info) == -1)
	//if (print_insn_arm((unsigned long)config_get_i("io.vaddr"), &info) == -1)
	if (print_insn_arm((unsigned long)offset, &info) == -1) {
		strcpy(str, " (data)");
		return 1;
	}

	return 0;
}
#if 0

/* Disassembler entry point */
const char *gnu_disarm(unsigned char *inst, u64 offset)
{
	struct disassemble_info info;

	str[0] = '\0';

	Offset = (unsigned long)offset;
	//endian_memcpy(bytes, inst, 4); //
	memcpy(bytes, inst, 4); // TODO handle thumb

	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	//info.arch = ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	info.buffer = bytes; //bytes; //&bytes;
	info.read_memory_func = &arm_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.endian = config_get_i("cfg.bigendian")?1:0;
	info.fprintf_func = &cons_fprintf;
	info.stream = stdout;

	//if (print_insn_arm((unsigned long)offset, &info) == -1)
	//if (print_insn_arm((unsigned long)config_get_i("io.vaddr"), &info) == -1)
//	if (print_insn_arm((unsigned long)0, &info) == -1)
//		return str_data;

	return str;
}
#endif
