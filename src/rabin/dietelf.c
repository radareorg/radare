/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../main.h"
#if __UNIX__
#include <sys/mman.h>
#endif

#include "aux.h"
#include "elf.h"
#include "dietelf.h"
#include "dietelf_static.h"
#include "dietelf_types.h"

static int endian = 0;

static void ELF_(aux_swap_endian)(u8 *value, int size)
{
	unsigned char buffer[8];

	if (endian) {
		switch(size) {
			case 2:
				memcpy(buffer, value, 2);
				value[0] = buffer[1];
				value[1] = buffer[0];
				break;
			case 4:
				memcpy(buffer, value, 4);
				value[0] = buffer[3];
				value[1] = buffer[2];
				value[2] = buffer[1];
				value[3] = buffer[0];
				break;
			case 8:
				memcpy(buffer, value, 8);
				value[0] = buffer[7];
				value[1] = buffer[6];
				value[2] = buffer[5];
				value[3] = buffer[4];
				value[4] = buffer[3];
				value[5] = buffer[2];
				value[6] = buffer[1];
				value[7] = buffer[0];
				break;
			default:
				printf("Invalid size: %d\n", size);
		}
	}
}

static int ELF_(aux_stripstr_from_file)(const char *filename, int min, int encoding, u64 seek, u64 limit, const char *filter, int str_limit, dietelf_string *strings)
{
	int fd = open(filename, O_RDONLY);
	dietelf_string *stringsp;
	unsigned char *buf;
	u64 i = seek;
	u64 len, string_len;
	int unicode = 0, matches = 0;
	static int ctr = 0;
	char str[ELF_STRING_LENGTH];

	if (fd == -1) {
		fprintf(stderr, "Cannot open target file.\n")    ;
		return 1;
	}

	len = lseek(fd, 0, SEEK_END);

	// TODO: use read here ?!?
	/* TODO: do not use mmap */
#if __UNIX__
	buf = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
	if (((int)buf) == -1 ) {
		perror("mmap");
		return 1;
	}
	if (min <1)
		min = 5;

	if (limit && limit < len)
		len = limit;

	stringsp = strings;
	for(i = seek; i < len && ctr < str_limit; i++) { 
		if ((aux_is_printable(buf[i]) || (aux_is_encoded(encoding, buf[i])))) {
			str[matches] = buf[i];
			if (matches < sizeof(str))
				matches++;
		} else {
			/* wide char check \x??\x00\x??\x00 */
			if (matches && buf[i+2]=='\0' && buf[i]=='\0' && buf[i+1]!='\0') {
				unicode = 1;
			}
			/* check if the length fits on our request */
			if (matches >= min) {
				str[matches] = '\0';
				string_len = strlen(str);
				if (string_len>2) {
					if (!filter || strstr(str, filter)) {
						stringsp->offset = i-matches;
						stringsp->type = (unicode?'U':'A');
						stringsp->size = string_len;
						memcpy(stringsp->string, str, ELF_STRING_LENGTH);
						stringsp->string[ELF_STRING_LENGTH-1] = '\0';
						ctr++; stringsp++;
					}
				}
			}
			matches = 0;
			unicode = 0;
		}
	}

	munmap(buf, len); 
#elif __WINDOWS__
	fprintf(stderr, "Not yet implemented\n");
#endif

	return ctr;
}

int ELF_(dietelf_close)(int fd) {
	close(fd);

	return 0;
}

static int ELF_(do_elf_checks)(ELF_(dietelf_bin_t) *bin)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;

	if (strncmp((char *)ehdr->e_ident, ELFMAG, SELFMAG)) {
		fprintf(stderr, "File not ELF\n");
		return -1;
	}

	if (ehdr->e_version != EV_CURRENT) {
		fprintf(stderr, "ELF version not current\n");
		return -1;
	}

	return 0;
}

char* ELF_(dietelf_get_arch)(ELF_(dietelf_bin_t) *bin)
{
	u16 machine = bin->ehdr.e_machine;


	switch (machine) {
		case EM_MIPS:
		case EM_MIPS_RS3_LE:
		case EM_MIPS_X:
			return "mips";
		case EM_ARM:
			return "arm";
		case EM_SPARC:
		case EM_SPARC32PLUS:
		case EM_SPARCV9:
			return "sparc";
		case EM_PPC:
		case EM_PPC64:
			return "ppc";
		case EM_68K:
			return "m68k";
		case EM_IA_64:
		case EM_X86_64:
			return "intel64";
		default: return "intel";
	}
}

u64 ELF_(dietelf_get_base_addr)(ELF_(dietelf_bin_t) *bin)
{
	return bin->phdr->p_vaddr & ELF_ADDR_MASK;
}

