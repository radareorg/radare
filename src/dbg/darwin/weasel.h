#ifndef WEASEL_H
#define WEASEL_H

#include <mach/boolean.h>
#include <mach/message.h>
#include "disasm.h"

#define PC 15

typedef signed int s32;
typedef unsigned int u32;

char *disassemble(u32 instr, u32 addr);

/* This is a Mach library function, but it's not defined in any header! Bad
 * Apple. */
boolean_t exc_server(mach_msg_header_t *in, mach_msg_header_t *out);

#endif

