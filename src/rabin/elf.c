/* Author: nibble */
/* TODO: rename to DIETELF (prefix all external functions) */
/* TODO: port to x86-64 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/types.h>

int rad = 0;
unsigned long BASE_ADDR = 0x8048000;

typedef struct {
    Elf32_Ehdr      ehdr;
    Elf32_Phdr*     phdr;
    Elf32_Shdr*     shdr;
    int		    plen;
    char**	    section;
    char*	    string;
    int		    bss;
} bin_t;


void do_elf_checks(Elf32_Ehdr *ehdr);
void get_syms_import(int fd, const char *bstring, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
int  get_syms_import_addr(int fd, const char *string, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, int sym);
void load_bin(int fd, bin_t *bin);
int  load_section(char **section, int fd, Elf32_Shdr *shdr);
int  main(int argc, char *argv[]);

void
do_elf_checks(Elf32_Ehdr *ehdr)
{
    if (strncmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
	fprintf(stderr, "File not ELF\n");
	exit(1);
    }

    if (ehdr->e_type != ET_EXEC) {
	fprintf(stderr, "ELF type not ET_EXEC or ET_DYN\n");
	exit(1);
    }

    if (ehdr->e_version != EV_CURRENT) {
	fprintf(stderr, "ELF version not current\n");
	exit(1);
    }
}

void
get_syms_import(int fd, const char *bstring, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr)
{
    Elf32_Shdr *shdrp = shdr;
    Elf32_Sym *sym, *symp;
    Elf32_Shdr *strtabhdr;
    char *string;
    int i, j, k;

    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (shdrp->sh_type == SHT_DYNSYM) {
	    strtabhdr = &shdr[shdrp->sh_link];

	    string = (char *)malloc(strtabhdr->sh_size);
	    if (string == NULL) {
		perror("malloc");
		exit(1);
	    }

	    if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
		perror("lseek");
		exit(1);
	    }

	    if (read(fd, string, strtabhdr->sh_size) != strtabhdr->sh_size) {
printf("one\n");
		perror("read");
		exit(1);
	    }

	    sym = (Elf32_Sym *)malloc(shdrp->sh_size);
	    if (sym == NULL) {
		perror("malloc");
		exit(1);
	    }

	    if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
		perror("lseek");
		exit(1);
	    }

	    if (read(fd, sym, shdrp->sh_size) != shdrp->sh_size) {
printf("one\n");
		perror("read");
		exit(1);
	    }

	    symp = sym;

	    for (j = 0, k = 0; j < shdrp->sh_size; j += sizeof(Elf32_Sym), k++, symp++) {
		if (k != 0)
		    printf("SYM: %s@plt (0x%.8x)\n", &string[symp->st_name], get_syms_import_addr(fd, bstring, ehdr, shdr, k));
	    }
	    
	    free(string);
	}
    }
}

int
list_sections(int fd, const char *string, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr)
{
    Elf32_Shdr *shdrp;
    Elf32_Rel *rel, *relp;
    Elf32_Addr plt_addr;
    int i, j;

#define GET_RWX(x) (x&4)?'r':'-', (x&2)?'w':'-', (x&1)?'x':'-'
    shdrp = shdr;
    if (rad)
	printf("fs sections\n");
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	if (rad) {
		/* TODO: filter section name (replace '.' for '_' and so) */
		/* TODO: check if sh_name points out of mother */
		printf("f section_%s @ 0x%08x\n", &string[shdrp->sh_name], shdrp->sh_offset);
		printf("f section_%s_end @ 0x%08x\n", &string[shdrp->sh_name], shdrp->sh_offset+shdrp->sh_size);
		printf("CC ");
	}
	printf("0x%08x align=0x%02x 0x%04x %d %c%c%c %s", 
		shdrp->sh_offset,
		shdrp->sh_addralign,
		shdrp->sh_size,
		shdrp->sh_type,
		GET_RWX(shdrp->sh_flags),
		&string[shdrp->sh_name]);
	if (rad)
		printf(" @ 0x%08x\n", shdrp->sh_offset);
	else printf("\n");
    }
}