u64 ELF_(dietelf_get_entry_offset)(ELF_(dietelf_bin_t) *bin)
{
	return bin->ehdr.e_entry - bin->base_addr; 
}

int ELF_(dietelf_get_stripped)(ELF_(dietelf_bin_t) *bin)
{
	int i;
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++)
		if (shdrp->sh_type == SHT_SYMTAB)
			return 0;
	return 1;
}

int ELF_(dietelf_get_static)(ELF_(dietelf_bin_t) *bin)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Phdr) *phdr = bin->phdr, *phdrp;
	int i;

	phdrp = phdr;
	for (i = 0; i < ehdr->e_phnum; i++, phdrp++)
		if (phdrp->p_type == PT_INTERP)
			return 0;

	return 1;
}

char* ELF_(dietelf_get_data_encoding)(ELF_(dietelf_bin_t) *bin)
{
	unsigned int encoding = bin->ehdr.e_ident[EI_DATA];
	static char buff[32];

	switch (encoding) {
		case ELFDATANONE: return "none";
		case ELFDATA2LSB: return "2's complement, little endian";
		case ELFDATA2MSB: return "2's complement, big endian";
		default:
		  snprintf (buff, sizeof (buff), "<unknown: %x>", encoding);
		  return buff;
	}
}

char* ELF_(dietelf_get_machine_name)(ELF_(dietelf_bin_t) *bin)
{
	unsigned int e_machine = bin->ehdr.e_machine;
	static char buff[64]; 

	switch (e_machine) {
		case EM_NONE: 			return "No machine";
		case EM_M32: 			return "AT&T WE 32100";
		case EM_SPARC: 			return "SUN SPARC";
		case EM_386: 			return "Intel 80386";
		case EM_68K: 			return "Motorola m68k family";
		case EM_88K: 			return "Motorola m88k family";
		case EM_860: 			return "Intel 80860";
		case EM_MIPS: 			return "MIPS R3000 big-endian";
		case EM_S370: 			return "IBM System/370";
		case EM_MIPS_RS3_LE: 	return "MIPS R3000 little-endian";
		case EM_PARISC: 		return "HPPA";
		case EM_VPP500: 		return "Fujitsu VPP500";
		case EM_SPARC32PLUS: 	return "Sun's \"v8plus\"";
		case EM_960: 			return "Intel 80960";
		case EM_PPC: 			return "PowerPC";
		case EM_PPC64: 			return "PowerPC 64-bit";
		case EM_S390: 			return "IBM S390";
		case EM_V800: 			return "NEC V800 series";
		case EM_FR20: 			return "Fujitsu FR20";
		case EM_RH32: 			return "TRW RH-32";
		case EM_RCE: 			return "Motorola RCE";
		case EM_ARM: 			return "ARM";
		case EM_FAKE_ALPHA: 	return "Digital Alpha";
		case EM_SH: 			return "Hitachi SH";
		case EM_SPARCV9: 		return "SPARC v9 64-bit";
		case EM_TRICORE: 		return "Siemens Tricore";
		case EM_ARC: 			return "Argonaut RISC Core";
		case EM_H8_300: 		return "Hitachi H8/300";
		case EM_H8_300H: 		return "Hitachi H8/300H";
		case EM_H8S: 			return "Hitachi H8S";
		case EM_H8_500: 		return "Hitachi H8/500";
		case EM_IA_64: 			return "Intel Merced";
		case EM_MIPS_X: 		return "Stanford MIPS-X";
		case EM_COLDFIRE: 		return "Motorola Coldfire";
		case EM_68HC12: 		return "Motorola M68HC12";
		case EM_MMA: 			return "Fujitsu MMA Multimedia Accelerator";
		case EM_PCP: 			return "Siemens PCP";
		case EM_NCPU: 			return "Sony nCPU embeeded RISC";
		case EM_NDR1: 			return "Denso NDR1 microprocessor";
		case EM_STARCORE:	 	return "Motorola Start*Core processor";
		case EM_ME16: 			return "Toyota ME16 processor";
		case EM_ST100: 			return "STMicroelectronic ST100 processor";
		case EM_TINYJ: 			return "Advanced Logic Corp. Tinyj emb.fam";
		case EM_X86_64: 		return "AMD x86-64 architecture";
		case EM_PDSP: 			return "Sony DSP Processor";
		case EM_FX66: 			return "Siemens FX66 microcontroller";
		case EM_ST9PLUS: 		return "STMicroelectronics ST9+ 8/16 mc";
		case EM_ST7: 			return "STmicroelectronics ST7 8 bit mc";
		case EM_68HC16: 		return "Motorola MC68HC16 microcontroller";
		case EM_68HC11: 		return "Motorola MC68HC11 microcontroller";
		case EM_68HC08: 		return "Motorola MC68HC08 microcontroller";
		case EM_68HC05: 		return "Motorola MC68HC05 microcontroller";
		case EM_SVX: 			return "Silicon Graphics SVx";
		case EM_ST19: 			return "STMicroelectronics ST19 8 bit mc";
		case EM_VAX: 			return "Digital VAX";
		case EM_CRIS: 			return "Axis Communications 32-bit embedded processor";
		case EM_JAVELIN: 		return "Infineon Technologies 32-bit embedded processor";
		case EM_FIREPATH:		return "Element 14 64-bit DSP Processor";
		case EM_ZSP: 			return "LSI Logic 16-bit DSP Processor";
		case EM_MMIX: 			return "Donald Knuth's educational 64-bit processor";
		case EM_HUANY: 			return "Harvard University machine-independent object files";
		case EM_PRISM: 			return "SiTera Prism";
		case EM_AVR: 			return "Atmel AVR 8-bit microcontroller";
		case EM_FR30: 			return "Fujitsu FR30";
		case EM_D10V: 			return "Mitsubishi D10V";
		case EM_D30V: 			return "Mitsubishi D30V";
		case EM_V850: 			return "NEC v850";
		case EM_M32R: 			return "Mitsubishi M32R";
		case EM_MN10300: 		return "Matsushita MN10300";
		case EM_MN10200: 		return "Matsushita MN10200";
		case EM_PJ: 			return "picoJava";
		case EM_OPENRISC:		return "OpenRISC 32-bit embedded processor";
		case EM_ARC_A5: 		return "ARC Cores Tangent-A5";
		case EM_XTENSA: 		return "Tensilica Xtensa Architecture";
		default:
								snprintf (buff, sizeof (buff), "<unknown>: 0x%x", e_machine);
								return buff;
	}
}

