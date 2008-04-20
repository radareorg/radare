/* trampoline to the gnu disassembler for mips */
#include <utils.h>
#include <string.h>
#include <radare.h>
#include <dis-asm.h>
#include "opcode/mips.h"

int mips_mode = 32;

static char str[128];
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
	return 0;
}


void hoho  (int status, bfd_vma memaddr, struct disassemble_info *info)
{

}


static void  print_address(bfd_vma address, struct disassemble_info *info)
{
	cons_printf("0x%lx", (address)); // control flags and so
	return ;
}

/* Disassembler entry point */
char *gnu_dismips(unsigned char *inst, unsigned long offset)
{
	struct disassemble_info info;
struct mips_cpu_info *arch_info =   { "loongson2f",     0,      ISA_MIPS3,      CPU_LOONGSON_2F };

	str[0] = '\0';

	Offset = (unsigned long)offset;
	//endian_memcpy(bytes, inst, 4); //
	memcpy(bytes, inst, 4); // TODO handle thumb

	/* prepare disassembler */
	memset(&info,'\0', sizeof(struct disassemble_info));
	//info.arch = CPU_LOONGSON_2F; //ARM_EXT_LOONGSON2F|ARM_EXT_V1|ARM_EXT_V4T|ARM_EXT_V5;
	info.buffer = bytes; //bytes; //&bytes;
	info.read_memory_func = &mips_buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.endian = 0;//config_get_i("cfg.endian");
	info.fprintf_func = &cons_fprintf;
	info.stream = stdout;

	//print_mips_disassembler_options(stdout);
	//parse_mips_dis_options("arch=loongson2f");

	// endian is controlled by radare
	if (print_insn_big_mips(offset, &info) == -1)
		cons_strcat("  (data)");

	return str;
}
