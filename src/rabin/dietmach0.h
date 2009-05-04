/* @LICENSE_START@ */
/*
 * Copyright (C) 2008 Alfredo Pesoli <revenge[AT]0xcafebabe.it>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its
 *    contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/* @LICENSE_END@ */

#if 0
//	__APPLE__
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#endif
#include <errno.h>

#define MAX_F_LENGTH 256

/* Relative position inside the original binary */
#define OFFT(offset) ( (unsigned int)offset-(unsigned int)startaddr )

/* Relative position inside the output file */
#define NOFFT(offset) ( (unsigned int)offset-(unsigned int)out )

#define TRUE 1
#define FALSE 0

#define KERN_SUCCESS 0

 /*
  * Terminal colors
  * used in bm_report()
  */
#define BRIGHT "\033[1;37m"
#define NORMAL "\033[0m"

#define SWAP_LONG(a) ( ((a) << 24) | \
                      (((a) << 8) & 0x00ff0000) | \
                      (((a) >> 8) & 0x0000ff00) | \
        ((unsigned long)(a) >> 24) )

#define FAT_MAGIC	0xcafebabe
#define FAT_CIGAM	0xbebafeca	/* NXSwapLong(FAT_MAGIC) */

#define LC_REQ_DYLD 0x80000000

/*
 * Global Vars
 *
 */
extern int swapped, fd, filesize;
extern char *filename, output[MAX_F_LENGTH], *field, *out;

extern char *fileaddr, *startaddr;

/*
 * Prototypes
 *
 */

// parser
void dm_read_header(int);
void dm_read_command(int);
void dm_read_segment(int);
void dm_read_dylinker(int);
void dm_read_section();
void dm_read_symtab(int);
void dm_read_dysymtab(int);
void dm_read_symseg(int);
void dm_read_fvmlib(int);
void dm_read_dylib(int);
void dm_read_sub_framework(int);
void dm_read_sub_umbrella(int);
void dm_read_sub_library(int);
void dm_read_twolevel_hints(int);
void dm_read_routines(int);

// utils
void *dm_allocate(size_t);
void dm_fatal(const char *, ...);
void dm_map_file(char *, int);

/* Constant for the magic field of the mach_header (32-bit architectures) */
#define	MH_MAGIC	0xfeedface	/* the mach magic number */
#define MH_CIGAM	0xcefaedfe	/* NXSwapInt(MH_MAGIC) */

#if 1
/*
 * Identify the byte order
 * of the current host.
 */
enum NXByteOrder {
  NX_UnknownByteOrder,
  NX_LittleEndian,
  NX_BigEndian
};

#ifndef u32
#define u32 unsigned long
#endif

#ifndef u64
#define u64 unsigned long long
#endif

#ifndef u8
#define u8 unsigned char
#endif

typedef unsigned int vm_offset_t;
typedef unsigned int vm_size_t;
typedef unsigned int boolean_t;
typedef int cpu_type_t;
typedef int cpu_subtype_t;
typedef int vm_prot_t;
typedef int kern_return_t;

/* Constants for the cmd field of all load commands, the type */
#define	LC_SEGMENT	0x1	/* segment of this file to be mapped */
#define	LC_SYMTAB	0x2	/* link-edit stab symbol table info */
#define	LC_SYMSEG	0x3	/* link-edit gdb symbol table info (obsolete) */
#define	LC_THREAD	0x4	/* thread */
#define	LC_UNIXTHREAD	0x5	/* unix thread (includes a stack) */
#define	LC_LOADFVMLIB	0x6	/* load a specified fixed VM shared library */
#define	LC_IDFVMLIB	0x7	/* fixed VM shared library identification */
#define	LC_IDENT	0x8	/* object identification info (obsolete) */
#define LC_FVMFILE	0x9	/* fixed VM file inclusion (internal use) */
#define LC_PREPAGE      0xa     /* prepage command (internal use) */
#define	LC_DYSYMTAB	0xb	/* dynamic link-edit symbol table info */
#define	LC_LOAD_DYLIB	0xc	/* load a dynamically linked shared library */
#define	LC_ID_DYLIB	0xd	/* dynamically linked shared lib ident */
#define LC_LOAD_DYLINKER 0xe	/* load a dynamic linker */
#define LC_ID_DYLINKER	0xf	/* dynamic linker identification */
#define	LC_PREBOUND_DYLIB 0x10	/* modules prebound for a dynamically */
				/*  linked shared library */
