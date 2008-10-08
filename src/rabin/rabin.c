/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       nibble
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "../main.h"
#include "rabin.h"
#include <stdio.h>
#if __UNIX__
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "dietelf.h"
#include "dietelf64.h"
#include "dietpe.h"
#include "dietpe_types.h"
#if defined(_DARWIN_C_SOURCE)
#define HAVE_MACHO 1
#include "dietmach0.h"
#else
#define HAVE_MACHO 0
#endif

#define ELF_CALL(func, bin, args...) elf64?Elf64_##func(&bin.e64,##args):Elf32_##func(&bin.e32,##args)

typedef union {
    Elf32_dietelf_bin_t e32;
    Elf64_dietelf_bin_t e64;
} dietelf_bin_t;

// TODO : move into rabin_t
char *file = NULL;
int filetype = FILETYPE_UNK;
int action   = ACTION_UNK;
int verbose  = 0;
int xrefs    = 0;
int rad      = 0; //radare output format
int fd       = -1;
static int pebase = 0;
static int elf64 = 0;

int rabin_show_help()
{
	printf(
"rabin [options] [bin-file]\n"
" -e        shows entrypoints one per line\n"
" -i        imports (symbols imported from libraries)\n"
" -s        symbols (exports)\n"
" -c        header checksum\n"
" -S        show sections\n"
" -l        linked libraries\n"
" -L [lib]  dlopen library and show address\n"
" -z        search for strings in elf non-executable sections\n"
" -x        show xrefs of symbols (-s/-i required)\n"
" -I        show binary info\n"
" -r        output in radare commands\n"
" -v        be verbose\n");
	return 1;
}

