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

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <errno.h>

#define MAX_F_LENGTH 256

/* Relative position inside the original binary */
#define OFFT(offset) ( (unsigned int)offset-(unsigned int)startaddr )

/* Relative position inside the output file */
#define NOFFT(offset) ( (unsigned int)offset-(unsigned int)out )

#define TRUE 1
#define FALSE 0

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

/*
 * Global Vars
 *
 */
int swapped, fd, filesize;
char *filename, output[MAX_F_LENGTH], *fileaddr, *field, *startaddr, *out;


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


/*
 * Struct Definitions
 *
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