int
get_syms_import_addr(int fd, const char *string, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, int sym)
{
    Elf32_Shdr *shdrp;
    Elf32_Rel *rel, *relp;
Elf32_Rela *rela;
    Elf32_Addr plt_addr;
    int i, j;

    if (rad)
	    printf("fs imports\n");

    shdrp = shdr;
    for (i = 0; i < ehdr->e_shnum; i++, shdrp++) {
	    if (!strcmp(&string[shdrp->sh_name], ".symtab")) {
	    rel = (Elf32_Rel *)malloc(shdrp->sh_size);
	    if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
		perror("lseek");
		exit(1);
	    }

	    read(fd, rel, shdrp->sh_size);
	    relp = rel;
	    rela = rel;
		    /* exports */
		    for (j = 0; j < shdrp->sh_size; j += sizeof(Elf32_Rel), rela = rela + 1) {
			    printf("== %08x\n", rela->r_offset);


			    if (ELF32_R_SYM(relp->r_info) == sym) {
				    printf("== SYMBOL FOUND: SEEKING TO %08x %08x\n",shdrp->sh_offset, relp->r_offset-BASE_ADDR-shdrp->sh_offset);
			    }
		    }
	    }
	if (!strcmp(&string[shdrp->sh_name], ".rel.plt")) {
	    rel = (Elf32_Rel *)malloc(shdrp->sh_size);
	    if (rel == NULL) {
		perror("malloc");
		exit(1);
	    }

//printf("SEEKIN TO %08x\n", shdrp->sh_offset);
	    if (lseek(fd, shdrp->sh_offset, SEEK_SET) != shdrp->sh_offset) {
		perror("lseek");
		exit(1);
	    }

	    if (read(fd, rel, shdrp->sh_size) != shdrp->sh_size) {
		perror("read");
		exit(1);
	    }

	    relp = rel;

printf("SO SIZE: %d\n", shdrp->sh_size);
	    for (j = 0; j < shdrp->sh_size; j += sizeof(Elf32_Rel), relp = relp + 1) {
printf("= %08x\n", relp->r_offset);
		if (ELF32_R_SYM(relp->r_info) == sym) {
printf("SYMBOL FOUND: SEEKING TO %08x %08x\n",shdrp->sh_offset, relp->r_offset-BASE_ADDR-shdrp->sh_offset);
		    if (lseek(fd, relp->r_offset-BASE_ADDR, SEEK_SET) != relp->r_offset-BASE_ADDR) {
			perror("lseek");
			exit(1);
		    }

		    if (read(fd, &plt_addr, sizeof(Elf32_Addr)) != sizeof(Elf32_Addr)) {
			perror("read@" __FILE__);
			return -1; //exit(1);
		    }
		    
		    return plt_addr-0x6;
		}
	    }
	}
    }

    return -1;
}

void
load_bin(int fd, bin_t *bin)
{
    char **sectionp;
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    int slen;
    Elf32_Shdr *strtabhdr;
    int i;

    ehdr = &bin->ehdr;

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
printf("error was here\n");
	perror("read");
	exit(1);
    }

    do_elf_checks(ehdr);

    bin->phdr = (Elf32_Phdr *)malloc(bin->plen = sizeof(Elf32_Phdr)*ehdr->e_phnum);
    if (bin->phdr == NULL) {
	perror("malloc");
	exit(1);
    }

    /* read the phdr's */

    if (lseek(fd, ehdr->e_phoff, SEEK_SET) < 0) {
	perror("lseek");
	exit(1);
    }

    if (read(fd, bin->phdr, bin->plen) != bin->plen) {
	perror("read");
	exit(1);
    }

    bin->shdr = (Elf32_Shdr *)malloc(slen = sizeof(Elf32_Shdr)*ehdr->e_shnum);
    if (bin->shdr == NULL) {
	perror("malloc");
	exit(1);
    }

    bin->section = (char **)malloc(sizeof(char **)*ehdr->e_shnum);
    if (bin->section == NULL) {
	perror("malloc");
	exit(1);
    }

    if (lseek(fd, ehdr->e_shoff, SEEK_SET) < 0) {
	perror("lseek");
	exit(1);
    }

    if (read(fd, bin->shdr, slen) != slen) {
	perror("read");
	exit(1);
    }

    strtabhdr = &bin->shdr[ehdr->e_shstrndx];

    bin->string = (char *)malloc(strtabhdr->sh_size);
    if (bin->string == NULL) {
	perror("malloc");
	exit(1);
    }

    if (lseek(fd, strtabhdr->sh_offset, SEEK_SET) != strtabhdr->sh_offset) {
	perror("lseek");
	exit(1);
    }

    if (read(fd, bin->string, strtabhdr->sh_size) != strtabhdr->sh_size) {
	perror("read");
	exit(1);
    }

    bin->bss = -1;

    for (i = 0, sectionp = bin->section, shdr = bin->shdr; i < ehdr->e_shnum; i++, sectionp++) {
	if (shdr[i].sh_type == SHT_NOBITS)
	    bin->bss = i;
	else load_section(sectionp, fd, &shdr[i]);
    }

    if (bin->bss < 0) {
	printf("No bss section\n");
	exit(1);
    }
}

int
load_section(char **section, int fd, Elf32_Shdr *shdr)
{
    if (lseek(fd, shdr->sh_offset, SEEK_SET) < 0) {
	perror("lseek");
	return 1;
    }

    *section = (char *)malloc(shdr->sh_size);
    if (*section == NULL) {
	perror("malloc");
    } else
    if (read(fd, *section, shdr->sh_size) != shdr->sh_size) {
	perror("read");
    } else
	return 0;
    return 1;
}

int
main(int argc, char *argv[])
{
    char *file;
    int fd;
    bin_t bin;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s [file]\n", argv[0]);
	return 1;
    }

    file=argv[1];
    if ((fd=open(file, O_RDONLY)) == -1) {
	fprintf(stderr, "Error: Cannot open \"%s\"\n", file);
	return 1;
    }

    load_bin(fd, &bin);

	if (rad)
		printf(" eval file.baddr=0x%08x\n", bin.phdr->p_vaddr & 0xffff0000);
	else printf("BASE ADDRESS: %08x\n", bin.phdr->p_vaddr & 0xffff0000);

	BASE_ADDR = bin.phdr->p_vaddr & 0xffff0000;

    list_sections(fd, bin.string, &bin.ehdr, bin.shdr);
    get_syms_import(fd, bin.string, &bin.ehdr, bin.shdr);

    close(fd);

    return 0;
}
