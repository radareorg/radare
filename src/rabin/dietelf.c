/* Author: nibble 
 * --------------
 * Based on Silvio Cesare's elf infector
 * Licensed under GPLv2
 * This file is part of radare
 *
 * TODO: port to x86-64
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "../main.h"
#include "dietelf.h"

static char rad_output[255];
enum {
	ENCODING_ASCII = 0,
	ENCODING_CP850 = 1
};

#ifdef RADARE_CORE
extern int xrefs;
extern int rad;
extern int verbose;
#else
int rad = 0;
int verbose = 1;
int xrefs = 0; // XXX
#endif

int
aux_is_encoded(int encoding, unsigned char c)
{
	switch(encoding) {
	case ENCODING_ASCII:
		break;
	case ENCODING_CP850:
		switch(c) {
		// CP850
		case 128: // cedilla
		case 133: // a grave
		case 135: // minicedilla
		case 160: // a acute
		case 161: // i acute
		case 129: // u dieresi
		case 130: // e acute
		case 139: // i dieresi
		case 162: // o acute
		case 163: // u acute
		case 164: // enye
		case 165: // enyemay
		case 181: // A acute
		case 144: // E acute
		case 214: // I acute
		case 224: // O acute
		case 233: // U acute
			return 1;
		}
		break;
	}
	return 0;
}

int
aux_is_printable (int c)
{
	if (c<' '||c>'~') return 0;
	return 1;
}

int 
aux_stripstr_iterate(const unsigned char *buf, int i, int min, int enc, u64 base, u64 offset)
{
	static int unicode = 0;
	static int matches = 0;
	char str[4096];

	if (aux_is_printable(buf[i]) || (aux_is_encoded(enc, buf[i]))) {
		if (matches == 0)
			offset += i;
		str[matches] = buf[i];
		if (matches < sizeof(str))
			matches++;
	} else {
		/* wide char check \x??\x00\x??\x00 */
		if (matches && buf[i+2]=='\0' && buf[i]=='\0' && buf[i+1]!='\0') {
			unicode = 1;
			return 1; // unicode
		}
		/* check if the length fits on our request */
		if (matches >= min) {
			str[matches] = '\0';
			int len = strlen(str);
			if (len>2) {
			    if (rad) {
				printf("f str_%s @ 0x%08llx\n",
					aux_filter_rad_output(str), offset-matches+base);
				printf("Cs %i @ 0x%08llx\n", len, offset-matches);
			    } else {
				printf("0x%08llx %03d %c %s\n",
					offset-matches+base, len, (unicode)?'U':'A', str);
			    }
			}
		}
		matches = 0;
		unicode = 0;
	}
	return 0;
}

int
aux_stripstr_from_file(const char *filename, int min, int encoding, u64 base, u64 seek, u64 limit)
{
	int fd = open(filename, O_RDONLY);
	unsigned char *buf;
	u64 i = seek;
	u64 len;

	if (fd == -1) {
		fprintf(stderr, "Cannot open target file.\n");
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

	for(i = seek; i < len; i++) 
		aux_stripstr_iterate(buf, i, min, encoding, base, i);

	munmap(buf, len); 
#elif __WINDOWS__
	fprintf(stderr, "Not yet implemented\n");
#endif
	close(fd);

	return 0;
}

int
do_elf_checks(dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;

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

char*
aux_filter_rad_output(const char *string)
{
    char *p = rad_output;

    for (; *string != '\0' && p-rad_output < 255; string++) {
	switch(*string) {
	    case ' ':
	    case '@':
	    case '%':
	    case '#':
	    case '!':
	    case '|':
	    case ':':
	    case '"':
	    case '&':
	    case '>':
	    case '<':
	    case ';':
	    case '`':
	    case '.':
	    case '*':
	    case '/':
	    case '+':
	    case '-':
	    case '\'':
	    case '\n':
	    case '\t':
		*p++ = '_';
		break;
	    default:
		*p++ = *string;
		break;
	}
    }

    *p='\0';

    return rad_output;
}

u64
dietelf_get_base_addr(dietelf_bin_t *bin)
{
    return bin->phdr->p_vaddr & 0xfffff000;
}

u64
dietelf_get_entry_addr(dietelf_bin_t *bin)
{
   return bin->ehdr.e_entry; 
}

u64
dietelf_get_section_offset(int fd, dietelf_bin_t *bin, const char *section_name)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    const char *string = bin->string;
    int i;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (!strcmp(&string[shdrp->sh_name], section_name))
	    return shdrp->sh_offset;
    }

    return -1;
}

