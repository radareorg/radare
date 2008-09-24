/*
 *     DTDumper
 *     Julien TINNES <julien at cr0.org>
 *
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdio.h>
#include <sys/types.h>
/* This is quite hackish ;) */
#ifndef BSD_SOURCE
#include <stdint.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define PAGE_ALIGN(a) (a&(~((1<<12)-1)))
#define PAGE_OFFSET(a) (a&((1<<12)-1))

#ifdef _LARGEFILE64_SOURCE
#define lseek lseek64
#endif

#ifdef BSD_SOURCE
#define off64_t off_t
#endif

#define _FILE_OFFSET_BITS 64

struct vmdt {
	int gdtla;
	int idtla;
	char *version;
} vminfos[]={
	{0xFFC05000, 0xFFC6A370, "VMware 3.2"},
	{0xFFC07000, 0xFFC17800, "VMware 4.0"},
	{0xFFC07C00, 0xFFC18000, "VMware 4.5 Windows"},
	{0xFFC07C80, 0xFFC18000, "VMware 5.0/5.5 Windows"},
	{0xFFC075A0, 0xFFC18000, "VMware 5.0.0 (13124) Linux"},
	{0xFFC07000, 0xFFC18000, "VMware GSX 3.1 Linux"},
	{0xFFC07E00, 0xFFC18000, "VMware GSX 3.1 Linux"},
	{0xFFC07880, 0xFFC18000, "VMware Player 1.0.1 Linux"},
	{0xFF400000, 0xFC571C20, "Xen-2.0.7 (dom0 or domU)"},
	{0xF903F800, 0xF903F000, "Kqemu 0.7.2"},
	{0xE80B6C08, 0xE80B6408, "MS Virtual PC 2004"}
};

/* Thanks to Nicolas Ruff, Jean-David Silberzahn, Raphael Rigo, Julien Raeis, solar and pipacs for testing dtdumper
 * and sending the above signatures.
 */

struct dt {
	uint16_t size;
	uint32_t la;
} __attribute__((packed)) gdtr, ldtr, idtr, ldtr_internal, tr;
/* NB: sldt and str instruction won't return the whole segment descriptor but only a segment selector (in GDT or LDT) */


//(mydesc->base3<<24)+(mydesc->base2<<16)+(mydesc->base1), (((mydesc->limit2<<16)+mydesc->limit1)<<(mydesc->g?12:0))+mydesc->g*0xFFF)
#define SD_BIG_BASE(sddesc)  (((sddesc)->base3<<24)+((sddesc)->base2<<16)+((sddesc)->base1))
#define SD_BIG_LIMIT(sddesc) (((((sddesc)->limit2<<16)+(sddesc)->limit1)<<((sddesc)->g?12:0))+(sddesc)->g*0xFFF)
/* non system descriptor type field */
#define NSD_TYPE_CODE 1<<3
/* data segment */
#define NSD_DATA_ED 1<<2	/* expand down */
#define NSD_DATA_W 1<<1		/* write */
#define NSD_DATA_A 1<<0		/* accessed */
/* code segment */
#define NSD_CODE_C 1<<2		/* conforming */
#define NSD_CODE_R 1<<1		/* read */
#define NSD_CODE_A 1<<0		/* accessed */

/* check if segment descriptor can exist in IA32 mode */
#define SD_IS_IA32(sdesc) (sdesc->l == 0)
/* for IA32 segments */
#define SD_IS_SYSTEM(sdesc) (sdesc->s == 0)

/* for system segments and gates descriptor */
/* TSS */
#define SS_IS_TSS(ssdesc) ((((ssdesc->type)&(1<<2)) == 0) && ((ssdesc->type)&1) && (ssdesc->db == 0)) /* NB TSS is always in GDT */
/* LDT */
#define SS_IS_LDT(ssdesc) (ssdesc->type==2)
/* 32 bits Call-Gate */
#define SS_IS_32CG(ssdesc) ( (((struct cgdesc*)ssdesc)->type==12) && (((struct cgdesc*)ssdesc)->zero == 0) )
/* Task-Gate */
#define SS_IS_TG(ssdesc) ( (((struct tgdesc*)ssdesc)->type == 5) )
/* Interrupt Gate */
#define SS_IS_IG(ssdesc) ( !((((struct igdesc*)ssdesc)->type ^ 6)&7) )
/* Trap Gate */
#define SS_IS_TRG(ssdesc) ( !((((struct igdesc*)ssdesc)->type ^ 7)&7) )

#define CG_BIG_OFFSET(ssdesc)	( ((ssdesc)->offset2<<16)+((ssdesc)->offset1) )
#define IG_BIG_OFFSET(ssdesc)	( ((ssdesc)->offset2<<16)+((ssdesc)->offset1) )