#define	LC_ROUTINES	0x11	/* image routines */
#define	LC_SUB_FRAMEWORK 0x12	/* sub framework */
#define	LC_SUB_UMBRELLA 0x13	/* sub umbrella */
#define	LC_SUB_CLIENT	0x14	/* sub client */
#define	LC_SUB_LIBRARY  0x15	/* sub library */
#define	LC_TWOLEVEL_HINTS 0x16	/* two-level namespace lookup hints */
#define	LC_PREBIND_CKSUM  0x17	/* prebind checksum */

/*
 * load a dynamically linked shared library that is allowed to be missing
 * (all symbols are weak imported).
 */
#define	LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)

#define	LC_SEGMENT_64	0x19	/* 64-bit segment of this file to be
				   mapped */
#define	LC_ROUTINES_64	0x1a	/* 64-bit image routines */
#define LC_UUID		0x1b	/* the uuid */
#define LC_RPATH       (0x1c | LC_REQ_DYLD)    /* runpath additions */
#define LC_CODE_SIGNATURE 0x1d	/* local of code signature */
#define LC_SEGMENT_SPLIT_INFO 0x1e /* local of info to split segments */
#define LC_REEXPORT_DYLIB (0x1f | LC_REQ_DYLD) /* load and re-export dylib */
#define	LC_LAZY_LOAD_DYLIB 0x20	/* delay load of dylib until first use */
#define	LC_ENCRYPTION_INFO 0x21	/* encrypted segment information */

#define CPU_TYPE_X86		((cpu_type_t) 7)
#define CPU_TYPE_I386		CPU_TYPE_X86		/* compatibility */
#define	CPU_TYPE_X86_64		(CPU_TYPE_X86 | CPU_ARCH_ABI64)

#ifndef _STRUCT_X86_THREAD_STATE32
#if __DARWIN_UNIX03
#define	_STRUCT_X86_THREAD_STATE32	struct __darwin_i386_thread_state
_STRUCT_X86_THREAD_STATE32
{
    unsigned int	__eax;
    unsigned int	__ebx;
    unsigned int	__ecx;
    unsigned int	__edx;
    unsigned int	__edi;
    unsigned int	__esi;
    unsigned int	__ebp;
    unsigned int	__esp;
    unsigned int	__ss;
    unsigned int	__eflags;
    unsigned int	__eip;
    unsigned int	__cs;
    unsigned int	__ds;
    unsigned int	__es;
    unsigned int	__fs;
    unsigned int	__gs;
};
#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_X86_THREAD_STATE32	struct i386_thread_state
_STRUCT_X86_THREAD_STATE32
{
    unsigned int	eax;
    unsigned int	ebx;
    unsigned int	ecx;
    unsigned int	edx;
    unsigned int	edi;
    unsigned int	esi;
    unsigned int	ebp;
    unsigned int	esp;
    unsigned int	ss;
    unsigned int	eflags;
    unsigned int	eip;
    unsigned int	cs;
    unsigned int	ds;
    unsigned int	es;
    unsigned int	fs;
    unsigned int	gs;
};
#endif /* !__DARWIN_UNIX03 */
#endif

typedef _STRUCT_X86_THREAD_STATE32 i386_thread_state_t;

/*
 * The 32-bit mach header appears at the very beginning of the object file for
 * 32-bit architectures.
 */
struct mach_header {
	u32	magic;		/* mach magic number identifier */
	cpu_type_t	cputype;	/* cpu specifier */
	cpu_subtype_t	cpusubtype;	/* machine specifier */
	u32	filetype;	/* type of file */
	u32	ncmds;		/* number of load commands */
	u32	sizeofcmds;	/* the size of all the load commands */
	u32	flags;		/* flags */
};

struct fat_header {
	u32	magic;		/* FAT_MAGIC */
	u32	nfat_arch;	/* number of structs that follow */
};

struct fat_arch {
	cpu_type_t	cputype;	/* cpu specifier (int) */
	cpu_subtype_t	cpusubtype;	/* machine specifier (int) */
	u32	offset;		/* file offset to this object file */
	u32	size;		/* size of this object file */
	u32	align;		/* alignment as a power of 2 */
};

/*
 * The 64-bit mach header appears at the very beginning of object files for
 * 64-bit architectures.
 */