char* ELF_(dietelf_get_file_type)(ELF_(dietelf_bin_t) *bin)
{
	unsigned int e_type = bin->ehdr.e_type;
	static char buff[32];

	switch (e_type) {
		case ET_NONE:	return "NONE (None)";
		case ET_REL:	return "REL (Relocatable file)";
		case ET_EXEC:	return "EXEC (Executable file)";
		case ET_DYN:	return "DYN (Shared object file)";
		case ET_CORE:	return "CORE (Core file)";

		default:
						if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
							snprintf (buff, sizeof (buff), "Processor Specific: (%x)", e_type);
						else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
							snprintf (buff, sizeof (buff), "OS Specific: (%x)", e_type);
						else
							snprintf (buff, sizeof (buff), "<unknown>: %x", e_type);
						return buff;
	}
}

char* ELF_(dietelf_get_elf_class)(ELF_(dietelf_bin_t) *bin)
{
	unsigned int elf_class = bin->ehdr.e_ident[EI_CLASS];
	static char buff[32];

	switch (elf_class) {
		case ELFCLASSNONE: return "none";
		case ELFCLASS32:   return "ELF32";
		case ELFCLASS64:   return "ELF64";
		default:
						   snprintf (buff, sizeof (buff), "<unknown: %x>", elf_class);
						   return buff;
	}
}

char* ELF_(dietelf_get_osabi_name)(ELF_(dietelf_bin_t) *bin)
{
	unsigned int osabi = bin->ehdr.e_ident[EI_OSABI];
	static char buff[32];

	switch (osabi) {
		case ELFOSABI_NONE:			return "linux"; // sysv
		case ELFOSABI_HPUX:			return "hpux";
		case ELFOSABI_NETBSD:		return "netbsd";
		case ELFOSABI_LINUX:		return "linux";
		case ELFOSABI_SOLARIS:		return "solaris";
		case ELFOSABI_AIX:			return "aix";
		case ELFOSABI_IRIX:			return "irix";
		case ELFOSABI_FREEBSD:		return "freebsd";
		case ELFOSABI_TRU64:		return "tru64";
		case ELFOSABI_MODESTO:		return "modesto";
		case ELFOSABI_OPENBSD:		return "openbsd";
		case ELFOSABI_STANDALONE:	return "standalone";
		case ELFOSABI_ARM:			return "arm";
		default:
									snprintf (buff, sizeof (buff), "<unknown: %x>", osabi);
									return buff;
	}
}

u64
ELF_(dietelf_get_section_index)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	const char *string = bin->string;
	int i;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], section_name))
			return i;
	}

	return -1;
}

u64 ELF_(dietelf_get_section_offset)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	const char *string = bin->string;
	int i;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], section_name))
			return shdrp->sh_offset;
	}

	return -1;
}

int ELF_(dietelf_get_section_size)(ELF_(dietelf_bin_t) *bin, int fd, const char *section_name)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	const char *string = bin->string;
	int i;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], section_name))
			return shdrp->sh_size;
	}

	return -1;
}