/* define generic segment descriptor structure */
struct sdesc {
	uint32_t limit1:16;
	uint32_t base1:16;
	uint32_t base2:8;
	uint32_t type:4;
	uint32_t s:1;
	uint32_t dpl:2;
	uint32_t p:1;
	uint32_t limit2:4;
	uint32_t avl:1;
	uint32_t l:1;
	uint32_t db:1;
	uint32_t g:1;
	uint32_t base3:8;
};

/* Call-Gate descriptor */
struct cgdesc {
	uint32_t offset1:16;
	uint32_t selector:16;
	uint32_t pc:5;		/* param. count */
	uint32_t zero:3;	/* must be zero */
	uint32_t type:4;	/* 0b1100 */
	uint32_t s:1;		/* must be zero */
	uint32_t dpl:2;
	uint32_t p:1;
	uint32_t offset2:16;
};

/* Task Gate Descriptor */

struct tgdesc {
	uint32_t unused1:16;
	uint32_t selector:16;	/* TSS Segment selector */
	uint32_t unused2:8;
	uint32_t type:4;
	uint32_t s:1;
	uint32_t dpl:2;
	uint32_t p:1;
	uint32_t unused3:8;
};

/* Interrupt-Gate and Trap-Gate descriptor */
struct igdesc {
	uint32_t offset1:16;
	uint32_t selector:16;
	uint32_t unused1:5;
	uint32_t zero:3;	/* must be zero */
	uint32_t type:4;	/* 0b1100 */
	uint32_t s:1;		/* must be zero */
	uint32_t dpl:2;
	uint32_t p:1;
	uint32_t offset2:16;
};

union gdesc {
	struct sdesc s;
	struct cgdesc cg;
	struct tgdesc tg;
	struct igdesc ig;
};

/* system segment and gates types */
char *ss_types[]={"Reserved","16bit TSS (Avail)", "LDT", "16bit TSS (Busy)", "16bit Call-Gate", "Task Gate", "16bit Interrupt Gate", "16bit Trap Gate",
		"Reserved", "32bit TSS (Avail)", "Reserved", "32bit TSS (Busy)", "32bit Call-Gate", "Reserved", "32bit Interrupt Gate", "32bit Trap Gate"};

/* Print a segment descriptor, assuming we're in IA32 mode */
/* selector is the selector without the DPL when applicable (GDT or LDT) or -1 */
void	sd_print(union gdesc *desc, int selector) {

	struct sdesc *mydesc=&(desc->s);
#define mycgate ((struct cgdesc *)mydesc)
#define mytgate ((struct tgdesc *)mydesc)
#define myigate ((struct igdesc *)mydesc)

	int i;
	char	type[4];
	type[3]=0;
	if (!SD_IS_IA32(mydesc)) {
		printf("ERROR: 64 bits segment but not in IA32e mode!\n");
		return;
	} else {

		/* print segment selector */
		if (selector != -1)
			printf("0x%.2X ", selector+mydesc->dpl);
		else
			printf("N/A  ");

		for (i=0; i<8; i++)
			printf("%.2X", ((char *)mydesc)[7-i]&0xFF);
		printf("  ");

		if (!SD_IS_SYSTEM(mydesc)) {
			/* non system segment, Code or Data */
			if (mydesc->type & NSD_TYPE_CODE) {
				printf("Code  ");
				type[0]=(mydesc->type & NSD_CODE_C)?'C':'c';
				type[1]=(mydesc->type & NSD_CODE_R)?'R':'r';
				type[2]=(mydesc->type & NSD_CODE_A)?'A':'a';
			} else {
				printf("Data  ");
				type[0]=(mydesc->type & NSD_DATA_ED)?'E':'e';
				type[1]=(mydesc->type & NSD_DATA_W)?'W':'w';
				type[2]=(mydesc->type & NSD_DATA_A)?'A':'a';
			}

			printf("D/B=%s  AVL=%d  %s  DPL=%d  TYPE= %s  BASE=0x%.8X  LIMIT=0x%.8X\n",mydesc->db ? "32bts":"16bts", mydesc->avl, mydesc->p?"Present":"NtPrsnt",  mydesc->dpl, type, SD_BIG_BASE(mydesc), SD_BIG_LIMIT(mydesc));
		} else {
			/* system descriptor (system segments or Gates) */
			if (SS_IS_TSS(mydesc)) {
				/* TSS */
				printf("%20.20s", ss_types[mydesc->type]);
				printf("  AVL=%d  %s  DPL=%d BASE=0x%.8X  LIMIT=0x%.8X\n", mydesc->avl, mydesc->p?"Present":"NtPrsnt",  mydesc->dpl, SD_BIG_BASE(mydesc), SD_BIG_LIMIT(mydesc));
			} else if (SS_IS_LDT(mydesc)) {
				/* LDT */
				printf("%20.20s", ss_types[mydesc->type]);
				printf("    %s  DPL=%d  BASE=0x%.8X  LIMIT=0x%.8X\n", mydesc->p?"Present":"NtPrsnt",  mydesc->dpl, SD_BIG_BASE(mydesc), SD_BIG_LIMIT(mydesc));
			} else if (SS_IS_32CG(mydesc)) {
				/* 32 bits Call-Gate */
				printf("%20.20s    %s  DPL=%d  SgmtSlctr=0x%X (%s)  ParamCount=%d  Offset=0x%.8X\n",ss_types[mycgate->type], mycgate->p?"Present":"NtPrsnt", mycgate->dpl, mycgate->selector, (mycgate->selector)&1<<2?"LDT":"GDT", mycgate->pc, CG_BIG_OFFSET(mycgate));

			} else if (SS_IS_TG(mydesc)) {
				/* Task Gate */
				printf("%20.20s    %s  DPL=%d  TSS SgmtSlctr=0x%X (%s) \n",ss_types[mytgate->type], mytgate->p?"Present":"NtPrsnt", mytgate->dpl, mytgate->selector, (mytgate->selector)&1<<2?"ERROR":"GDT");
			} else if (SS_IS_IG(mydesc)) {
				printf("%20.20s    %s  DPL=%d  SgmtSlctr=0x%X (%s)  Offset=0x%.8X\n",ss_types[myigate->type], myigate->p?"Present":"NtPrsnt", myigate->dpl, myigate->selector, (myigate->selector)&1<<2?"LDT":"GDT", IG_BIG_OFFSET(myigate));

			} else if (SS_IS_TRG(mydesc)) {
				printf("%20.20s    %s  DPL=%d  SgmtSlctr=0x%X (%s)  Offset=0x%.8X\n",ss_types[myigate->type], myigate->p?"Present":"NtPrsnt", myigate->dpl, myigate->selector, (myigate->selector)&1<<2?"LDT":"GDT", IG_BIG_OFFSET(myigate));

			} else {
				printf("%20.20s\n", ss_types[mydesc->type]);
			}
		}
	}
}