struct mach_header_64 {
	u32	magic;		/* mach magic number identifier */
	cpu_type_t	cputype;	/* cpu specifier */
	cpu_subtype_t	cpusubtype;	/* machine specifier */
	u32	filetype;	/* type of file */
	u32	ncmds;		/* number of load commands */
	u32	sizeofcmds;	/* the size of all the load commands */
	u32	flags;		/* flags */
	u32	reserved;	/* reserved */
};

struct load_command {
	u32 cmd;		/* type of load command */
	u32 cmdsize;	/* total size of command in bytes */
};

union lc_str {
	u32	offset;	/* offset to the string */
#ifndef __LP64__
	char		*ptr;	/* pointer to the string */
#endif 
};

struct segment_command { /* for 32-bit architectures */
	u32	cmd;		/* LC_SEGMENT */
	u32	cmdsize;	/* includes sizeof section structs */
	char		segname[16];	/* segment name */
	u32	vmaddr;		/* memory address of this segment */
	u32	vmsize;		/* memory size of this segment */
	u32	fileoff;	/* file offset of this segment */
	u32	filesize;	/* amount to map from the file */
	vm_prot_t	maxprot;	/* maximum VM protection */
	vm_prot_t	initprot;	/* initial VM protection */
	u32	nsects;		/* number of sections in segment */
	u32	flags;		/* flags */
};

struct segment_command_64 { /* for 64-bit architectures */
	u32	cmd;		/* LC_SEGMENT_64 */
	u32	cmdsize;	/* includes sizeof section_64 structs */
	char		segname[16];	/* segment name */
	u64	vmaddr;		/* memory address of this segment */
	u64	vmsize;		/* memory size of this segment */
	u64	fileoff;	/* file offset of this segment */
	u64	filesize;	/* amount to map from the file */
	vm_prot_t	maxprot;	/* maximum VM protection */
	vm_prot_t	initprot;	/* initial VM protection */
	u32	nsects;		/* number of sections in segment */
	u32	flags;		/* flags */
};

struct section { /* for 32-bit architectures */
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	u32	addr;		/* memory address of this section */
	u32	size;		/* size in bytes of this section */
	u32	offset;		/* file offset of this section */
	u32	align;		/* section alignment (power of 2) */
	u32	reloff;		/* file offset of relocation entries */
	u32	nreloc;		/* number of relocation entries */
	u32	flags;		/* flags (section type and attributes)*/
	u32	reserved1;	/* reserved (for offset or index) */
	u32	reserved2;	/* reserved (for count or sizeof) */
};

struct section_64 { /* for 64-bit architectures */
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	u64	addr;		/* memory address of this section */
	u64	size;		/* size in bytes of this section */
	u32	offset;		/* file offset of this section */
	u32	align;		/* section alignment (power of 2) */
	u32	reloff;		/* file offset of relocation entries */
	u32	nreloc;		/* number of relocation entries */
	u32	flags;		/* flags (section type and attributes)*/
	u32	reserved1;	/* reserved (for offset or index) */
	u32	reserved2;	/* reserved (for count or sizeof) */
	u32	reserved3;	/* reserved */
};

struct fvmlib {
	union lc_str	name;		/* library's target pathname */
	u32	minor_version;	/* library's minor version number */
	u32	header_addr;	/* library's header address */
};

struct fvmlib_command {
	u32	cmd;		/* LC_IDFVMLIB or LC_LOADFVMLIB */
	u32	cmdsize;	/* includes pathname string */
	struct fvmlib	fvmlib;		/* the library identification */
};

struct dylib {
    union lc_str  name;			/* library's path name */
    u32 timestamp;			/* library's build time stamp */
    u32 current_version;		/* library's current version number */
    u32 compatibility_version;	/* library's compatibility vers number*/
};

struct dylib_command {
	u32	cmd;		/* LC_ID_DYLIB, LC_LOAD_{,WEAK_}DYLIB,
					   LC_REEXPORT_DYLIB */
	u32	cmdsize;	/* includes pathname string */
	struct dylib	dylib;		/* the library identification */
};

struct sub_framework_command {
	u32	cmd;		/* LC_SUB_FRAMEWORK */
	u32	cmdsize;	/* includes umbrella string */
	union lc_str 	umbrella;	/* the umbrella framework name */
};

struct sub_client_command {
	u32	cmd;		/* LC_SUB_CLIENT */
	u32	cmdsize;	/* includes client string */
	union lc_str 	client;		/* the client name */
};

