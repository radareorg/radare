/* $VER: vda68k V1.1 (07.03.2001)
 *
 * Simple M68k file and memory disassembler.
 * Copyright (c) 2000-2002  Frank Wille
 *
 * vdappc is freeware and part of the portable and retargetable ANSI C
 * compiler vbcc, copyright (c) 1995-2002 by Volker Barthelmann.
 * vdappc may be freely redistributed as long as no modifications are
 * made and nothing is charged for it. Non-commercial usage is allowed
 * without any restrictions.
 * EVERY PRODUCT OR PROGRAM DERIVED DIRECTLY FROM MY SOURCE MAY NOT BE
 * SOLD COMMERCIALLY WITHOUT PERMISSION FROM THE AUTHOR.
 *
 *
 * v1.1  (07.03.2001) phx
 *       Support for little-endian architectures.
 * v1.0  (26.06.2000) phx
 *       File created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m68k_disasm.h"

#define VERSION 1
#define REVISION 2

const char *_ver = "$VER: vda68k 1.2 (18.04.2002)\r\n";


main(int argc,char *argv[])
{
  FILE *fh = NULL;
  m68k_word buf[12];
  m68k_word *p,*ip;
  unsigned long foff=0;
  long pos;
  struct DisasmPara_68k dp;
  char opcode[16];
  char operands[128];
  char iwordbuf[32];
  char tmpbuf[8];
  int n;
  char *s;

  if (argc!=2 || !strncmp(argv[1],"-h",2) || argv[1][0]=='?') {
    printf("vda68k V%d.%d  (c)2000-2002 by Frank Wille\n"
           "M68k disassembler V%d.%d  (c)1999-2002 by Frank Wille\n"
           "Based on NetBSD disassembler  (c)1994 by Christian E. Hopps\n"
           "Build date: " __DATE__ ", " __TIME__ "\n\n"
           "Usage: %s file|address\n",
           VERSION,REVISION,M68KDISASM_VER,M68KDISASM_REV,argv[0]);
    exit(1);
  }

  /* initialize DisasmPara */
  memset(&dp,0,sizeof(struct DisasmPara_68k));
  dp.opcode = opcode;
  dp.operands = operands;
  dp.radix = 16;  /* we want hex! */
  iwordbuf[26] = '\0';

  if (isdigit((unsigned int)argv[1][0])) {
    sscanf(argv[1],"%i",(int *)&p);
  }
  else {
    /* open file */
    if (!(fh = fopen(argv[1],"r"))) {
      fprintf(stderr,"%s: Can't open %s!\n",argv[0],argv[1]);
      exit(10);
    }
    dp.instr = buf;
  }

  for (;;) {
    /* disassembler loop */
    if (fh) {
      pos = ftell(fh);
      memset(buf,0,sizeof(m68k_word)*8);
      if (fread(buf,sizeof(m68k_word),8,fh) < 1)
        break;  /* EOF */
      dp.iaddr = (m68k_word *)foff;
      n = M68k_Disassemble(&dp) - dp.instr;
      fseek(fh,pos,SEEK_SET);
      if (fread(buf,sizeof(m68k_word),n,fh) != n)
        break;  /* read error */
    }
    else
      dp.instr = dp.iaddr = p;
    p = M68k_Disassemble(&dp);

    /* print up to 5 instruction words */
    for (n = 0; n<26; iwordbuf[n++]=' ');
    if ((n = (int)(p-dp.instr)) > 5)
      n = 5;
    ip = dp.instr;
    s = iwordbuf;
    while (n--) {
      sprintf(tmpbuf,"%02x%02x",*(unsigned char *)ip,
              *((unsigned char *)ip+1));
      ip++;
      strncpy(s,tmpbuf,4);
      s += 5;
    }

    printf("%08lx: %s%-7s %s\n",(unsigned long)dp.iaddr,iwordbuf,
           opcode,operands);
    if (fh)
      foff += (p - dp.instr) * sizeof(m68k_word);
  }

  /* cleanup */
  if (fh)
    fclose(fh);
  exit(0);
}