void rabin_show_info(const char *file)
{
	char *str, pe_class_str[PE_NAME_LENGTH], pe_subsystem_str[PE_NAME_LENGTH], pe_machine_str[PE_NAME_LENGTH];
	int pe_class, pe_subsystem, pe_machine;
	u64 baddr;
	dietelf_bin_t bin;
	dietpe_bin pebin;
	dietpe_entrypoint entrypoint;

	switch(filetype) {
	case FILETYPE_ELF:
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}

		baddr = ELF_CALL(dietelf_get_base_addr,bin);
		
		if (rad) {
			printf("e file.type = elf\n");
			str = getenv("DEBUG");
			if (str && strncmp(str, "1", 1))
				printf("e file.baddr = 0x%08llx\n", baddr);
			if (ELF_CALL(dietelf_is_big_endian,bin))
				printf("e cfg.bigendian = true\n");
			else
				printf("e cfg.bigendian = false\n");
			if (ELF_CALL(dietelf_get_stripped,bin))
			    printf("e dbg.dwarf = false\n");
			else printf("e dbg.dwarf = true\n");
			printf("e asm.arch = %s\n", ELF_CALL(dietelf_get_osabi_name,bin));
			switch (ELF_CALL(dietelf_get_arch,bin)) {
				case EM_MIPS:
				case EM_MIPS_RS3_LE:
				case EM_MIPS_X:
					printf("e asm.arch = mips\n"); break;
				case EM_ARM:
					printf("e asm.arch = arm\n"); break;
				case EM_SPARC:
				case EM_SPARC32PLUS:
				case EM_SPARCV9:
					printf("e asm.arch = sparc\n"); break;
				case EM_PPC:
				case EM_PPC64:
					printf("e asm.arch = ppc\n"); break;
				case EM_68K:
					printf("e asm.arch = m68k\n"); break;
				case EM_IA_64:
				case EM_X86_64:
					printf("e asm.arch = intel64\n"); break;
				default: printf("e asm.arch = intel\n");
			}
		} else {
			printf("ELF class:       %s\n"
			       "Data enconding:  %s\n"
			       "OS/ABI name:     %s\n"
			       "Machine name:    %s\n"
			       "File type:       %s\n",
			       ELF_CALL(dietelf_get_elf_class,bin),
			       ELF_CALL(dietelf_get_data_encoding,bin),
			       ELF_CALL(dietelf_get_osabi_name,bin),
			       ELF_CALL(dietelf_get_machine_name,bin),
			       ELF_CALL(dietelf_get_file_type,bin));

			printf("Stripped:        %s\n",
				(ELF_CALL(dietelf_get_stripped,bin))?"Yes":"No");

			printf("Static:          %s\n",
				(ELF_CALL(dietelf_get_static,bin))?"Yes":"No");
		}

		close(fd);
		break;
	case FILETYPE_CLASS:
		if (rad) {
			printf("e asm.arch = java\n");
			printf("e cfg.bigendian = true\n");
		} else printf("File type: JAVA CLASS\n");
		break;
	case FILETYPE_PE:
		if ((fd = dietpe_open(&pebin, file)) == -1) {
			fprintf(stderr, "Cannot open file\n");
			return;
		}

		dietpe_get_entrypoint(&pebin, &entrypoint);
		pe_class = dietpe_get_class(&pebin, pe_class_str);
		pe_machine = dietpe_get_machine(&pebin, pe_machine_str);
		pe_subsystem = dietpe_get_subsystem(&pebin, pe_subsystem_str);

		if (rad) printf("e file.type = pe\n");
		else { 
			printf("PE Class: %s (0x%x)\n"
					"DLL: %s\n"
					"Machine: %s (0x%x)\n"
					"Big endian: %s\n"
					"Subsystem: %s (0x%x)\n"
					"Stripped:\n"
					"  - Relocs: %s\n"
					"  - Line numbers: %s\n"
					"  - Local symbols: %s\n"
					"  - Debug: %s\n"
					"Number of sections: %i\n"
					"Image base: 0x%.08x\n"
					"Entrypoint (disk): 0x%.08x\n"
					"Entrypoint (rva): 0x%.08x\n"
					"Section alignment: %i\n"
					"File alignment: %i\n"
					"Image size: %i\n",
					pe_class_str, pe_class, dietpe_is_dll(&pebin)?"True":"False", pe_machine_str, pe_machine,
					dietpe_is_big_endian(&pebin)?"True":"False", pe_subsystem_str, pe_subsystem,
					dietpe_is_stripped_relocs(&pebin)?"True":"False", dietpe_is_stripped_line_nums(&pebin)?"True":"False",
					dietpe_is_stripped_local_syms(&pebin)?"True":"False", dietpe_is_stripped_debug(&pebin)?"True":"False",
					dietpe_get_sections_count(&pebin), dietpe_get_image_base(&pebin), entrypoint.offset, entrypoint.rva,
					dietpe_get_section_alignment(&pebin), dietpe_get_file_alignment(&pebin), dietpe_get_image_size(&pebin));
		}

		dietpe_close(fd);
		break;
	case FILETYPE_MZ:
		if (rad) printf("e file.type = mz\n");
		else printf("File type: DOS COM\n");
		break;
	case FILETYPE_DEX:
		if (!rad)
			printf("File type: DEX (google android)\n");
		break;
	case FILETYPE_MACHO:
		if (rad) {
			printf("e file.type = macho\n");
			printf("e asm.arch = ppc\n");
			printf("e cfg.bigendian = false\n");
		} else printf("File type: MACH-O\n");
		break;
	case FILETYPE_CSRFW:
		if (rad)
			printf("e asm.arch = csr\n");
		else printf("File type: CSR firmware\n");
		break;
	case FILETYPE_ARMFW:
		if (rad)
			printf("e asm.arch = arm\n");
		else printf("File type: ARM firmwarre\n");
		break;
	case FILETYPE_UNK:
		if (rad) printf("e file.type = unk\n");
		else printf("File type: UNKNOWN\n");
		break;
	}
}

void rabin_show_strings(const char *file)
{
	dietelf_bin_t bin;
	char buf[1024];


	switch(filetype) {
	case FILETYPE_ELF:
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_strings,bin,fd);
		close(fd);
		break;
	case FILETYPE_PE:
		// TODO: native version and support for non -r
		snprintf(buf, 1022, "rsc strings-pe-flag %s",file);
		system(buf);
		break;
	default:
		snprintf(buf, 1022, "echo /s | radare -nv %s",file);
		system(buf);
		break;
	}
	
	//sprintf(buf, "echo /s | radare -e file.id=true -nv %s", file);
	//system(buf);
}