int ELF_(dietelf_is_big_endian)(ELF_(dietelf_bin_t) *bin)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;

	return (ehdr->e_ident[EI_DATA] == ELFDATA2MSB);
}

static u64 ELF_(get_import_addr)(ELF_(dietelf_bin_t) *bin, int fd, int sym)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	ELF_(Addr) plt_sym_addr, got_addr = 0;
	const char *string = bin->string;
	int i, j;
	u64 got_offset;
	
	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], ".got.plt"))
			got_addr = shdrp->sh_offset;
	}
	if (got_addr == 0) {
		/* TODO: Unknown GOT address */
	}

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], ".rel.plt")) {
			ELF_(Rel) *rel, *relp;
			rel = (ELF_(Rel) *)malloc(shdrp->sh_size);
			if (rel == NULL) {
				perror("malloc");
				return 0;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return 0;
			}

			if (read(fd, rel, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return 0;
			}

			relp = rel;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Rel)), relp++) {
				ELF_(aux_swap_endian)((u8*)&(relp->r_offset), sizeof(ELF_(Addr)));
				ELF_(aux_swap_endian)((u8*)&(relp->r_info), sizeof(ELF_(Word)));
			}

			got_offset = (rel->r_offset - bin->base_addr - got_addr) & ELF_GOTOFF_MASK;
			relp = rel;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Rel)), relp++) {
				if (ELF_R_SYM(relp->r_info) == sym) {
					if (lseek(fd, relp->r_offset-bin->base_addr-got_offset, SEEK_SET)
							!= relp->r_offset-bin->base_addr-got_offset) {
						perror("lseek");
						return 0;
					}

					if (read(fd, &plt_sym_addr, sizeof(ELF_(Addr))) != sizeof(ELF_(Addr))) {
						perror("read");
						return 0;
					}

					return plt_sym_addr-6;
				}
			}
		} else if (!strcmp(&string[shdrp->sh_name], ".rela.plt")) {
			ELF_(Rela) *rel, *relp;
			rel = (ELF_(Rela) *)malloc(shdrp->sh_size);
			if (rel == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, rel, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return -1;
			}

			relp = rel;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Rela)), relp++) {
				ELF_(aux_swap_endian)((u8*)&(relp->r_offset), sizeof(ELF_(Addr)));
				ELF_(aux_swap_endian)((u8*)&(relp->r_info), sizeof(ELF_(Word)));
			}

			got_offset = (rel->r_offset - bin->base_addr - got_addr) & ELF_GOTOFF_MASK;
			relp = rel;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Rela)), relp++) {
				if (ELF_R_SYM(relp->r_info) == sym) {
					if (lseek(fd, relp->r_offset-bin->base_addr-got_offset, SEEK_SET)
							!= relp->r_offset-bin->base_addr-got_offset) {
						perror("lseek");
						return 0;
					}

					if (read(fd, &plt_sym_addr, sizeof(ELF_(Addr))) != sizeof(ELF_(Addr))) {
						perror("read");
						return 0;
					}

					return plt_sym_addr-6;
				}
			}
		}
	}

	return 0;
}

int ELF_(dietelf_get_sections)(ELF_(dietelf_bin_t) *bin, int fd, dietelf_section *section)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	dietelf_section *sectionp;
	const char *string = bin->string;
	int i;


	shdrp = shdr;
	sectionp = section;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++, sectionp++) {
		sectionp->offset = shdrp->sh_offset;
		sectionp->size = shdrp->sh_size;
		sectionp->align = shdrp->sh_addralign;
		sectionp->flags = shdrp->sh_flags;
		memcpy(sectionp->name, &string[shdrp->sh_name], ELF_NAME_LENGTH);
		sectionp->name[ELF_NAME_LENGTH-1] = '\0';
	}

	return 0;
}

int ELF_(dietelf_get_sections_count)(ELF_(dietelf_bin_t) *bin)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;


	return ehdr->e_shnum;
}