int
dietelf_get_section_size(int fd, dietelf_bin_t *bin, const char *section_name)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    const char *string = bin->string;
    int i;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (!strcmp(&string[shdrp->sh_name], section_name))
	    return shdrp->sh_size;
    }

    return -1;
}

u64
get_import_addr(int fd, dietelf_bin_t *bin, int sym)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    Elf32_Rel *rel, *relp;
    Elf32_Addr plt_sym_addr, got_addr = 0;
    const char *string = bin->string;
    int i, j, got_offset;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (!strcmp(&string[shdrp->sh_name], ".got.plt"))
	    got_addr = shdrp->sh_offset & 0xfffff000;
    }
    if (got_addr == 0) {
	/* Unknown GOT address */
    }

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (!strcmp(&string[shdrp->sh_name], ".rel.plt")) {
	    rel = (Elf32_Rel *)malloc(shdrp->sh_size);
	    if (rel == NULL) {
		perror("malloc");
		return -1;
	    }

	    if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
		perror("lseek");
		return -1;
	    }

	    if (read(fd, rel, shdrp->sh_size) != shdrp->sh_size) {
		perror("read syms_addr");
		return -1;
	    }

	    got_offset = ((rel->r_offset - bin->base_addr) & 0xfffff000) - got_addr;

	    relp = rel;

	    for (j = 0; j < shdrp->sh_size; j += sizeof(Elf32_Rel), relp++) {
		if (ELF32_R_SYM(relp->r_info) == sym) {
		    if (lseek(fd, relp->r_offset-bin->base_addr-got_offset, SEEK_SET) != relp->r_offset-bin->base_addr-got_offset) {
			perror("lseek oops");
			return -1;
		    }

		    if (read(fd, &plt_sym_addr, sizeof(Elf32_Addr)) != sizeof(Elf32_Addr)) {
			perror("read syms_addr read");
			return -1;
		    }
		    
		    plt_sym_addr-=0x6;

		    return plt_sym_addr;
		}
	    }
	}
    }

    return -1;
}

int
dietelf_list_sections(int fd, dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    const char *string = bin->string;
    int i;

#define GET_FLAGS(x) (x&SHF_WRITE)?'w':'-', (x&SHF_ALLOC)?'a':'-', (x&SHF_EXECINSTR)?'x':'-'
    shdrp = shdr;
    if (rad)
	printf("fs sections\n");
    else printf("Sections:\n");

    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (shdrp->sh_offset < bin->base_addr)
		shdrp->sh_offset += bin->base_addr;
	if (rad) {
		printf("f section_%s @ 0x%08llx\n", aux_filter_rad_output(&string[shdrp->sh_name]), (u64)shdrp->sh_offset);
		printf("f section_%s_end @ 0x%08llx\n", aux_filter_rad_output(&string[shdrp->sh_name]), (u64)shdrp->sh_offset+shdrp->sh_size);
		printf("CC ");
	}
	printf("0x%08x 0x%04x align=0x%02x %02d %c%c%c %s", 
		shdrp->sh_offset,
		shdrp->sh_size,
		shdrp->sh_addralign,
		shdrp->sh_type,
		GET_FLAGS(shdrp->sh_flags),
		&string[shdrp->sh_name]);
	if (rad)
		printf(" @ 0x%08x\n", shdrp->sh_offset);
	else printf("\n");
    }

    return i;
}