void rabin_show_checksum(const char *file)
{
	unsigned char buf[32];
	unsigned long addr = 0;
	int i;

	switch(filetype) {
	case FILETYPE_DEX:
		lseek(fd, 8, SEEK_SET);
		read(fd, &addr, 4);
		printf("Checksum: 0x%08lx\n", addr);
		read(fd, &buf, 20);
		printf("SHA-1 Signature: ");
		for(i=0;i<20;i++)
			printf("%08x ", buf[i]);
		break;
	case FILETYPE_ELF:
		break;
	case FILETYPE_MZ:
		break;
	case FILETYPE_PE:
		lseek(fd, pebase+0x18, SEEK_SET);
		read(fd, &addr, 4);
		printf("0x%x checksum file offset\n", pebase+0x18);
		printf("0x%04x checksum\n", (unsigned int) (unsigned short)addr);
		break;
	}
}

void rabin_show_entrypoint()
{
	u64 addr = 0;
	u64 base = 0;
	dietelf_bin_t bin;

	switch(filetype) {
	case FILETYPE_ELF:
		/* pW 4 @ 0x18 */
		fd = ELF_CALL(dietelf_new,bin, file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		addr = ELF_CALL(dietelf_get_entry_addr, bin);
		base = ELF_CALL(dietelf_get_base_addr, bin);

		if (rad) {
			printf("fs symbols\n");
			printf("f entrypoint @ 0x%08llx\n", addr);
			printf("s entrypoint\n");
		} else {
			if (verbose) {
				printf("0x%08llx memory\n", addr);
				printf("0x%08llx disk\n", addr - base);
			} else {
				printf("0x%08llx\n", addr);
			}
		}

		close(fd);
		break;
	case FILETYPE_MACHO:
		printf("fs symbols\n");
#if 0
		/* TODO: Walk until LOAD COMMAND 9 */
Load command 9
        cmd LC_UNIXTHREAD
    cmdsize 80
     flavor i386_THREAD_STATE
      count i386_THREAD_STATE_COUNT
            eax 0x00000000 ebx    0x00000000 ecx 0x00000000 edx 0x00000000
            edi 0x00000000 esi    0x00000000 ebp 0x00000000 esp 0x00000000
            ss  0x00000000 eflags 0x00000000 eip 0x000023d0 cs  0x00000000
            ds  0x00000000 es     0x00000000 fs  0x00000000 gs  0x00000000
#endif
		{
		char buf[256];
		sprintf(buf, "otool -l %s | grep eip | awk '{print $6}'", file);
		system(buf);
		}
		break;
	case FILETYPE_MZ:
		break;
	case FILETYPE_PE:
		lseek(fd, pebase+0x28, SEEK_SET);
		read(fd, &addr, 4);
	//	printf("0x%08x disk offset for ep\n", pebase+0x28);
		lseek(fd, pebase+0x45, SEEK_SET);
		read(fd, &base, 4);
		if (rad) {
			printf("f entrypoint @ 0x%08llx\n", addr);
		} else {
			if (verbose) {
				printf("0x%08llx memory\n", base+addr);
				printf("0x%08llx disk\n", addr-0xc00);
			} else	printf("0x%08llx\n", base+addr);
		}
		break;
	}
}

unsigned long long addr_for_lib(char *name)
{
#if __UNIX__
	unsigned long long *addr = dlopen(name, RTLD_LAZY);
	if (addr) {
		dlclose(addr);
		return (unsigned long long)((addr!=NULL)?(*addr):0);
	} else {
		printf("cannot open '%s' library\n", name);
		return 0LL;
	}
#endif
	return 0LL;
}

void rabin_show_arch(char *file)
{
	u32 dw;
	u16 w;

	switch(filetype) {
	case FILETYPE_MACHO:
#if HAVE_MACHO
		dm_read_header(1);
#endif
		break;
	case FILETYPE_ELF:
		lseek(fd, 16+2, SEEK_SET);
		read(fd, &w, 2);
		switch(w) {
		case 3:
			printf("arch: x86-32\n");
			break;
		case 0x28:
			printf("arch: ARM\n");
			break;
		default:
			printf("arch: 0x%x (unknown)\n", w);
		}
		break;
	case FILETYPE_PE:
		// [[0x3c]+4]
		lseek(fd, 0x3c, SEEK_SET);
		read(fd, &dw, 4);
		lseek(fd, dw+4, SEEK_SET);
		read(fd, &w, 2);
		switch(w) {
		case 0x1c0:
			printf("arch: ARM\n");
			break;
		case 0x14c:
			printf("arch: x86-32\n");
			break;
		default:
			printf("arch: 0x%x (unknown)\n", w);
		}
		break;
	}
}

void rabin_show_imports(const char *file)
{
	dietelf_bin_t bin;
	dietpe_bin pebin;
	dietpe_import *import, *importp;
	int i, imports_count;

	switch(filetype) {
	case FILETYPE_ELF:
#if 1
		{ char buf[1024];
		//sprintf(buf, "readelf -sA '%s'|grep GLOBAL | awk ' {print $8}'", file);
//		sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' UND ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
		sprintf(buf, "objdump -d '%s' | grep 'plt>:' | sed -e 's,@plt>:,,g' -e 's,[<],,g' | awk '{print \"f imp_\"$2\" @ 0x\"$1 }'", file);
		system(buf);
		}
#else
	// XXX doesnt works!!
		
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_imports,bin,fd);
		close(fd);
#endif
		break;
	case FILETYPE_MACHO:
		setenv("target", file, 1);
		if (rad) {
			printf("fs imports\n");
			fflush(stdout);
			system("otool -vI $target | grep 0x | awk '{ print \"f imp_\"$3\" @ \"$1 }'");
		} else {
			system("otool -vI $target | grep 0x");
#if 0
		   #if __DARWIN_BYTE_ORDER
			sprintf(buf, "nm '%s' | grep ' T ' | sed 's/ T / /' | awk '{print \"0x\"$1\" \"$2}'", file);
			system(buf);
		   #else
			sprintf(buf, "arm-apple-darwin-nm '%s' | grep ' T ' | sed 's/ T / /' | awk '{print \"0x\"$1\" \"$2}'", file);
			system(buf);
		   #endif
#endif
		}
		break;
	case FILETYPE_PE:
		if ((fd = dietpe_open(&pebin, file)) == -1) {
			fprintf(stderr, "Cannot open file\n");
			return;
		}

		imports_count = dietpe_get_imports_count(&pebin, fd);

		import = malloc(imports_count * sizeof(dietpe_import));
		dietpe_get_imports(&pebin, fd, import);

		if (!rad)
			printf("==> Imports:\n");
		importp = import;
		for (i = 0; i < imports_count; i++, importp++) {
			if (!rad)
				printf("0x%.08x rva=0x%.08x hint=%.04i ordinal=%.03i %s\n",
					   importp->offset, importp->rva, importp->hint, importp->ordinal, importp->name);
		}

		dietpe_close(fd);
		break;
	}
}