struct sub_umbrella_command {
	u32	cmd;		/* LC_SUB_UMBRELLA */
	u32	cmdsize;	/* includes sub_umbrella string */
	union lc_str 	sub_umbrella;	/* the sub_umbrella framework name */
};

struct sub_library_command {
	u32	cmd;		/* LC_SUB_LIBRARY */
	u32	cmdsize;	/* includes sub_library string */
	union lc_str 	sub_library;	/* the sub_library name */
};

struct prebound_dylib_command {
	u32	cmd;		/* LC_PREBOUND_DYLIB */
	u32	cmdsize;	/* includes strings */
	union lc_str	name;		/* library's path name */
	u32	nmodules;	/* number of modules in library */
	union lc_str	linked_modules;	/* bit vector of linked modules */
};

struct dylinker_command {
	u32	cmd;		/* LC_ID_DYLINKER or LC_LOAD_DYLINKER */
	u32	cmdsize;	/* includes pathname string */
	union lc_str    name;		/* dynamic linker's path name */
};

struct thread_command {
	u32	cmd;		/* LC_THREAD or  LC_UNIXTHREAD */
	u32	cmdsize;	/* total size of this command */
	/* u32 flavor		   flavor of thread state */
	/* u32 count		   count of longs in thread state */
	/* struct XXX_thread_state state   thread state for this flavor */
	/* ... */
};

struct routines_command { /* for 32-bit architectures */
	u32	cmd;		/* LC_ROUTINES */
	u32	cmdsize;	/* total size of this command */
	u32	init_address;	/* address of initialization routine */
	u32	init_module;	/* index into the module table that */
				        /*  the init routine is defined in */
	u32	reserved1;
	u32	reserved2;
	u32	reserved3;
	u32	reserved4;
	u32	reserved5;
	u32	reserved6;
};

struct routines_command_64 { /* for 64-bit architectures */
	u32	cmd;		/* LC_ROUTINES_64 */
	u32	cmdsize;	/* total size of this command */
	u64	init_address;	/* address of initialization routine */
	u64	init_module;	/* index into the module table that */
					/*  the init routine is defined in */
	u64	reserved1;
	u64	reserved2;
	u64	reserved3;
	u64	reserved4;
	u64	reserved5;
	u64	reserved6;
};

struct symtab_command {
	u32	cmd;		/* LC_SYMTAB */
	u32	cmdsize;	/* sizeof(struct symtab_command) */
	u32	symoff;		/* symbol table offset */
	u32	nsyms;		/* number of symbol table entries */
	u32	stroff;		/* string table offset */
	u32	strsize;	/* string table size in bytes */
};

struct dysymtab_command {
    u32 cmd;	/* LC_DYSYMTAB */
    u32 cmdsize;	/* sizeof(struct dysymtab_command) */

    /*
     * The symbols indicated by symoff and nsyms of the LC_SYMTAB load command
     * are grouped into the following three groups:
     *    local symbols (further grouped by the module they are from)
     *    defined external symbols (further grouped by the module they are from)
     *    undefined symbols
     *
     * The local symbols are used only for debugging.  The dynamic binding
     * process may have to use them to indicate to the debugger the local
     * symbols for a module that is being bound.
     *
     * The last two groups are used by the dynamic binding process to do the
     * binding (indirectly through the module table and the reference symbol
     * table when this is a dynamically linked shared library file).
     */
    u32 ilocalsym;	/* index to local symbols */
    u32 nlocalsym;	/* number of local symbols */

    u32 iextdefsym;/* index to externally defined symbols */
    u32 nextdefsym;/* number of externally defined symbols */

    u32 iundefsym;	/* index to undefined symbols */
    u32 nundefsym;	/* number of undefined symbols */

    /*
     * For the for the dynamic binding process to find which module a symbol
     * is defined in the table of contents is used (analogous to the ranlib
     * structure in an archive) which maps defined external symbols to modules
     * they are defined in.  This exists only in a dynamically linked shared
     * library file.  For executable and object modules the defined external
     * symbols are sorted by name and is use as the table of contents.
     */
    u32 tocoff;	/* file offset to table of contents */
    u32 ntoc;	/* number of entries in table of contents */

    /*
     * To support dynamic binding of "modules" (whole object files) the symbol
     * table must reflect the modules that the file was created from.  This is
     * done by having a module table that has indexes and counts into the merged
     * tables for each module.  The module structure that these two entries
     * refer to is described below.  This exists only in a dynamically linked
     * shared library file.  For executable and object modules the file only
     * contains one module so everything in the file belongs to the module.
     */
    u32 modtaboff;	/* file offset to module table */
    u32 nmodtab;	/* number of module table entries */

