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
//__APPLE__
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>
#endif

#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "dietmach0_errors.h"
#include "dietmach0.h"

#include "../main.h"
#if __WINDOWS__
#else
#include <sys/mman.h>
#endif

/*
 * Gloabal Variables
 */
static int fd, fdout;
static struct stat sb;

void
*dm_allocate (size_t nbytes)
{
  void *pointer;

  if ( !(pointer = malloc(nbytes)) ) {
    //dm_fatal(EMAPFD);
    return NULL;
  }

  memset(pointer, '\0', nbytes);

  return pointer;
}

void
dm_fatal (const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  (void) vfprintf(stderr, fmt, ap);
  va_end(ap);

  /*
   * Closing all open files
   */
  exit(1);
}

int filesize;
char *fileaddr;
char *startaddr;
void
dm_map_file (char *filename, int fd)
{
#if __APPLE__
  kern_return_t result;
#endif

  if ( stat(filename, &sb) == -1 ) {
    close(fd);
    dm_fatal("cannot stat()\n");
    exit(-1);
  }

  /*
   *  Print out information about mach-o file
   */
  //fprintf(stderr, "\nFilename: %s\n", filename);
  filesize = sb.st_size;
if (filesize == 0) {
	filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
}
/* hack and wiki! -- ugly hack but working */
if (filesize == 0)
	filesize = 8192;
  //fprintf(stderr, "File '%s' size: %d\n\n", filename, filesize);

  fileaddr = NULL;
#if __APPLE__
  fileaddr = malloc(filesize);
  lseek(fd, 0, SEEK_SET);
  read(fd, fileaddr, filesize);
#elif __WINDOWS__
  fprintf(stderr, "NO mmap for this platform?!?!\n");
  return -1;
#if 0
#elif __BSD__
  if ( (map_fd((int)fd, (vm_offset_t)0, (vm_offset_t *)&fileaddr,
		TRUE, (vm_size_t)filesize)) != KERN_SUCCESS ) {
	eprintf("Cannot map filedescriptor\n");
		return -1;
	}
#endif
#elif __linux__ || __APPLE__ || __BSD__
  fileaddr = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
//perror("mmap");
//printf("FILEADDR=%p\n", fileaddr);
  if ( fileaddr == MAP_FAILED )
    {
      dm_fatal("Cannot map file %s\n", filename);
      close(fd);
      exit(-1);
    }
#else
#warning MACHO parser not working on this arch

  //fileaddr = (char *)malloc(filesize);
#endif

  //if ( (result = mmap(&fileaddr, (size_t)filesize, PROT_READ|PROT_WRITE,
   //                   MAP_SHARED, (int)fd, 0)) == (int *)MAP_FAILED)

  //printf(" fileaddr = 0x%08x\n", fileaddr);
  //fread(fileaddr, sizeof(char), filesize, fd);
  //fclose(fd);

  startaddr = fileaddr;
}

char
*dm_split (char *strmine, const char delim, int size)
{
  int i;
  char *rt = NULL;

  if (strmine[0] == '\0')
    return NULL;

  for (i=0; i < size; i++)
    {
      if (strmine[i] == '\0')
        {
          rt = strmine;
          break;
        }
      else if (strmine[i] == delim)
        {
          strmine[i] = '\0';
          rt = strmine;
          break;
        } 
    }

  return rt;
}

void
Debug (const char *fmt, ...)
{
  va_list ap;

  fprintf(stderr, "DEBUG:\t");
  va_start(ap, fmt);
  (void) vfprintf(stderr, fmt, ap);
  va_end(ap);
}

ut32 n0(const unsigned char *addr)
{
	ut32 csz;
#if __WINDOWS__
	/* HTONL IS NOT PORTABLE !!! */
	return (ut32)addr;
#else
	if (!memcmp("\xfe\xed\xfa\xce", fileaddr, 4))
		csz = htonl(addr);
	else csz = (ut32)addr;
#endif
	return csz;
}