void rabin_show_symbols(char *file)
{
	char buf[1024];
	dietelf_bin_t bin;
	dietpe_bin pebin;
	dietpe_export *export, *exportp;
	int exports_count, i;

	switch(filetype) {
	case FILETYPE_ELF:
#if 0		
		sprintf(buf, "readelf -s '%s' | grep FUNC | grep GLOBAL | grep DEFAULT  | grep ' 12 ' | awk '{ print \"0x\"$2\" \"$8 }' | sort | uniq" , file);
		system(buf);
#endif		
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_symbols,bin,fd);
		close(fd);
		break;
	case FILETYPE_MACHO:
		setenv("target", file, 1);
		if (rad) {
			printf("fs symbols\n");
			fflush(stdout);
			system("otool -tv $target | grep -C 1 -e : | grep -v / | awk '{if (/:/){label=$1;gsub(\":\",\"\",label);next}if (label!=\"\"){print \"f sym\"label\" @ 0x\"$1;label=\"\"}}'");
		} else {
		   #if __DARWIN_BYTE_ORDER
			sprintf(buf, "nm '%s' | grep ' T ' | sed 's/ T / /' | awk '{print \"0x\"$1\" \"$2}'", file);
			system(buf);
		   #else
			sprintf(buf, "arm-apple-darwin-nm '%s' | grep ' T ' | sed 's/ T / /' | awk '{print \"0x\"$1\" \"$2}'", file);
			system(buf);
		   #endif
		}
		break;
	case FILETYPE_CLASS:
		// TODO: native version and support for non -r
		if (rad)
			snprintf(buf, 1022, "javasm -rc %s",file);
		else
			snprintf(buf, 1022, "javasm -c '%s'", file);
		system(buf);
		break;
	case FILETYPE_PE:
	if ((fd = dietpe_open(&pebin, file)) == -1) {
		fprintf(stderr, "Cannot open file\n");
		return;
	}

	exports_count = dietpe_get_exports_count(&pebin, fd);
	
	export = malloc(exports_count * sizeof(dietpe_export));
	dietpe_get_exports(&pebin, fd, export);
	
	if (!rad)
		printf("==> Exports:\n");
	exportp = export;
	for (i = 0; i < exports_count; i++, exportp++) {
		if (!rad)
			printf("0x%.08x rva=%.08x ordinal=%.03i forwarder=%s %s\n", exportp->offset, exportp->rva, exportp->ordinal, exportp->forwarder, exportp->name);
	}

	// DietPE Close
	dietpe_close(fd);
		break;
	}
}