void	*map_dt(int do_mmap, int fd, size_t length, off64_t offset) {
	
	void *ret;
	off_t seekret;
	if (do_mmap) {
		ret=mmap(0, length, PROT_READ, MAP_PRIVATE, fd, PAGE_ALIGN(offset));
		if (ret == MAP_FAILED) {
			perror("mmap(): ");
			printf("Try with -s\n");
			exit(1);
		}

		ret+=PAGE_OFFSET(offset);

	} else {
		size_t len=length;
		ssize_t cc;
		void *buf;

		buf=ret=malloc(len);

		if (ret == NULL) {
			perror("malloc(): ");
			exit(1);
		}
		seekret=lseek(fd, offset, SEEK_SET);
		if (seekret == -1) {
			perror("lseek(): ");
			exit(1);
		}	
		while (len > 0) {
			cc=read(fd, buf, len);
			if (cc < 0) {
				perror("read(): ");
				exit(1);
			}
			if (cc == 0) {
				printf("Got EOF after %d bytes\n", gdtr.size+1 - len);
				exit(1);
			}
			len-=cc;
			buf=buf + cc;
		}
	}

	return ret;
}

int	main(int argc, char *argv[]) {
	
	int	kmemfd;
	void	*gdt, *ldt, *idt;
	struct sdesc *ldtdesc;
	short	cs, ds, ss, es;
	int	c, i;
	int  do_mmap=1; 	/* 0 will seek and read (more compatible), 1 will mmap */

	int	pgdt=0;
	int	pldt=0;
	int	pidt=0;
	

	printf("\nDT Dumper v0.1.3\n"
	       "(C) Julien TINNES <julien at cr0.org>\n\n");
	
	for (;;) {
		int option_index=0;
		static struct option long_options[] = {
			{"seek", 0, 0, 's'},
			{"gdt", 0, 0, 'g'},
			{"ldt", 0, 0, 'l'},
			{"idt", 0, 0, 'i'},
			{"help", 0, 0, 'h'},
			{0,0,0,0}
		};

		c=getopt_long(argc, argv, "hilgs", long_options,  &option_index);
		if (c == -1)
			break;
		switch (c) {

			case 0:
				printf("This should not happen\n");
				exit(1);
			case 'i':
				pidt=1;
				break;
			case 'g':
				pgdt=1;
				break;
			case 'l':
				pldt=1;
				break;
			case 's':
				do_mmap=0;
				break;
			case '?':
			case 'h':
			default:
				printf("Usage: %s [options]\n\n"
					"-g\t Dump GDT\n"
					"-l\t Dump LDT\n"
					"-i\t Dump IDT\n"
					"-s\t Use seek() and read() instead of mmap()\n", argv[0]);
				exit(1);
		}
	}
					
	asm("sgdt %0\t\n"
	    "sldt %1\t\n"
	    "sidt %2\t\n"
	    "str  %3\t\n"
	    "movw %%cs, %4\t\n"
	    "movw %%ds, %5\t\n"
	    "movw %%ss, %6\t\n"
	    "movw %%es, %7\t\n"
	    : "=m" (gdtr), "=m" (ldtr), "=m" (idtr), "=m" (tr), "=r" (cs), "=r" (ds), "=S" (ss), "=D" (es) );

	printf("GDT size: 0x%X (%d entries), GDT LA: 0x%X\n"
	       "LDT's selector is 0x%X (entry #%d in %s)\n"
	       "TSS's selector is 0x%X (entry #%d in %s)\n"
	       "IDT size: 0x%X (%d entries), IDT LA: 0x%X\n"
	       "CS: 0x%X\t"
	       "DS: 0x%X\t"
	       "SS: 0x%X\t"
	       "ES: 0x%X\n",
	       gdtr.size, (gdtr.size+1)/8, gdtr.la,  
	       ldtr.size, ldtr.size>>3, ldtr.size&(1<<2)?"LDT":"GDT",
	       tr.size, tr.size >>3, tr.size&(1<<2)?"LDT":"GDT",
	       idtr.size, (idtr.size+1)/8, idtr.la,
	       cs, ds, ss, es);

	if ((gdtr.la & 0xE0000000) == 0xE0000000) {
		for (i=0; i < (sizeof(vminfos)/sizeof(*vminfos)); i++)
			if (gdtr.la == vminfos[i].gdtla) {
				printf("You seem do be running %s, userland dtdumper will not work\n", vminfos[i].version);
				break;
			}
		if (i == (sizeof(vminfos)/sizeof(*vminfos))) {
			printf("You're probably running an unknown version of VMware/Xen/Kqemu, please report your\n"
					"output and VMware/Xen/Kqemu version to <julien at cr0.org>\n");
			exit(1);
		}
	}

	if (argc == 1) {
		printf("Use %s --help for help\n", argv[0]);
		exit(1);
	}	
				

	kmemfd=open("/dev/kmem", O_RDONLY );

	if(kmemfd == -1) {
		perror("open(): ");
		exit(1);
	}

	gdt=map_dt(do_mmap, kmemfd, gdtr.size+1, gdtr.la);

	if (ldtr.size != 0) {
		/* use LDT selector to find LDT descriptor in GDT */
		ldtdesc=(struct sdesc*)gdt+(ldtr.size>>3);
		ldtr_internal.size=SD_BIG_LIMIT(ldtdesc);
		ldtr_internal.la=SD_BIG_BASE(ldtdesc);

		if (!(SD_IS_IA32(ldtdesc) && SD_IS_SYSTEM(ldtdesc) && SS_IS_LDT(ldtdesc))) {
			printf("Error, Invalid LDT, something wrong, try changing maping method and/or device\n");
			exit(1);
		}
		ldt=map_dt(do_mmap, kmemfd, ldtr_internal.size+1, ldtr_internal.la);
	}
	
	idt=map_dt(do_mmap, kmemfd, idtr.size+1, idtr.la);

	if (pgdt) {
		printf("Dumping GDT (0x%X)\n", gdtr.la);
		for (i=0; i< (gdtr.size+1)/8; i++) {
			union gdesc *sd;
			sd=((union gdesc*)gdt)+i;
			if (sd->s.s != 100) {
				printf("(#%3.3d) ", i);
				sd_print(sd, i<<3);
			}
		}
	}
	if (pldt) {
		if (ldtr.size == 0) 
			printf("No LDT present\n");
		else {
			printf("Dumping LDT (0x%X)\n", ldtr_internal.la);
			for (i=0; i<  (ldtr_internal.size+1)/8; i++) {
				union gdesc *sd;
				sd=((union gdesc*)ldt)+i;
				printf("(#%3.3d) ", i);
				sd_print(sd, (i<<3)+(1<<2));
			}
		}
	}
	if (pidt) {
		printf("Dumping IDT (0x%X)\n", idtr.la);
		for (i=0; i<  (idtr.size+1)/8; i++) {
			union gdesc *sd;
			sd=((union gdesc*)idt)+i;
			printf("(#%3.3d) ", i);
			sd_print(sd, -1);
		}
	}

	return 0;
}