int
dietelf_list_imports(int fd, dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    Elf32_Sym *sym, *symp;
    Elf32_Shdr *strtabhdr;
    char *string;
    int i, j, k;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (shdrp->sh_type == SHT_DYNSYM) {
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

	    sym = (Elf32_Sym *)malloc(shdrp->sh_size);
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

	    if (rad)
		printf("fs sym_imports\n");

	    for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(Elf32_Sym), k++, symp++) {
		if (k != 0) {
		    if (symp->st_shndx == STN_UNDEF && ELF32_ST_TYPE(symp->st_info) == STT_FUNC) {
			if (rad) {
			    printf("f sym_imp_%s @ 0x%08llx\n", aux_filter_rad_output(&string[symp->st_name]), get_import_addr(fd, bin, k));
			} else {
			    if (verbose) printf("Symbol (Import): ");
			    printf("0x%08llx %s\n", get_import_addr(fd, bin, k), &string[symp->st_name]);
			    if (xrefs) {
				char buf[1024];
				sprintf(buf, "xrefs -b 0x%08llx '%s' 0x%08llx", (u64)bin->base_addr, bin->file, (u64)get_import_addr(fd, bin, k));
				system(buf);
			    }
			}
		    }
		}
	    }
	    
	    free(string);
	}
    }
    return i;
}

int
dietelf_list_exports(int fd, dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    Elf32_Sym *sym, *symp;
    Elf32_Shdr *strtabhdr;
    char *string;
    int i, j, k, stripped = 0;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++)
	if (shdrp->sh_type == SHT_SYMTAB)
	    stripped = 1;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (shdrp->sh_type == (stripped?SHT_SYMTAB:SHT_DYNSYM)) {
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

	    sym = (Elf32_Sym *)malloc(shdrp->sh_size);
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

	    if (rad)
		printf("fs sym_exports\n");

	    for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(Elf32_Sym), k++, symp++) {
		if (k != 0) {
		    if ((symp->st_shndx > 10 && symp->st_shndx < 14) && ELF32_ST_TYPE(symp->st_info) == STT_FUNC) {
			if (rad) {
			    if (symp->st_size != 0) printf("b %i && ", symp->st_size); 
			    printf("f sym_exp_%s @ 0x%08x\n", aux_filter_rad_output(&string[symp->st_name]), symp->st_value);
			} else { 
			    if (verbose) printf("Symbol (Export) size=%05i: ", symp->st_size);
			    printf("0x%08llx %s\n", (u64)symp->st_value, &string[symp->st_name]);
			    if (xrefs) {
				char buf[1024];
				sprintf(buf, "xrefs -b 0x%08llx '%s' 0x%08llx", (u64)bin->base_addr, bin->file, (u64)symp->st_value);
				system(buf);
			    }
			}
		    }
		}
	    }
	    
	    free(string);
	}
    }
    return i;
}

int
dietelf_list_others(int fd, dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    Elf32_Sym *sym, *symp;
    Elf32_Shdr *strtabhdr;
    char *string;
    int i, j, k;

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (shdrp->sh_type == (SHT_SYMTAB)) {
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

	    sym = (Elf32_Sym *)malloc(shdrp->sh_size);
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

	    if (rad)
		printf("fs sym_others\n");

	    for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(Elf32_Sym), k++, symp++) {
		if (k != 0) {
		    if (symp->st_shndx == 3  && ELF32_ST_TYPE(symp->st_info) == STT_FUNC) {
			if (rad) {
			    if (symp->st_size != 0) printf("b %i && ", symp->st_size); 
			    printf("f sym_oth_%s @ 0x%08x\n", aux_filter_rad_output(&string[symp->st_name]), symp->st_value);
			} else { 
			    if (verbose) printf("Symbol (Other) size=%05i: ", symp->st_size);
			    printf("0x%08llx %s\n", (u64)symp->st_value, &string[symp->st_name]);
			    if (xrefs) {
				char buf[1024];
				sprintf(buf, "xrefs -b 0x%08llx '%s' 0x%08llx", (u64)bin->base_addr, bin->file, (u64)symp->st_value);
				system(buf);
			    }
			}
		    }
		}
	    }
	    
	    free(string);
	}
    }
    return i;
}