int ELF_(dietelf_get_imports)(ELF_(dietelf_bin_t) *bin, int fd, dietelf_import *import)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	ELF_(Sym) *sym, *symp;
	ELF_(Shdr) *strtabhdr;
	dietelf_import *importp;
	char *string;
	int i, j, k;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (shdrp->sh_type == (bin->ehdr.e_type == ET_REL?SHT_SYMTAB:SHT_DYNSYM)) {
			strtabhdr = &shdr[shdrp->sh_link];

			string = (char *)malloc(strtabhdr->sh_size);
			if (string == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, string, strtabhdr->sh_size) != strtabhdr->sh_size) {
				perror("read");
				return -1;
			}

			sym = (ELF_(Sym) *)malloc(shdrp->sh_size);
			if (sym == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, sym, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return -1;
			}

			symp = sym;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), symp++) {
				ELF_(aux_swap_endian)((u8*)&(symp->st_name), sizeof(ELF_(Word)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_value), sizeof(ELF_(Addr)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_size), sizeof(ELF_(Word)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_shndx), sizeof(ELF_(Section)));
			}

			importp = import;
			symp = sym;
			for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), k++, symp++) {
				if (k == 0)
					continue;
				if (symp->st_shndx == STN_UNDEF) {
					memcpy(importp->name, &string[symp->st_name], ELF_NAME_LENGTH);
					importp->name[ELF_NAME_LENGTH-1] = '\0';
					if (symp->st_value)
						importp->offset = symp->st_value;
					else
						importp->offset = ELF_(get_import_addr)(bin, fd, k);
					if (importp->offset >= bin->base_addr)
						importp->offset -= bin->base_addr;

					switch (ELF_ST_BIND(symp->st_info)) {
					case STB_LOCAL:  snprintf(importp->bind, ELF_NAME_LENGTH, "LOCAL"); break;
					case STB_GLOBAL: snprintf(importp->bind, ELF_NAME_LENGTH, "GLOBAL"); break;
					case STB_NUM:    snprintf(importp->bind, ELF_NAME_LENGTH, "NUM"); break;
					case STB_LOOS:   snprintf(importp->bind, ELF_NAME_LENGTH, "LOOS"); break;
					case STB_HIOS:   snprintf(importp->bind, ELF_NAME_LENGTH, "HIOS"); break;
					case STB_LOPROC: snprintf(importp->bind, ELF_NAME_LENGTH, "LOPROC"); break;
					case STB_HIPROC: snprintf(importp->bind, ELF_NAME_LENGTH, "HIPROC"); break;
					default:	 snprintf(importp->bind, ELF_NAME_LENGTH, "UNKNOWN");
					}
					switch (ELF_ST_TYPE(symp->st_info)) {
					case STT_NOTYPE:  snprintf(importp->type, ELF_NAME_LENGTH, "NOTYPE"); break;
					case STT_OBJECT:  snprintf(importp->type, ELF_NAME_LENGTH, "OBJECT"); break;
					case STT_FUNC:    snprintf(importp->type, ELF_NAME_LENGTH, "FUNC"); break;
					case STT_SECTION: snprintf(importp->type, ELF_NAME_LENGTH, "SECTION"); break;
					case STT_FILE:    snprintf(importp->type, ELF_NAME_LENGTH, "FILE"); break;
					case STT_COMMON:  snprintf(importp->type, ELF_NAME_LENGTH, "COMMON"); break;
					case STT_TLS:     snprintf(importp->type, ELF_NAME_LENGTH, "TLS"); break;
					case STT_NUM:     snprintf(importp->type, ELF_NAME_LENGTH, "NUM"); break;
					case STT_LOOS:    snprintf(importp->type, ELF_NAME_LENGTH, "LOOS"); break;
					case STT_HIOS:    snprintf(importp->type, ELF_NAME_LENGTH, "HIOS"); break;
					case STT_LOPROC:  snprintf(importp->type, ELF_NAME_LENGTH, "LOPROC"); break;
					case STT_HIPROC:  snprintf(importp->type, ELF_NAME_LENGTH, "HIPROC"); break;
					default:	  snprintf(importp->type, ELF_NAME_LENGTH, "UNKNOWN");
					}
					importp++;
				}
			}
		}
	}
	
	return 0;
}

int ELF_(dietelf_get_imports_count)(ELF_(dietelf_bin_t) *bin, int fd)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	ELF_(Sym) *sym, *symp;
	int i, j, k, ctr = 0;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (shdrp->sh_type == (bin->ehdr.e_type == ET_REL?SHT_SYMTAB:SHT_DYNSYM)) {
			sym = (ELF_(Sym) *)malloc(shdrp->sh_size);
			if (sym == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, sym, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return -1;
			}

			symp = sym;
			for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), k++, symp++) {
				if (k == 0)
					continue;
				if (symp->st_shndx == STN_UNDEF) {
					ctr++;
				}
			}
		}
	}

	return ctr;
}