    /*
     * To support dynamic module binding the module structure for each module
     * indicates the external references (defined and undefined) each module
     * makes.  For each module there is an offset and a count into the
     * reference symbol table for the symbols that the module references.
     * This exists only in a dynamically linked shared library file.  For
     * executable and object modules the defined external symbols and the
     * undefined external symbols indicates the external references.
     */
    u32 extrefsymoff;	/* offset to referenced symbol table */
    u32 nextrefsyms;	/* number of referenced symbol table entries */

    /*
     * The sections that contain "symbol pointers" and "routine stubs" have
     * indexes and (implied counts based on the size of the section and fixed
     * size of the entry) into the "indirect symbol" table for each pointer
     * and stub.  For every section of these two types the index into the
     * indirect symbol table is stored in the section header in the field
     * reserved1.  An indirect symbol table entry is simply a 32bit index into
     * the symbol table to the symbol that the pointer or stub is referring to.
     * The indirect symbol table is ordered to match the entries in the section.
     */
    u32 indirectsymoff; /* file offset to the indirect symbol table */
    u32 nindirectsyms;  /* number of indirect symbol table entries */

    /*
     * To support relocating an individual module in a library file quickly the
     * external relocation entries for each module in the library need to be
     * accessed efficiently.  Since the relocation entries can't be accessed
     * through the section headers for a library file they are separated into
     * groups of local and external entries further grouped by module.  In this
     * case the presents of this load command who's extreloff, nextrel,
     * locreloff and nlocrel fields are non-zero indicates that the relocation
     * entries of non-merged sections are not referenced through the section
     * structures (and the reloff and nreloc fields in the section headers are
     * set to zero).
     *
     * Since the relocation entries are not accessed through the section headers
     * this requires the r_address field to be something other than a section
     * offset to identify the item to be relocated.  In this case r_address is
     * set to the offset from the vmaddr of the first LC_SEGMENT command.
     * For MH_SPLIT_SEGS images r_address is set to the the offset from the
     * vmaddr of the first read-write LC_SEGMENT command.
     *
     * The relocation entries are grouped by module and the module table
     * entries have indexes and counts into them for the group of external
     * relocation entries for that the module.
     *
     * For sections that are merged across modules there must not be any
     * remaining external relocation entries for them (for merged sections
     * remaining relocation entries must be local).
     */
    u32 extreloff;	/* offset to external relocation entries */
    u32 nextrel;	/* number of external relocation entries */

    /*
     * All the local relocation entries are grouped together (they are not
     * grouped by their module since they are only used if the object is moved
     * from it staticly link edited address).
     */
    u32 locreloff;	/* offset to local relocation entries */
    u32 nlocrel;	/* number of local relocation entries */

};	

struct dylib_module {
    u32 module_name;	/* the module name (index into string table) */

    u32 iextdefsym;	/* index into externally defined symbols */
    u32 nextdefsym;	/* number of externally defined symbols */
    u32 irefsym;		/* index into reference symbol table */
    u32 nrefsym;		/* number of reference symbol table entries */
    u32 ilocalsym;		/* index into symbols for local symbols */
    u32 nlocalsym;		/* number of local symbols */

    u32 iextrel;		/* index into external relocation entries */
    u32 nextrel;		/* number of external relocation entries */

    u32 iinit_iterm;	/* low 16 bits are the index into the init
				   section, high 16 bits are the index into
			           the term section */
    u32 ninit_nterm;	/* low 16 bits are the number of init section
				   entries, high 16 bits are the number of
				   term section entries */

    u32			/* for this module address of the start of */
	objc_module_info_addr;  /*  the (__OBJC,__module_info) section */
    u32			/* for this module size of */
	objc_module_info_size;	/*  the (__OBJC,__module_info) section */
};	

struct dylib_module_64 {
    u32 module_name;	/* the module name (index into string table) */

    u32 iextdefsym;	/* index into externally defined symbols */
    u32 nextdefsym;	/* number of externally defined symbols */
    u32 irefsym;		/* index into reference symbol table */
    u32 nrefsym;		/* number of reference symbol table entries */
    u32 ilocalsym;		/* index into symbols for local symbols */
    u32 nlocalsym;		/* number of local symbols */

