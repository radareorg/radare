/* trampoline to the gnu disassembler for arm */
#include <radare.h>
#include <dis-asm.h>

int arm_mode = 32;

static char str[128];
extern void cons_fprintf(FILE *stream, const char *format, ...);
unsigned long Offset = 0;
//unsigned char *bytes = "\xe1\x2f\xff\x32";
static unsigned char bytes[4];// = "\xe1\x2f\xff\x32";

static int buffer_read_memory (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	memcpy (myaddr, bytes, length);
	return 0;
}

int symbol_at_address(bfd_vma addr, struct disassemble_info * info)
{
	return 0;
}

static int hoho(int a, int b, int c)
{
	return 0;
}

static int print_address(unsigned long address, struct disassemble_info *info)
{
	cons_printf("0x%lx", (address)); // control flags and so
	return 0;
}

/* Disassembler entry point */
char *gnu_disarm(unsigned char *inst, unsigned long offset)
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
	info.read_memory_func = &buffer_read_memory;
	info.symbol_at_address_func = &symbol_at_address;
	info.memory_error_func = &hoho;
	info.print_address_func = &print_address;
	info.endian = 0;//config_get_i("cfg.endian");
	info.fprintf_func = &cons_fprintf;
	info.stream = stdout;

	if (print_insn_arm(offset, &info) == -1)
		cons_strcat("  (data)");

	return str;
}