#if 0
void rabin_show_others(char *file)
{
	dietelf_bin_t bin;

	switch(filetype) {
	case FILETYPE_ELF:
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_others,bin,fd);
		close(fd);
		break;
	}
}
#endif

void rabin_show_sections(const char *file)
{
	int fd, i, sections_count;
	dietelf_bin_t bin;
	dietpe_bin pebin;
	dietpe_section *section, *sectionp;

	switch(filetype) {
	case FILETYPE_MACHO:
#if HAVE_MACHO
		dm_read_command(0);
#endif
		break;
	case FILETYPE_ELF:
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_sections,bin,fd);
		close(fd);
		break;
	case FILETYPE_PE:
		if ((fd = dietpe_open(&pebin, file)) == -1) {
			fprintf(stderr, "Cannot open file\n");
			return;
		}
		
		sections_count = dietpe_get_sections_count(&pebin);

		section = malloc(sections_count * sizeof(dietpe_section));
		dietpe_get_sections(&pebin, section);
		sectionp = section;
		if (!rad)
			printf("==> Sections:\n");
		for (i = 0; i < sections_count; i++, sectionp++) {
			if (!rad)
				printf("[%.02i] 0x%.08x rva=0x%.08x size=0x%.08x privileges=%c%c%c%c %s\n",
					   i, sectionp->offset, sectionp->rva, sectionp->size,
					   (sectionp->characteristics & PE_IMAGE_SCN_MEM_READ)?'r':'-',
					   (sectionp->characteristics & PE_IMAGE_SCN_MEM_WRITE)?'w':'-',
					   (sectionp->characteristics & PE_IMAGE_SCN_MEM_EXECUTE)?'x':'-',
					   (sectionp->characteristics & PE_IMAGE_SCN_MEM_SHARED)?'s':'-',
					   sectionp->name);
		}

		dietpe_close(fd);
		break;
#if 0
	default:
		// TODO: use the way that rsc flag-sections does
		char buf[1024];
		sprintf(buf, "readelf -S '%s'|grep '\\[' | grep -v '\\[Nr\\]' | cut -c 4- | awk '{ print \"0x\"$4\" \"$2 }'", file);
		system(buf);
#endif
	}
}

void rabin_show_libs(const char *file)
{
	char buf[1024];
	int fd;
	dietelf_bin_t bin;

	switch(filetype) {
	case FILETYPE_ELF:
		fd = ELF_CALL(dietelf_new,bin,file);
		if (fd == -1) {
			fprintf(stderr, "cannot open file\n");
			return;
		}
		ELF_CALL(dietelf_list_libs,bin,fd);
		close(fd);
		break;
	case FILETYPE_MACHO:
		sprintf(buf, "otool -L '%s'", file);
		system(buf);
		break;
	default:
		sprintf(buf, "strings '%s' | grep -e '^lib'", file);
		system(buf);
		break;
	}
}

