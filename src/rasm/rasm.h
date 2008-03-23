
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include "../main.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../utils.h"
#include "../radare.h"

int rasm_x86(u64 offset, const char *str, u8 *data);
int rasm_arm(u64 offset, const char *str, u8 *data);
int rasm_java(u64 offset, const char *str, u8 *data);
int rasm_ppc(u64 offset, const char *str, u8 *data);

/* assemble disassemble list .. */
int rasm_asm(const char *arch, u64 offset, const char *str, unsigned char *data);
int rasm_directive(u64 *offset, const char *str, u8 *data);
int rasm_file(const char *arch, u64 offset, const char *file, const char *ofile);
int rasm_show_list();