    u32 iextrel;		/* index into external relocation entries */
    u32 nextrel;		/* number of external relocation entries */

    u32 iinit_iterm;	/* low 16 bits are the index into the init
				   section, high 16 bits are the index into
				   the term section */
    u32 ninit_nterm;      /* low 16 bits are the number of init section
				  entries, high 16 bits are the number of
				  term section entries */

    u32			/* for this module size of */
        objc_module_info_size;	/*  the (__OBJC,__module_info) section */
    u64			/* for this module address of the start of */
        objc_module_info_addr;	/*  the (__OBJC,__module_info) section */
};

struct twolevel_hints_command {
    u32 cmd;	/* LC_TWOLEVEL_HINTS */
    u32 cmdsize;	/* sizeof(struct twolevel_hints_command) */
    u32 offset;	/* offset to the hint table */
    u32 nhints;	/* number of hints in the hint table */
};

struct twolevel_hint {
    u32 
	isub_image:8,	/* index into the sub images */
	itoc:24;	/* index into the table of contents */
};

struct prebind_cksum_command {
    u32 cmd;	/* LC_PREBIND_CKSUM */
    u32 cmdsize;	/* sizeof(struct prebind_cksum_command) */
    u32 cksum;	/* the check sum or zero */
};

struct uuid_command {
    u32	cmd;		/* LC_UUID */
    u32	cmdsize;	/* sizeof(struct uuid_command) */
    u8	uuid[16];	/* the 128-bit uuid */
};

struct rpath_command {
    u32	 cmd;		/* LC_RPATH */
    u32	 cmdsize;	/* includes string */
    union lc_str path;		/* path to add to run path */
};

struct linkedit_data_command {
    u32	cmd;		/* LC_CODE_SIGNATURE or LC_SEGMENT_SPLIT_INFO */
    u32	cmdsize;	/* sizeof(struct linkedit_data_command) */
    u32	dataoff;	/* file offset of data in __LINKEDIT segment */
    u32	datasize;	/* file size of data in __LINKEDIT segment  */
};

struct encryption_info_command {
   u32	cmd;		/* LC_ENCRYPTION_INFO */
   u32	cmdsize;	/* sizeof(struct encryption_info_command) */
   u32	cryptoff;	/* file offset of encrypted range */
   u32	cryptsize;	/* file size of encrypted range */
   u32	cryptid;	/* which enryption system,
				   0 means not-encrypted yet */
};

struct symseg_command {
	u32	cmd;		/* LC_SYMSEG */
	u32	cmdsize;	/* sizeof(struct symseg_command) */
	u32	offset;		/* symbol segment offset */
	u32	size;		/* symbol segment size in bytes */
};

struct ident_command {
	u32 cmd;		/* LC_IDENT */
	u32 cmdsize;	/* strings that follow this command */
};

struct fvmfile_command {
	u32 cmd;			/* LC_FVMFILE */
	u32 cmdsize;		/* includes pathname string */
	union lc_str	name;		/* files pathname */
	u32	header_addr;	/* files virtual address */
};
#endif

/* Compatibility with Leopard */
#if __DARWIN_UNIX03
#define eax    __eax
#define ebx    __ebx
#define ecx    __ecx
#define edx    __edx
#define edi    __edi
#define esi    __esi
#define ebp    __ebp
#define esp    __esp
#define ss     __ss
#define eflags __eflags
#define eip    __eip
#define cs     __cs
#define ds     __ds
#define es     __es
#define fs     __fs
#define gs     __gs
#endif

/*
 * Struct definitions
 */
struct fat_header               fat_header;
struct mach_header              m_header;
struct fat_arch                 *archs;
struct load_command             *l_command;
struct segment_command          *seg_command;
struct dylinker_command         *dy_command;
struct section                  *sect;
struct symtab_command           *sy_command;
struct dysymtab_command         *dysym_command;
struct symseg_command           *symseg_command;
struct fvmlib_command           *fvm_command;
struct dylib_command            *dyl_command;
struct sub_framework_command    *sub_framework;
struct sub_umbrella_command     *sub_umbrella;
struct sub_library_command      *sub_library;
struct sub_client_command       *sub_client;
struct prebound_dylib_command   *prebound_dy_command;
struct thread_command           *thr_command;
struct twolevel_hints_command   *tlh_command;
struct routines_command         *routines_command;
struct uuid_command             *uuid;
struct linkedit_data_command    *ld;