int ELF_(dietelf_get_symbols)(ELF_(dietelf_bin_t) *bin, int fd, dietelf_symbol *symbol)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	ELF_(Sym) *sym, *symp;
	ELF_(Shdr) *strtabhdr;
	dietelf_symbol *symbolp;
	u64 sym_offset;
	char *string;
	int i, j, k;

	sym_offset = (bin->ehdr.e_type == ET_REL ? ELF_(dietelf_get_section_offset)(bin, fd, ".text") : 0);

	shdrp = shdr;

	/* No section headers found */
	if (ehdr->e_shnum == 0) {
	} else
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (shdrp->sh_type == (ELF_(dietelf_get_stripped)(bin)?SHT_DYNSYM:SHT_SYMTAB)) {
			strtabhdr = &shdr[shdrp->sh_link];

			string = (char *)malloc(strtabhdr->sh_size);
			if (string == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, string, strtabhdr->sh_size) != strtabhdr->sh_size) {
				perror("read");
				return -1;
			}

			sym = (ELF_(Sym) *)malloc(shdrp->sh_size);
			if (sym == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, sym, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return -1;
			}

			symp = sym;
			for (j = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), symp++) {
				ELF_(aux_swap_endian)((u8*)&(symp->st_name), sizeof(ELF_(Word)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_value), sizeof(ELF_(Addr)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_size), sizeof(ELF_(Word)));
				ELF_(aux_swap_endian)((u8*)&(symp->st_shndx), sizeof(ELF_(Section)));
			}

			symbolp = symbol;
			symp = sym;
			for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), k++, symp++) {
				if (k == 0)
					continue;
				if (symp->st_shndx != STN_UNDEF && ELF_ST_TYPE(symp->st_info) != STT_SECTION && ELF_ST_TYPE(symp->st_info) != STT_FILE) {
					symbolp->size = (u64)symp->st_size; 
					memcpy(symbolp->name, &string[symp->st_name], ELF_NAME_LENGTH); 
					symbolp->name[ELF_NAME_LENGTH-1] = '\0';
					symbolp->offset = (u64)symp->st_value + sym_offset;
					if (symbolp->offset >= bin->base_addr)
						symbolp->offset -= bin->base_addr;
					switch (ELF_ST_BIND(symp->st_info)) {
					case STB_LOCAL:		snprintf(symbolp->bind, ELF_NAME_LENGTH, "LOCAL"); break;
					case STB_GLOBAL:	snprintf(symbolp->bind, ELF_NAME_LENGTH, "GLOBAL"); break;
					case STB_NUM:		snprintf(symbolp->bind, ELF_NAME_LENGTH, "NUM"); break;
					case STB_LOOS:		snprintf(symbolp->bind, ELF_NAME_LENGTH, "LOOS"); break;
					case STB_HIOS:		snprintf(symbolp->bind, ELF_NAME_LENGTH, "HIOS"); break;
					case STB_LOPROC:	snprintf(symbolp->bind, ELF_NAME_LENGTH, "LOPROC"); break;
					case STB_HIPROC:	snprintf(symbolp->bind, ELF_NAME_LENGTH, "HIPROC"); break;
					default:			snprintf(symbolp->bind, ELF_NAME_LENGTH, "UNKNOWN");
					}
					switch (ELF_ST_TYPE(symp->st_info)) {
					case STT_NOTYPE:	snprintf(symbolp->type, ELF_NAME_LENGTH, "NOTYPE"); break;
					case STT_OBJECT:	snprintf(symbolp->type, ELF_NAME_LENGTH, "OBJECT"); break;
					case STT_FUNC:		snprintf(symbolp->type, ELF_NAME_LENGTH, "FUNC"); break;
					case STT_SECTION:	snprintf(symbolp->type, ELF_NAME_LENGTH, "SECTION"); break;
					case STT_FILE:		snprintf(symbolp->type, ELF_NAME_LENGTH, "FILE"); break;
					case STT_COMMON:	snprintf(symbolp->type, ELF_NAME_LENGTH, "COMMON"); break;
					case STT_TLS:		snprintf(symbolp->type, ELF_NAME_LENGTH, "TLS"); break;
					case STT_NUM:		snprintf(symbolp->type, ELF_NAME_LENGTH, "NUM"); break;
					case STT_LOOS:		snprintf(symbolp->type, ELF_NAME_LENGTH, "LOOS"); break;
					case STT_HIOS:		snprintf(symbolp->type, ELF_NAME_LENGTH, "HIOS"); break;
					case STT_LOPROC:	snprintf(symbolp->type, ELF_NAME_LENGTH, "LOPROC"); break;
					case STT_HIPROC:	snprintf(symbolp->type, ELF_NAME_LENGTH, "HIPROC"); break;
					default:		snprintf(symbolp->type, ELF_NAME_LENGTH, "UNKNOWN");
					}

					symbolp++;
				}
			}
		}
	}

	return 0;
}

int ELF_(dietelf_get_fields)(ELF_(dietelf_bin_t) *bin, dietelf_field *field)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Phdr) *phdr = bin->phdr;
	char string[ELF_NAME_LENGTH];
	int i = 0, j = 0;

	strncpy(field[i].name, "ehdr", ELF_NAME_LENGTH); 
	field[i++].offset = 0;
	strncpy(field[i].name, "shoff", ELF_NAME_LENGTH); 
	field[i++].offset = ehdr->e_shoff;
	strncpy(field[i].name, "phoff", ELF_NAME_LENGTH); 
	field[i++].offset = ehdr->e_phoff;

	for (j = 0; j < ehdr->e_phnum; i++, j++) {
		snprintf(string, ELF_NAME_LENGTH, "phdr_%i", j);
		strncpy(field[i].name, string, ELF_NAME_LENGTH); 
		field[i].offset = phdr[i].p_offset;
	}

	return 0;
}