int
dietelf_list_strings(int fd, dietelf_bin_t *bin)
{
    /* TODO: define callback for printing strings found */
    Elf32_Ehdr *ehdr = &bin->ehdr;
    Elf32_Shdr *shdr = bin->shdr, *shdrp;
    const char *string = bin->string;
    int i;

    shdrp = shdr;
    if (rad)
	printf("fs strings\n"); //, aux_filter_rad_output(&string[shdrp->sh_name]));
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (i != 0 && !(shdrp->sh_flags & SHF_EXECINSTR)) {
	    if (!rad) printf("==> Strings in %s:\n", &string[shdrp->sh_name]);
	    aux_stripstr_from_file(bin->file, 3, ENCODING_ASCII, bin->base_addr, shdrp->sh_offset, shdrp->sh_offset+shdrp->sh_size);
	}
    }

    return i;
}

int
dietelf_open(int fd, dietelf_bin_t *bin)
{
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    Elf32_Shdr *strtabhdr;
    char **sectionp;
    int i, slen;
	bin->base_addr = 0;

    ehdr = &bin->ehdr;

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
	perror("read 6");
	return -1;
    }

    if (do_elf_checks(bin) == -1)
	return -1;

    bin->phdr = (Elf32_Phdr *)malloc(bin->plen = sizeof(Elf32_Phdr)*ehdr->e_phnum);
    if (bin->phdr == NULL) {
	perror("malloc");
	return -1;
    }

    if (lseek(fd, ehdr->e_phoff, SEEK_SET) < 0) {
	perror("lseek 0");
	return -1;
    }

    if (read(fd, bin->phdr, bin->plen) != bin->plen) {
	perror("read 0");
	return -1;
    }

    bin->shdr = (Elf32_Shdr *)malloc(slen = sizeof(Elf32_Shdr)*ehdr->e_shnum);
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
	perror("lseek 1");
	return -1;
    }

    if (read(fd, bin->shdr, slen) != slen) {
	perror("read 2");
	return -1;
    }

    strtabhdr = &bin->shdr[ehdr->e_shstrndx];

    bin->string = (char *)malloc(strtabhdr->sh_size);
    if (bin->string == NULL) {
	perror("malloc");
	return -1;
    }

    if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
	perror("lseek 2");
	return -1;
    }

    if (read(fd, bin->string, strtabhdr->sh_size) != strtabhdr->sh_size) {
	perror("read 3");
	return -1;
    }

    bin->bss = -1;

    for (i = 0, sectionp = bin->section, shdr = bin->shdr; i < ehdr->e_shnum; i++, sectionp++) {
	if (shdr[i].sh_type == SHT_NOBITS) {
	    bin->bss = i;
	} else {
	    if (load_section(sectionp, fd, &shdr[i]) == -1)
		return -1;
	}
    }

    if (bin->bss < 0) {
	printf("No bss section\n");
	return -1;
    }

    bin->base_addr = dietelf_get_base_addr(bin);

    return 0;
}

int 
load_section(char **section, int fd, Elf32_Shdr *shdr)
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
	perror("read load section");
	return -1;
    }

    return 0;
}

int dietelf_new(const char *file, dietelf_bin_t *bin)
{
    int fd;

    if ((fd=open(file, O_RDONLY)) == -1) {
	fprintf(stderr, "Error: Cannot open \"%s\"\n", file);
	return -1;
    }
    
    if (dietelf_open(fd, bin) == -1)
	return -1;

    bin->file = file;

    return fd;
}

#ifndef RADARE_CORE

int
main(int argc, char *argv[])
{
    char *file;
    int fd;
    dietelf_bin_t bin;

    if (argc != 2)
    {
	fprintf(stderr, "Usage: %s [file]\n", argv[0]);
	return 1;
    }

    file=argv[1];
    if ((fd=open(file, O_RDONLY)) == -1)
    {
	fprintf(stderr, "Error: Cannot open \"%s\"\n", file);
	return 1;
    }
    
    dietelf_open(fd, &bin);
    bin.file = file;

    if (rad)
    	printf(" eval file.baddr=0x%08x\n", bin.base_addr);
    else
	printf("BASE ADDRESS: 0x%08x\n", bin.base_addr);

    dietelf_list_sections(fd, &bin);
    dietelf_list_imports(fd, &bin);
    
    close(fd);

    return 0;
}

#endif
