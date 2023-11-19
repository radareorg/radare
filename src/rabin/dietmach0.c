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

#if 0
//__APPLE__
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/swap.h>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach/i386/thread_state.h>
#include <mach/machine/thread_status.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "dietmach0_errors.h"
#include "dietmach0.h"
int swapped, fd, filesize;
char *filename, output[MAX_F_LENGTH], *field, *out;

char *fileaddr, *startaddr;

// XXX This must be ut64 !!!!
static void *offset, *toff, *init_offset;
static unsigned int ncmds;

extern int rad;

void dm_read_codesign (int i)
{  
	char *stmt;
	int   rc;

	ld = dm_allocate(sizeof(struct linkedit_data_command));
	memcpy(ld, (char *)offset, 
			sizeof(struct linkedit_data_command));

	if (rad) {
		printf("f section_lc_codesign @ 0x%08x\n", offset);
		printf("f section_lc_codesign_end @ 0x%08x\n", offset
				+ sizeof(struct linkedit_data_command));
		printf("CC [%02i] 0x%08x size=%d\n\n",
				i, offset, n0(ld->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);

		printf("\ncmd \t\t\t: LC_CODE_SIGNATURE\n");
		printf("cmd size \t\t: %d\n", n0(ld->cmdsize));
		printf("dataoff \t\t: %u\n", n0(ld->dataoff));
		printf("datasize \t\t: %u\n", n0(ld->datasize));
	}
}

void dm_read_uuid (int i)
{  
	char *stmt;
	int   rc;

	uuid = dm_allocate(sizeof(struct uuid_command));
	memcpy(uuid, (char *)offset, 
			sizeof(struct uuid_command));

	if (rad) {
		printf("f section.lc_uuid @ 0x%08x\n", offset);
		printf("f section.lc_uuid_end @ 0x%08x\n", offset
				+ sizeof(struct uuid_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(uuid->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);
		printf("\ncmd \t\t\t: LC_UUID\n");
		printf("cmd size \t\t: %d\n", n0(uuid->cmdsize));
		printf("uuid 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			(unsigned int)uuid->uuid[0], (unsigned int)uuid->uuid[1],
			(unsigned int)uuid->uuid[2],  (unsigned int)uuid->uuid[3],
			(unsigned int)uuid->uuid[4],  (unsigned int)uuid->uuid[5],
			(unsigned int)uuid->uuid[6],  (unsigned int)uuid->uuid[7]);
		printf("     0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			(unsigned int)uuid->uuid[8],  (unsigned int)uuid->uuid[9],
			(unsigned int)uuid->uuid[10], (unsigned int)uuid->uuid[11],
			(unsigned int)uuid->uuid[12], (unsigned int)uuid->uuid[13],
			(unsigned int)uuid->uuid[14], (unsigned int)uuid->uuid[15]);
	}
}

void dm_read_routines (int i)
{  
	char *stmt;
	int   rc;

	routines_command = dm_allocate(sizeof(struct routines_command));
	memcpy(routines_command, (char *)offset, sizeof(struct routines_command));

	if (rad) {
		printf("f section.lc_routines @ 0x%08x\n", offset);
		printf("f section.lc_routines_end @ 0x%08x\n", offset
				+ sizeof(struct routines_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(routines_command->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);

		printf("\ncmd \t\t\t: LC_ROUTINES\n");
		printf("cmd size \t\t: %d\n", n0(routines_command->cmdsize));
		printf("init_address \t\t\t %d:\n", n0(routines_command->init_address));
		printf("init_module \t\t\t: %d\n", n0(routines_command->init_module));
		printf("reserved1 \t\t\t: %d\n", n0(routines_command->reserved1));
		printf("reserved2 \t\t\t: %d\n", n0(routines_command->reserved2));
		printf("reserved3 \t\t\t: %d\n", n0(routines_command->reserved3));
		printf("reserved4 \t\t\t: %d\n", n0(routines_command->reserved4));
		printf("reserved5 \t\t\t: %d\n", n0(routines_command->reserved5));
		printf("reserved6 \t\t\t: %d\n", n0(routines_command->reserved6));
	}
}

void dm_read_twolevel_hints (int i)
{
	char *stmt;
	int   rc;

	tlh_command = dm_allocate(sizeof(struct twolevel_hints_command));
	memcpy(tlh_command, (char *)offset,
			sizeof(struct twolevel_hints_command));

	if (rad) {
		printf("f section.lc_twolevel_hints @ 0x%08x\n", offset);
		printf("f section.lc_twolevel_hints_end @ 0x%08x\n", offset
				+ sizeof(struct twolevel_hints_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(tlh_command->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);
		printf("\ncmd \t\t\t: LC_TWOLEVEL_HINTS\n");
		printf("cmd size \t\t: %d\n", n0(tlh_command->cmdsize));
		printf("offset \t\t\t: %u\n", n0(tlh_command->offset));
		printf("nhints \t\t\t: %u\n", n0(tlh_command->nhints));
	}
}

void dm_read_thread (int i)
{
	char *p;
	char *stmt;
	int   rc;
	unsigned long flavor;
	unsigned long count;

	thr_command = dm_allocate(sizeof(struct thread_command));
	memcpy(thr_command, (char *)offset, sizeof(struct thread_command));

	if (rad) {
		printf("f section.lc_thread @ 0x%08x\n", offset);
		printf("f section.lc_thread_end @ 0x%08x\n", offset
				+ sizeof(struct thread_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(thr_command->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);

		if (thr_command->cmd == LC_UNIXTHREAD)
			printf("\ncmd \t\t\t: LC_UNIXTHREAD\n");
		else printf("\ncmd \t\t\t: LC_THREAD\n");

		printf("cmd size \t\t: %d\n", n0(thr_command->cmdsize));
		printf("cputype: %d\n", n0(m_header.cputype));
	}

	switch(n0(m_header.cputype)) {
	case 18: // POWERPC
		{
		unsigned int state[18];
		/*
		 * Pointing to the begin of flavor field
		 */
		p = (char *)offset + sizeof(struct thread_command);

		//memcpy(&flavor, p, sizeof(unsigned long));
		flavor = n0(p);
		printf("; flavor \t\t\t: %lu\n", flavor);

		/*
		 * Pointing to the count field
		 */
		p += sizeof(unsigned long);

		//memcpy(&count, p, sizeof(unsigned long));
		count = n0(p);
		printf("; count \t\t\t: %lu\n", count);

		/*
		 * Moving on to the beginning of the 
		 * i386_thread_state structure
		 */
		p += sizeof(unsigned long);

		memset(&state, 0x0, sizeof(state));
		memcpy(&state, (char *)p, sizeof(state));
		printf("fs regs\n");
		for(i=0;i<18;i++) {
			printf("f r%d @ 0x%08x\n", i, n0(state[i]));
		}
		printf("f entrypoint @ 0x%08x\n", n0(state[15]));
		printf("e asm.arch=ppc\n");
		printf("e cfg.bigendian=true\n");
		printf("e io.paddr=0x1000\n");
		}
		break;
	case 12: // ARM
		{
		unsigned int state[32];
		/*
		 * Pointing to the begin of flavor field
		 */
		p = (char *)offset + sizeof(struct thread_command);

		memcpy(&flavor, p, sizeof(unsigned long));
		printf("; flavor \t\t\t: %lu\n", flavor);

		/*
		 * Pointing to the count field
		 */
		p += sizeof(unsigned long);

		memcpy(&count, p, sizeof(unsigned long));
		printf("; count \t\t\t: %lu\n", count);

		/*
		 * Moving on to the beginning of the 
		 * i386_thread_state structure
		 */
		p += sizeof(unsigned long);

		memset(&state, 0x0, sizeof(state));
		memcpy(&state, (char *)p, sizeof(state));
		printf("fs regs\n");
		for(i=0;i<32;i++) {
			printf("f r%d @ 0x%08x\n", i, n0(state[i]));
		}
		printf("f entrypoint @ 0x%08x\n", n0(state[15]));
		printf("e io.paddr = 0x1000\n");
		printf("e asm.arch=arm\n");
		}
		break;
	case CPU_TYPE_I386:
		{
		i386_thread_state_t         state;

		/*
		 * Pointing to the begin of flavor field
		 */
		p = (char *)offset + sizeof(struct thread_command);

		memcpy(&flavor, p, sizeof(unsigned long));
		printf("; flavor \t\t\t: %lu\n", flavor);

		/*
		 * Pointing to the count field
		 */
		p += sizeof(unsigned long);

		memcpy(&count, p, sizeof(unsigned long));
		printf("; count \t\t\t: %lu\n", count);

		/*
		 * Moving on to the beginning of the 
		 * i386_thread_state structure
		 */
		p += sizeof(unsigned long);

		memset(&state, 0x0, sizeof(i386_thread_state_t));
		memcpy(&state, (char *)p, sizeof(i386_thread_state_t));

		printf("fs regs\n");
		printf("f entrypoint @ 0x%08x\n", n0(state.eip));
		printf("f eax @ 0x%08x\n", n0(state.eax));
		printf("f ebx @ 0x%08x\n", n0(state.ebx));
		printf("f ecx @ 0x%08x\n", n0(state.ecx));
		printf("f edx @ 0x%08x\n", n0(state.edx));
		printf("f esp @ 0x%08x\n", n0(state.esp));
		printf("f ebp @ 0x%08x\n", n0(state.ebp));
		printf("f eip @ 0x%08x\n", n0(state.eip));
		printf("f esi @ 0x%08x\n", n0(state.esi));
		printf("f edi @ 0x%08x\n", n0(state.edi));
		printf("f eflags @ 0x%04x\n", n0(state.eflags));
		printf(
		"; State \t\t\t: eax 0x%08x ebx    0x%08x ecx 0x%08x edx 0x%08x\n"
		"; State \t\t\t: edi 0x%08x esi    0x%08x ebp 0x%08x esp 0x%08x\n"
		"; State \t\t\t: ss  0x%08x eflags 0x%08x eip 0x%08x cs  0x%08x\n"
		"; State \t\t\t: ds  0x%08x es     0x%08x fs  0x%08x gs  0x%08x\n",
		n0(state.eax), n0(state.ebx), n0(state.ecx), n0(state.edx), n0(state.edi), n0(state.esi),
		n0(state.ebp), n0(state.esp), n0(state.ss), n0(state.eflags), n0(state.eip), n0(state.cs),
		n0(state.ds), n0(state.es), n0(state.fs), n0(state.gs));
		printf("e asm.arch=x86\n");
		}
		break;
	default:
		dm_fatal(ECPU);
	}
}
    
void dm_read_prebound_dylib_command (int i)
{
  char *pname, *plinked_modules;
  unsigned long j;

  prebound_dy_command = dm_allocate(sizeof(struct prebound_dylib_command));
  memcpy(prebound_dy_command, (char *)offset,
         sizeof(struct prebound_dylib_command));

  if (rad)
    {
      printf("f section.lc_prebound_dylib @ 0x%08x\n", offset);
      printf("f section.lc_prebound_dylib_end @ 0x%08x\n", offset
             + sizeof(struct prebound_dylib_command));
		  printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
             i, offset, prebound_dy_command->cmdsize);
    }
  else
    {
      printf("\n [-] Load command %d\n", i);

      printf("\ncmd \t\t\t: LC_PREBOUND_DYLIB\n");
      printf("cmd size \t\t: %d\n", n0(prebound_dy_command->cmdsize));

      pname = (char *)offset + n0(prebound_dy_command->name.offset);
      printf("name %s (offset %u)\n", pname, prebound_dy_command->name.offset);

      printf("nmodules \t\t\t: %d\n",  n0(prebound_dy_command->nmodules));

      plinked_modules = (char *)offset + 
        n0(prebound_dy_command->linked_modules.offset);

      for (j = 0; j < n0(prebound_dy_command->nmodules) && j < 8; j++)
        {
          if ( ((plinked_modules[j/8] >> (j%8)) & 1) )
            printf("%lu\n", j); 
        }
    }
}

void dm_read_sub_client (int i)
{
	char *p, *stmt;
	int   rc;

	sub_client = dm_allocate(sizeof(struct sub_client_command));
	memcpy(sub_client, (char *)offset, sizeof(struct sub_client_command));

	if (rad) {
		printf("f section.lc_sub_client @ 0x%08x\n", offset);
		printf("f section.lc_sub_client_end @ 0x%08x\n", offset
				+ sizeof(struct sub_client_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(sub_client->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);

		printf("\ncmd \t\t\t: LC_SUB_CLIENT\n");
		printf("cmd size \t\t: %d\n", sub_client->cmdsize);

		p = (char *)offset + sub_client->client.offset;
		printf("sub_client %s (offset %u)\n", p, sub_client->client.offset);
	}
}

void dm_read_sub_library (int i)
{
	char* p;
	char *stmt;
	int   rc;

	sub_library = dm_allocate(sizeof(struct sub_library_command));
	memcpy(sub_library, (char *)offset, sizeof(struct sub_library_command));

	if (rad) {
		printf("f section.lc_sub_library @ 0x%08x\n", offset);
		printf("f section.lc_sub_library_end @ 0x%08x\n", offset
				+ sizeof(struct sub_library_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(sub_library->cmdsize));
	} else {
		printf("\n [-] Load command %d\n", i);

		printf("\ncmd \t\t\t: LC_SUB_LIBRARY\n");
		printf("cmd size \t\t: %d\n", n0(sub_library->cmdsize));

		p = (char *)offset + n0(sub_library->sub_library.offset);
		printf("sub_library %s (offset %u)\n", p, n0(sub_library->sub_library.offset));
	}
}

void dm_read_sub_umbrella (int i)
{
	char *p;
	char *stmt;
	int   rc;

	sub_umbrella = dm_allocate(sizeof(struct sub_umbrella_command));
	memcpy(sub_umbrella, (char *)offset, sizeof(struct sub_umbrella_command));

	if (rad) {
		printf("f section.lc_sub_umbrella @ 0x%08x\n", offset);
		printf("f section.lc_sub_umbrella_end @ 0x%08x\n", offset
				+ sizeof(struct sub_umbrella_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(sub_umbrella->cmdsize));
	} else {
		printf ("\n [-] Load command %d\n", i);

		printf("\ncmd \t\t\t: LC_SUB_UMBRELLA\n");
		printf("cmd size \t\t: %d\n", n0(sub_umbrella->cmdsize));

		p = (char *)offset + n0(sub_umbrella->sub_umbrella.offset);
		printf("sub_umbrella %s (offset %u)\n", p, sub_umbrella->sub_umbrella.offset);
	}
}

void dm_read_sub_framework (int i)
{
	char *p, *stmt;
	int rc;

	sub_framework = dm_allocate(sizeof(struct sub_framework_command));
	memcpy(sub_framework, (char *)offset, sizeof(struct sub_framework_command));

	if (rad) {
		printf("f section.lc_sub_framework @ 0x%08x\n", offset);
		printf("f section.lc_sub_framework_end @ 0x%08x\n", offset
				+ sizeof(struct sub_framework_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(sub_framework->cmdsize));
	} else {
		printf ("\n [-] Load command %d\n", i);
		printf("\ncmd \t\t\t: LC_SUB_FRAMEWORK\n");
		printf("cmd size \t\t: %d\n", n0(sub_framework->cmdsize));
		p = (char *)offset + n0(sub_framework->umbrella.offset);
		printf("umbrella %s (offset %u)\n", p, n0(sub_framework->umbrella.offset));
	}
}

void dm_read_dylib (int i)
{
	char *p, *stmt;
	int rc;

	dyl_command = dm_allocate(sizeof(struct dylib_command));
	memcpy(dyl_command, (char *)offset, sizeof(struct dylib_command));

	if (rad) {
		printf("f section.lc_dylib @ 0x%08x\n", offset);
		printf("f section.lc_dylib_end @ 0x%08x\n", offset
				+ sizeof(struct dylib_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(dyl_command->cmdsize));
	} else {
		printf ("\n [-] Load command %d\n", i);

		if ( dyl_command->cmd == LC_ID_DYLIB )
			printf("\ncmd \t\t\t: LC_ID_DYLIB\n");
		else if ( dyl_command->cmd == LC_LOAD_DYLIB )
			printf("\ncmd \t\t\t: LC_LOAD_DYLIB\n");
		else printf("\ncmd \t\t\t: LC_LOAD_WEAK_DYLIB\n");

		printf("cmd size \t\t: %d\n", n0(dyl_command->cmdsize));

		p = (char *)offset + n0(dyl_command->dylib.name.offset);
		printf("name \t\t\t: %s (offset %u)\n", p,  n0(dyl_command->dylib.name.offset));

		printf("time stamp \t\t: %u ", n0(dyl_command->dylib.timestamp));
		/* XXX . not endian safe */
		printf("%s", ctime((const long *)&(dyl_command->dylib.timestamp)));
		printf("current version \t: %u.%u.%u\n",
			dyl_command->dylib.current_version >> 16,
			(dyl_command->dylib.current_version >> 8) & 0xff,
			dyl_command->dylib.current_version & 0xff);
		printf("compatibility version \t: %u.%u.%u\n",
			dyl_command->dylib.compatibility_version >> 16,
			(dyl_command->dylib.compatibility_version >> 8) & 0xff,
			dyl_command->dylib.compatibility_version & 0xff);
	}
}

void dm_read_fvmlib (int i)
{
	char *p, *stmt;
	int rc;

	fvm_command = dm_allocate(sizeof(struct fvmlib_command));
	memcpy(fvm_command, (char *)offset, sizeof(struct fvmlib_command));

	if (rad) {
		printf("f section.lc_fvmlib @ 0x%08x\n", offset);
		printf("f section.lc_fvmlib_end @ 0x%08x\n", offset
				+ sizeof(struct fvmlib_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, n0(fvm_command->cmdsize));
	} else {
		printf ("\n [-] Load command %d\n", i);

		if ( fvm_command->cmd == LC_IDFVMLIB )
			printf ("\ncmd \t\t: LC_IDFVMLIB\n");
		else
			printf ("\ncmd \t\t: LC_LOADFVMLIB\n");

		printf ("cmd size \t: %d\n", n0(fvm_command->cmdsize));

		p = (char *)offset + fvm_command->fvmlib.name.offset;
		printf("Path %s (offset %u)\n", p,  fvm_command->fvmlib.name.offset);
		printf("minor version %u\n", fvm_command->fvmlib.minor_version);
		printf("header addr 0x%08x\n", fvm_command->fvmlib.header_addr);
	}
}

void dm_read_symseg (int i)
{
	int rc;
	char *stmt;

	symseg_command = dm_allocate(sizeof(struct symseg_command));
	memcpy(symseg_command, (char *)offset, sizeof(struct symseg_command));

	if (rad) {
		printf("f section.lc_symseg @ 0x%08x\n", offset);
		printf("f section.lc_symseg_end @ 0x%08x\n", offset
				+ sizeof(struct symseg_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, symseg_command->cmdsize);
	} else {
		printf ("\n [-] Load command %d\n", i);
		printf ("\ncmd \t\t: LC_SYMSEG\n");
		printf ("cmd size \t: %d\n",    symseg_command->cmdsize);
		printf ("symoff \t\t: %d\n",    symseg_command->offset);
		printf ("size \t\t: %d\n",      symseg_command->size);
	}
}

void dm_read_dysymtab (int i)
{
	int rc;
	char *stmt;

	dysym_command = dm_allocate(sizeof(struct dysymtab_command));
	memcpy(dysym_command, (char *)offset, sizeof(struct dysymtab_command));

	if (rad) {
		printf("f section.lc_dysymtab @ 0x%08x\n", offset);
		printf("f section.lc_dysymtab_end @ 0x%08x\n", offset
				+ sizeof(struct dysymtab_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, dysym_command->cmdsize);
	} else {
		printf ("\n [-] Load command %d\n", i);
		printf ("\nSection Name \t: LC_DYSYMTAB\n");
		printf ("cmd size \t: %d\n",n0(dysym_command->cmdsize));
		printf ("ilocalsym \t: %d\n",       n0(dysym_command->ilocalsym));
		printf ("nlocalsym \t: %d\n",       n0(dysym_command->nlocalsym));
		printf ("iextdefsym \t: %d\n",      n0(dysym_command->iextdefsym));
		printf ("nextdefsym \t: %d\n",      n0(dysym_command->nextdefsym));
		printf ("iundefsym \t: %d\n",       n0(dysym_command->iundefsym));
		printf ("nundefsym \t: %d\n",       n0(dysym_command->nundefsym));
		printf ("tocoff \t\t: %d\n",        n0(dysym_command->tocoff));
		printf ("ntoc \t\t: %d\n",          n0(dysym_command->ntoc));
		printf ("modtaboff \t: %d\n",       n0(dysym_command->modtaboff));
		printf ("nmodtab \t: %d\n",         n0(dysym_command->nmodtab));
		printf ("extrefsymoff \t: %d\n",    n0(dysym_command->extrefsymoff));
		printf ("indirectsymoff \t: %d\n",  n0(dysym_command->indirectsymoff));
		printf ("nindirectsyms \t: %d\n",   n0(dysym_command->nindirectsyms));
		printf ("extreloff \t: %d\n",       n0(dysym_command->extreloff));
		printf ("nextrel \t: %d\n",         n0(dysym_command->nextrel));
		printf ("locreloff \t: %d\n",       n0(dysym_command->locreloff));
		printf ("nlocrel \t: %d\n",         n0(dysym_command->nlocrel));
	}
}

void dm_read_symtab (int i)
{
	int rc;
	char *stmt;

	sy_command = dm_allocate(sizeof(struct symtab_command));
	memcpy(sy_command, (char *)offset, sizeof(struct symtab_command));

	if (rad) {
		printf("f section.lc_dysymtab @ 0x%08x\n", offset);
		printf("f section.lc_dysymtab_end @ 0x%08x\n", offset
				+ sizeof(struct dysymtab_command));
		printf("CC [%02i] 0x%08x cmdsize=%d\n\n",
				i, offset, sy_command->cmdsize);
	} else {
		printf("\n [-] Load command %d\n", i);
		printf("\ncmd \t\t: LC_SYMTAB\n");
		printf("cmd size \t: %d\n", n0(sy_command->cmdsize));
		printf("symoff \t\t: %d\n", n0(sy_command->symoff));
		printf("nsyms \t\t: %d\n",  n0(sy_command->nsyms));
		printf("stroff \t\t: %d\n", n0(sy_command->stroff));
		printf("strsize \t: %d\n", n0(sy_command->strsize));
	}
}

void dm_read_section (int i)
{
	char *stmt;
	int rc;

	sect = dm_allocate(sizeof(struct section));
	memcpy(sect, (char *)toff, sizeof(struct section));

	if (rad) {
		printf("f section.%s @ 0x%08x\n", sect->sectname, offset);
		printf("f section.%s_end @ 0x%08x\n", sect->sectname, offset
				+ sizeof(sect));
		printf("CC [%02i] 0x%08x segname=%s addr=0x%08x\n\n",
				i, sect, sect->segname, sect->addr);
	} else {
		printf("\n [-] Section \n");
		printf("\nSection Name \t: %s\n", sect->sectname);
		printf("Segment Name \t: %s\n", 	sect->segname);
		printf("Address \t: 0x%08x\n", 	  sect->addr);
		printf("Size \t\t: 0x%08x\n",     sect->size);
		printf("Offset \t\t: %d\n",       sect->offset);
		printf("Align \t\t: %d\n",        sect->align);
		printf("Reloff \t\t: %d\n",       sect->reloff);
		printf("Nreloc \t\t: %d\n",       sect->nreloc);
		printf("Flags \t\t: 0x%08x\n", 	  sect->flags);
		printf("Reserved1 \t: %d\n",      sect->reserved1);
		printf("Reserved2 \t: %d\n",      sect->reserved2);
	}
}

void dm_read_dylinker (int i)
{
	char *p;
	char *stmt;
	int  rc;

	dy_command = dm_allocate(sizeof(struct dylinker_command));
	memcpy(dy_command, offset, sizeof(struct dylinker_command));

	if (rad) {
		printf("f section.lc_dylinker @ 0x%08x\n", offset);
		printf("f section.lc_dylinker_end @ 0x%08x\n", offset
				+ sizeof(struct dylinker_command));
		printf("CC [%02i] 0x%08x cmd=%d\n\n",
				i, dy_command, dy_command->cmd);
	} else {
		printf("\n [-] Load command %d\n", i);

		if (dy_command->cmd == LC_ID_DYLINKER)
			printf("\ncmd \t\t: LC_ID_DYLINKER\n");
		else
			printf("\ncmd \t\t: LC_LOAD_DYLINKER\n");

		printf("cmd size \t: %d\n",n0(dy_command->cmdsize));

		p = (char *)offset + n0(dy_command->name.offset);
		printf("Path %s (offset %u)\n", p, dy_command->name.offset);
	}
}

void dm_read_header (int p)
{
	void          *archp;
	int           i386_found=0, i, rc;
	unsigned int  nfat;
	char	        *stmt;

	if (fileaddr == NULL) {
		fprintf(stderr, "file not mmaped in memory.\n");
		return;
	}
	memset(&fat_header, 0x0,        sizeof(struct fat_header));
	memcpy(&fat_header, fileaddr,   sizeof(struct fat_header));

	// TODO: use switch case?
	if (fat_header.magic == FAT_CIGAM) {
#if __APPLE__
		// swap_fat_header(&fat_header, LITTLE_ENDIAN);
#elif __linux__
		fat_header.magic = SWAP_LONG(fat_header.magic);
		fat_header.nfat_arch = SWAP_LONG(fat_header.nfat_arch);
#endif
		swapped = 1;

		if (rad) {
			printf("fs header\n");
			printf("f header.fat @ 0x%08llx\n", fileaddr);
			printf("f header.fat_end @ 0x%08x\n",
					fileaddr + sizeof(struct fat_header));
			printf("CC [] %08x narchs=%d\n\n",
					fileaddr, fat_header.nfat_arch);
		} else {
			printf ("\n\n [-] fat Header\n\n");
			printf ("Magic Number \t: 0x%08x\n", fat_header.magic);
			printf ("fat archs \t: %d\n", fat_header.nfat_arch);
		}
	}

	nfat = fat_header.nfat_arch;
	archp = fileaddr + sizeof(struct fat_header);

	if ( fat_header.magic == FAT_MAGIC ) {
		archs = dm_allocate(sizeof(struct fat_arch) * nfat);

		memcpy(archs, archp, sizeof(struct fat_arch) * nfat);
#if __APPLE__
		// XXX swap_fat_arch(archs, nfat, LITTLE_ENDIAN);
#endif

		if (rad)
			printf("fs archs\n");

		for(i = 0; i < nfat; i++) {
#if __linux__
			archs[i].cputype    = n0(archs[i].cputype);
			archs[i].cpusubtype = n0(archs[i].cpusubtype);
			archs[i].offset     = n0(archs[i].offset);
			archs[i].size       = n0(archs[i].size);
			archs[i].align      = n0(archs[i].align);
#endif
			if (!rad)
				printf("\n [-] Architecture %d\n\n", i);

			if (archs[i].cputype == CPU_TYPE_X86) {
				fileaddr += archs[i].offset;
				i386_found++;
				if (rad) {
					printf("f section.arch_x86 @ 0x%08x\n",
							archp + sizeof(struct fat_arch) * i);
					printf("f section.arch_x86_end @ 0x%08x\n", archp
							+ sizeof(struct fat_arch) * i
							+ sizeof(struct fat_arch));
					printf("CC [] 0x%08x offset=0x%08x\n\n",
							archp + sizeof(struct fat_arch) * i,
							archs[i].offset);
				} else {
					printf("cputype \t: CPU_TYPE_X86\n");
					printf("offset \t\t: %u\n", (unsigned int)archs[i].offset);
				}
			} else {
				if (rad) {
					/*
					 * FIX: Add support for all the other arch name
					 */
					printf("f section.arch_%d @ 0x%08x\n",
							archs[i].cputype,
							archp + sizeof(struct fat_arch) * i);
					printf("f section.arch_%d_end @ 0x%08x\n",
							archs[i].cputype,
							archp + sizeof(struct fat_arch) * i
							+ sizeof(struct fat_arch));
					printf("CC [] 0x%08x offset=0x%08x\n\n",
							archp + sizeof(struct fat_arch) * i,
							archs[i].offset);

				} else {
					printf("cputype \t: %d\n", archs[i].cputype);
					printf("offset \t\t: %u\n", (unsigned int)archs[i].offset);
				}
			}
		}
	}

	if ( !swapped || i386_found ) {
		memset(&m_header, '\0',     sizeof(struct mach_header));
		memcpy(&m_header, fileaddr, sizeof(struct mach_header));
		if ( p ) {
			printf ("\n\n [-] mach-o Header\n");
			printf ("\nMagic Number \t: 0x%08x\n", 	n0(m_header.magic));
			printf ("CPU Type \t: %d\n", n0(m_header.cputype));
			printf ("CPU Sub Type \t: %d\n", n0(m_header.cpusubtype));
			printf ("File Type \t: %d\n", n0(m_header.filetype));
			printf ("Num Of Cmds \t: %d\n", n0(m_header.ncmds));
			printf ("Size Of Cmds \t: %d\n",n0(m_header.sizeofcmds));
			printf ("Flags \t\t: 0x%08x\n\n",n0(m_header.flags));
		}
	}
}

void dm_read_segment (int i)
{
	char *stmt;
	int rc;

	if (rad) {
		printf("f section.lc_segment @ 0x%08x\n", offset);
		printf("f section.lc_segment_end @ 0x%08x\n",
			offset+ sizeof(seg_command));
		printf("CC [%02i] 0x%08x size=%08i nsects=%03i\n\n",
			i, seg_command, n0(seg_command->cmdsize),
			n0(seg_command->nsects));
	} else {
		printf ("\n [-] Load command %d\n", i);
		printf ("\ncmd \t\t: LC_SEGMENT\n");
		printf ("cmd size \t: %d\n",        n0(seg_command->cmdsize));
		printf ("seg name \t: %s\n",        seg_command->segname);
		printf ("VM Addr \t: 0x%08x\n",     n0(seg_command->vmaddr));
		printf ("VM Size \t: 0x%08x\n",     n0(seg_command->vmsize));
		printf ("Fileoff \t: %d\n",         n0(seg_command->fileoff));
		printf ("Filesize \t: %d\n",        n0(seg_command->filesize));
		printf ("Max Prot \t: 0x%08x\n",    n0(seg_command->maxprot));
		printf ("Init Prot \t: 0x%08x\n",   n0(seg_command->initprot));
		printf ("NSects \t\t: %d\n",        n0(seg_command->nsects));
		printf ("Flags \t\t: 0x%08x\n",     n0(seg_command->flags));    
	}
}

void dm_read_command (int ctr)
{
unsigned char *p;
	int i, z, cmd, nsects;
	/*if ( ctr )*/
	/*dm_read_header(1);*/
	/*else*/
	dm_read_header(0);
	ncmds = n0(m_header.ncmds);
	// Offset to the first load command
	init_offset = (void *)fileaddr+sizeof(struct mach_header);
	// Offset to walk through the load_command structures
	offset = init_offset;
	// Offset to walk through the section structures
	toff = offset;

	if (rad)
		printf("fs sections\n");

p = offset;

	for ( i=0; i < ncmds; i++ ) {
		/*
		 * l_command = dm_allocate(sizeof(struct load_command));
		 * memcpy((struct load_command *)l_command, (struct load_command *)offset, 
		 * sizeof(struct load_command));
		 */
		seg_command = dm_allocate(sizeof(struct segment_command));
		if (seg_command == NULL)
			break;
		memcpy((struct segment_command *)seg_command,
				(struct segment_command *)p,
				sizeof(struct segment_command));

		cmd = n0(seg_command->cmd);
		
//printf("%d %d\n", cmd, seg_command->cmd);
		switch (cmd) {
		case LC_SEGMENT:
			nsects = n0(seg_command->nsects);
			dm_read_segment(i);
			toff = offset+sizeof(struct segment_command);
			for ( z=0; z < nsects; z++ ) {
				dm_read_section(z);
				toff += sizeof(struct section);
			}
			break;
		case LC_SYMTAB:
			dm_read_symtab(i);
			break;
		case LC_DYSYMTAB:
			dm_read_dysymtab(i);
			break;
		case LC_IDFVMLIB:
		case LC_LOADFVMLIB:
			dm_read_fvmlib(i);
			break;
		case LC_ID_DYLIB:
		case LC_LOAD_DYLIB:
		case LC_LOAD_WEAK_DYLIB:
			dm_read_dylib(i);
			break;
		case LC_SUB_FRAMEWORK:
			dm_read_sub_framework(i);
			break;
		case LC_SUB_UMBRELLA:
			dm_read_sub_umbrella(i);
			break;
		case LC_SUB_LIBRARY:
			dm_read_sub_library(i);
			break;
		case LC_SUB_CLIENT:
			dm_read_sub_client(i);
			break;
		case LC_PREBOUND_DYLIB:
			dm_read_prebound_dylib_command(i);
			break;
		case LC_ID_DYLINKER:
		case LC_LOAD_DYLINKER:
			dm_read_dylinker(i);
			break;
		case LC_THREAD:
		case LC_UNIXTHREAD:
			dm_read_thread(i);
			break;
		case LC_TWOLEVEL_HINTS:
			dm_read_twolevel_hints(i);
			break;
		case LC_ROUTINES:
			dm_read_routines(i);
			break;
		case LC_UUID:
			dm_read_uuid(i);
			break;
		case LC_CODE_SIGNATURE:
			dm_read_codesign(i);
			break;
		default:
			break;
		}
		{
			//ut32 csz = n0(seg_command->cmdsize);
			ut32 csz = n0(seg_command->cmdsize);
			if (csz == 0) /* avoid infinite loops or so? XXX can be tricked */
				break;
			offset += csz; //seg_command->cmdsize;
			p = offset;
		}
	}
}