int ELF_(dietelf_get_fields_count)(ELF_(dietelf_bin_t) *bin)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	int ctr=0;
	
	ctr = 3;
	ctr += ehdr->e_phnum;

	return ctr;
}


int ELF_(dietelf_get_symbols_count)(ELF_(dietelf_bin_t) *bin, int fd)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	ELF_(Sym) *sym, *symp;
	int i, j, k, ctr=0;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (shdrp->sh_type == (ELF_(dietelf_get_stripped)(bin)?SHT_DYNSYM:SHT_SYMTAB)) {
			sym = (ELF_(Sym) *)malloc(shdrp->sh_size);
			if (sym == NULL) {
				perror("malloc");
				return -1;
			}

			if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
				perror("lseek");
				return -1;
			}

			if (read(fd, sym, shdrp->sh_size) != shdrp->sh_size) {
				perror("read");
				return -1;
			}

			symp = sym;
			for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(ELF_(Sym)), k++, symp++) {
				if (k == 0)
					continue;
				if (symp->st_shndx != STN_UNDEF && ELF_ST_TYPE(symp->st_info) != STT_SECTION && ELF_ST_TYPE(symp->st_info) != STT_FILE)
					ctr++;
			}
		}
	}

	return ctr;
}

int ELF_(dietelf_get_strings)(ELF_(dietelf_bin_t) *bin, int fd, int verbose, int str_limit, dietelf_string *strings)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	const char *string = bin->string;
	int i, ctr = 0;


	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (verbose < 2 && i != 0 && !strcmp(&string[shdrp->sh_name], ".rodata"))
			ctr = ELF_(aux_stripstr_from_file)(bin->file, 3, ENCODING_ASCII, shdrp->sh_offset, shdrp->sh_offset+shdrp->sh_size, NULL, str_limit-ctr, strings+ctr);
		if (verbose == 2 && i != 0 && !(shdrp->sh_flags & SHF_EXECINSTR)) {
			ctr = ELF_(aux_stripstr_from_file)(bin->file, 3, ENCODING_ASCII, shdrp->sh_offset, shdrp->sh_offset+shdrp->sh_size, NULL, str_limit-ctr, strings+ctr);
		}
	}

	return ctr;
}

int ELF_(dietelf_get_libs)(ELF_(dietelf_bin_t) *bin, int fd, int str_limit, dietelf_string *strings)
{
	ELF_(Ehdr) *ehdr = &bin->ehdr;
	ELF_(Shdr) *shdr = bin->shdr, *shdrp;
	const char *string = bin->string;
	int i, ctr = 0;

	shdrp = shdr;
	for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
		if (!strcmp(&string[shdrp->sh_name], ".dynstr")) {
			ctr = ELF_(aux_stripstr_from_file)(bin->file, 3, ENCODING_ASCII, shdrp->sh_offset,
				shdrp->sh_offset+shdrp->sh_size, ".so.", str_limit, strings+ctr);
		}
	}

	return ctr;
}

