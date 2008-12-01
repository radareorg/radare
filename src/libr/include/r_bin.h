#ifndef _INCLUDE_R_BIN_H_
#define _INCLUDE_R_BIN_H_

#include "r_bin_elf.h"
#include "r_bin_elf64.h"
#include "r_bin_pe.h"

#define R_BIN_SCN_IS_EXECUTABLE(x) x & 0x1
#define R_BIN_SCN_IS_WRITABLE(x)   x & 0x2
#define R_BIN_SCN_IS_READABLE(x)   x & 0x4
#define R_BIN_SCN_IS_SHAREABLE(x)  x & 0x8

#define R_BIN_SIZEOF_NAMES 64

/* types */
typedef union {
    Elf32_r_bin_elf_obj e32;
	Elf64_r_bin_elf_obj e64;
} r_bin_elf_obj;

typedef struct {
	union {
		r_bin_elf_obj elf;
		r_bin_pe_obj pe;
	} object;
	u32 format;
	char *file;
	int fd;
} r_bin_obj;

typedef struct {
	u64 rva;
	u64 offset;
} r_bin_entry;

typedef struct {
	char name[R_BIN_SIZEOF_NAMES];
	u32 size;
	u32 vsize;
	u64 rva;
	u64 offset;
	u32 characteristics;
	int last;
} r_bin_section;

typedef struct {
	char name[R_BIN_SIZEOF_NAMES];
	char forwarder[R_BIN_SIZEOF_NAMES];
	char bind[R_BIN_SIZEOF_NAMES];
	char type[R_BIN_SIZEOF_NAMES];
	u64 rva;
	u64 offset;
	u32 size;
	u32 ordinal;
	int last;
} r_bin_symbol;

typedef struct {
	char name[R_BIN_SIZEOF_NAMES];
	char bind[R_BIN_SIZEOF_NAMES];
	char type[R_BIN_SIZEOF_NAMES];
	u64 rva;
	u64 offset;
	u32 ordinal;
	u32 hint;
	int last;
} r_bin_import;

/* bin/r_bin.c */
int r_bin_open(r_bin_obj *bin, char *file);
int r_bin_close(r_bin_obj *bin);
u64 r_bin_get_baddr(r_bin_obj *bin);
r_bin_entry* r_bin_get_entry(r_bin_obj *bin);
r_bin_section* r_bin_get_sections(r_bin_obj *bin);
r_bin_symbol* r_bin_get_symbols(r_bin_obj *bin);
r_bin_import* r_bin_get_imports(r_bin_obj *bin);

#endif
