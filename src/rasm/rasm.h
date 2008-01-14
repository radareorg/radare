
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../utils.h"
#define uchar unsigned char
int rasm_x86(off_t offset, char *str, unsigned char *data);
int rasm_arm(off_t offset, char *str, unsigned char *data);
int rasm_java(off_t offset, char *str, unsigned char *data);
int rasm_ppc(off_t offset, char *str, unsigned char *data);