int rabin_identify_header()
{
	unsigned char buf[1024];

	lseek(fd, 0, SEEK_SET);
	read(fd, buf, 1024);

	if ( !memcmp(buf, "\xCA\xFE\xBA\xBE", 4) ) {
		if (buf[9])
			filetype = FILETYPE_CLASS;
		else
			filetype = FILETYPE_MACHO;
	} else if ( !memcmp(buf, "\xFE\xED\xFA\xCE", 4) ) {
		filetype = FILETYPE_MACHO;
		/* ENDIAN = BIG */
		if (rad)
			printf("e cfg.bigendian = big\n");
	} else if ( !memcmp(buf, "CSR-", 4) ) {
		filetype = FILETYPE_CSRFW;
		//	config_set("asm.arch", "csr");
	} else if ( !memcmp(buf, "dex\n009\0", 8) ) {
		filetype = FILETYPE_DEX;
	} else if ( !memcmp(buf, "\x7F\x45\x4c\x46", 4) ) {
		filetype = FILETYPE_ELF;
		if (buf[EI_CLASS] == ELFCLASS64)
			elf64 = 1;
	} else if ( !memcmp(buf, "\x4d\x5a", 2) ) {
		int pe = buf[0x3c] + (buf[0x3d] << 8);
		filetype = FILETYPE_MZ;
		if (buf[pe]=='P' && buf[pe+1]=='E') {
			filetype = FILETYPE_PE;
			pebase = pe;
		}
	} else if (buf[2]==0 && buf[3]==0xea) {
		filetype = FILETYPE_ARMFW;
	} else {
		if (!rad)
			printf("Unknown filetype\n");
	}
	return filetype;
}

int main(int argc, char **argv, char **envp)
{
	int c;

	while ((c = getopt(argc, argv, "acerlishL:SIvxz")) != -1)
	{
		switch( c ) {
		case 'a':
			action |= ACTION_ARCH;
			break;
#if 0
	/* XXX depend on sections ??? */
		case 'b':
			action |= ACTION_BASE;
			break;
#endif
		case 'i':
			action |= ACTION_IMPORTS;
			break;
		case 'c':
			action |= ACTION_CHECKSUM;
			break;
		case 's':
			action |= ACTION_SYMBOLS;
			//action |= ACTION_EXPORTS;
			break;
#if 0
		case 't':
			action |= ACTION_FILETYPE;
			break;
		case 'o':
			action |= ACTION_OTHERS;
			break;
#endif
		case 'S':
			action |= ACTION_SECTIONS;
			break;
		case 'I':
			action |= ACTION_INFO;
			break;
		case 'e':
			action |= ACTION_ENTRY;
			break;
		case 'l':
			action |= ACTION_LIBS;
			break;
		case 'L':
			printf("0x%08llx %s\n", addr_for_lib(optarg), optarg);
			action |= ACTION_NOP;
			break;
		case 'r':
			rad = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'x':
			xrefs = 1;
			break;
		case 'z':
			action |= ACTION_STRINGS;
			break;
		case 'h':
		default:
			return rabin_show_help();
		}
	}

	file = argv[optind];

	if (action == ACTION_UNK)
		return rabin_show_help();

	if (action != ACTION_NOP) {
		if (file == NULL)
			return rabin_show_help();
		fd = open(file, O_RDONLY);
		if (fd == -1) {
			fprintf(stderr, "Cannot open file\n");
			return 0;
		}
	} else return 0;

#if HAVE_MACHO
	dm_map_file(file, fd);
#endif
	rabin_identify_header();

	if (action&ACTION_ARCH)
		rabin_show_arch(file);
	if (action&ACTION_ENTRY)
		rabin_show_entrypoint(file);
#if 0
	if (action&ACTION_BASE)
		rabin_show_baseaddr(file);
	if (action&ACTION_FILETYPE)
		rabin_show_filetype();
	if (action&ACTION_EXPORTS)
		rabin_show_exports(file);
	if (action&ACTION_OTHERS)
		rabin_show_others(file);
#endif
	if (action&ACTION_IMPORTS)
		rabin_show_imports(file);
	if (action&ACTION_SYMBOLS)
		rabin_show_symbols(file);
	if (action&ACTION_SECTIONS)
		rabin_show_sections(file);
	if (action&ACTION_INFO)
		rabin_show_info(file);
	if (action&ACTION_LIBS)
		rabin_show_libs(file);
	if (action&ACTION_CHECKSUM)
		rabin_show_checksum(file);
	if (action&ACTION_STRINGS)
		rabin_show_strings(file);

	close(fd);

	return 0;
}