static int ELF_(dietelf_init)(ELF_(dietelf_bin_t) *bin, int fd)
{
	ELF_(Ehdr) *ehdr;
	ELF_(Shdr) *shdr;
	ELF_(Shdr) *strtabhdr;
	ELF_(Phdr) *phdr;
	char **sectionp;
	int i, slen;
	bin->base_addr = 0;

	ehdr = &bin->ehdr;

	if (read(fd, ehdr, sizeof(ELF_(Ehdr))) != sizeof(ELF_(Ehdr))) {
		perror("read");
		return -1;
	}

	if (ehdr->e_ident[EI_DATA] == ELFDATA2MSB)
		endian = LIL_ENDIAN;
	else    endian = !LIL_ENDIAN;

	ELF_(aux_swap_endian)((u8*)&(ehdr->e_type), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_machine), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_version), sizeof(ELF_(Word)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_entry), sizeof(ELF_(Addr)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_phoff), sizeof(ELF_(Off)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_shoff), sizeof(ELF_(Off)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_flags), sizeof(ELF_(Word)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_ehsize), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_phentsize), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_phnum), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_shentsize), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_shnum), sizeof(ELF_(Half)));
	ELF_(aux_swap_endian)((u8*)&(ehdr->e_shstrndx), sizeof(ELF_(Half)));

	if (ELF_(do_elf_checks)(bin) == -1)
		return -1;

	bin->phdr = (ELF_(Phdr) *)malloc(bin->plen = sizeof(ELF_(Phdr))*ehdr->e_phnum);
	if (bin->phdr == NULL) {
		perror("malloc");
		return -1;
	}

	if (lseek(fd, ehdr->e_phoff, SEEK_SET) < 0) {
		perror("lseek");
		fprintf(stderr, "Warning: Cannot read program headers (0x%08x->0x%08x)\n",
			ehdr->e_phoff, (long)&ehdr->e_phoff-(long)&ehdr->e_ident);
		//return -1;
	}

	if (read(fd, bin->phdr, bin->plen) != bin->plen) {
		perror("read");
		fprintf(stderr, "Warning: Cannot read program headers (0x%08x->0x%08x)\n",
			ehdr->e_phoff, (long)&ehdr->e_phoff-(long)&ehdr->e_ident);
		//return -1;
	}

	for (i = 0, phdr = bin->phdr; i < ehdr->e_phnum; i++) {
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_type), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_offset), sizeof(ELF_(Off)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_vaddr), sizeof(ELF_(Addr)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_paddr), sizeof(ELF_(Addr)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_filesz), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_memsz), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_flags), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(phdr[i].p_align), sizeof(ELF_(Word)));
	}

	bin->shdr = (ELF_(Shdr) *)malloc(slen = sizeof(ELF_(Shdr))*ehdr->e_shnum);
	if (bin->shdr == NULL) {
		perror("malloc");
		return -1;
	}

	bin->section = (char **)malloc(sizeof(char **)*ehdr->e_shnum);
	if (bin->section == NULL) {
		perror("malloc");
		return -1;
	}

	if (lseek(fd, ehdr->e_shoff, SEEK_SET) < 0) {
		perror("lseek");
		ehdr->e_shnum=0;
		fprintf(stderr, "Warning: Cannot read %d section headers (0x%08x->0x%08x)\n",
			ehdr->e_shnum, ehdr->e_shoff, (long)&ehdr->e_shoff-(long)&ehdr->e_ident);
	//	return -1;
	}

	if (read(fd, bin->shdr, slen) != slen) {
		perror("read");
		fprintf(stderr, "Warning: Cannot read %d section headers (0x%08x->0x%08x)\n",
			ehdr->e_shnum, ehdr->e_shoff, (long)&ehdr->e_shoff-(long)&ehdr->e_ident);
		ehdr->e_shnum=0;
		//return -1;
	}

	for (i = 0, shdr = bin->shdr; i < ehdr->e_shnum; i++) {
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_name), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_type), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_flags), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_addr), sizeof(ELF_(Addr)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_offset), sizeof(ELF_(Off)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_size), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_link), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_info), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_addralign), sizeof(ELF_(Word)));
		ELF_(aux_swap_endian)((u8*)&(shdr[i].sh_entsize), sizeof(ELF_(Word)));
	}

	strtabhdr = &bin->shdr[ehdr->e_shstrndx];

	bin->string = (char *)malloc(strtabhdr->sh_size);
	if (bin->string == NULL) {
		perror("malloc");
		return -1;
	}

	if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
		perror("lseek");
		return -1;
	}

	if (read(fd, bin->string, strtabhdr->sh_size) != strtabhdr->sh_size) {
		perror("read");
		return -1;
	}

	bin->bss = -1;

	for (i = 0, sectionp = bin->section, shdr = bin->shdr; i < ehdr->e_shnum; i++, sectionp++) {
		if (shdr[i].sh_type == SHT_NOBITS) {
			bin->bss = i;
		} else {
			if (ELF_(load_section)(sectionp, fd, &shdr[i]) == -1)
				return -1;
		}
	}

	bin->base_addr = ELF_(dietelf_get_base_addr)(bin);

	return 0;
}

static int ELF_(load_section)(char **section, int fd, ELF_(Shdr) *shdr)
{
	if (lseek(fd, shdr->sh_offset, SEEK_SET) < 0) {
		perror("lseek");
		return -1;
	}

	*section = (char *)malloc(shdr->sh_size);
	if (*section == NULL) {
		perror("malloc");
		return -1;
	}

	if (read(fd, *section, shdr->sh_size) != shdr->sh_size) {
		perror("read");
		return -1;
	}

	return 0;
}

int ELF_(dietelf_open)(ELF_(dietelf_bin_t) *bin, const char *file)
{
	int fd;

	if ((fd=open(file, O_RDONLY)) == -1) {
		fprintf(stderr, "Error: Cannot open \"%s\"\n", file);
		return -1;
	}

	if (ELF_(dietelf_init)(bin, fd) == -1)
		return -1;

	bin->file = file;

	return fd;
}

