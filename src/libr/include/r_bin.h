#ifndef _LIB_R_BIN_H_
#define _LIB_R_BIN_H_

#include "r_bin_dietelf.h"
#include "r_bin_dietelf64.h"
#include "r_bin_dietpe.h"

#define BIN_SCN_IS_EXECUTABLE(x) x & 0x1
#define BIN_SCN_IS_WRITABLE(x)   x & 0x2
#define BIN_SCN_IS_READABLE(x)   x & 0x4
#define BIN_SCN_IS_SHAREABLE(x)  x & 0x8

#define SIZEOF_NAMES 64

/* types */
typedef union {
    Elf32_dietelf_bin_t e32;
    Elf64_dietelf_bin_t e64;
} dietelf_bin;

typedef struct {
	union {
		dietelf_bin elf;
		dietpe_bin pe;
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
	char name[SIZEOF_NAMES];
	u32 size;
	u32 vsize;
	u64 rva;
	u64 offset;
	u32 characteristics;
	int last;
} r_bin_section;

typedef struct {
	char name[SIZEOF_NAMES];
	char forwarder[SIZEOF_NAMES];
	char bind[SIZEOF_NAMES];
	char type[SIZEOF_NAMES];
	u64 rva;
	u64 offset;
	u32 size;
	u32 ordinal;
	int last;
} r_bin_symbol;

typedef struct {
	char name[SIZEOF_NAMES];
	char bind[SIZEOF_NAMES];
	char type[SIZEOF_NAMES];
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
