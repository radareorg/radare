
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include "../main.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../utils.h"
#include "../radare.h"

int rasm_x86(ut64 offset, const char *str, u8 *data);
int rasm_nasm_x86(ut64 offset, const char *str,u8 *data);
int rasm_olly_x86(ut64 offset, const char *str, u8 *data);
int rasm_arm(ut64 offset, const char *str, u8 *data);
int rasm_java(ut64 offset, const char *str, u8 *data);
int rasm_ppc(ut64 offset, const char *str, u8 *data);
int rasm_rsc(ut64 offset, const char *str, unsigned char *data);

/* assemble disassemble list .. */
int rasm_asm(const char *arch, ut64 *offset, const char *str, unsigned char *data);
int rasm_disasm(const char *arch, ut64 *offset, const char *str, unsigned char *data);
int rasm_directive(const char *arch, ut64 *offset, const char **str, u8 *data);
int rasm_file(const char *arch, ut64 offset, const char *file, const char *ofile);
int rasm_show_list();
