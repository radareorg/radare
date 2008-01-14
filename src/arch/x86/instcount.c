/* Shit Oriented Programming Model Example - SHOP-ME (TM) */

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

/* TODO
 * ----
 *    16 bit
 *    64 bit (REX prefix)
 */

int instLength(unsigned char *p, int s, int mode);

#if 0
int main(int argc, char **argv)
{
  unsigned char buffer[256];
  int nbytes, i, j, l, o, n, c;

  if(argc > 1)
    sscanf(argv[1], "%d", &l);
  
  nbytes = read(0, buffer, sizeof(buffer));
  o = 0;
  for(i = 0; i < l; i++)
  {
    n = instLength(buffer + o, nbytes - o, 0);
    printf("inst[%d] = %d (offset %d) ", i, n, o);
    for(c = '[', j = 0; j < n; c = ' ', j++)
      printf("%c%02x", c, buffer[o + j]);
    printf("]\n");
    o += n;
  }

  exit(0);
}
#endif

int instLength(unsigned char *p, int s, int mode)
{
  unsigned char *b;
  int nbytes  = 0;
  int mprefix = 0;
  int modrm   = 0;
  int imm     = 0; /* immediate data: 0 none, 1 immediate data */
  int opsize  = 0; /* immediate size: 0 byte, 1 word           */
  int wdsize  = 4; /* word size: 2 bytes, 4 bytes ...          */
  int extra   = 0; /* extra bytes (unclassificable)            */
  int adsize  = 4; /* addressing: 0 = 16 bits, 1 = 32 bits     */

  nbytes = 0;
  b = p;

  /* PREFIX DECODING */
  {
    int nprefix;

    for(mprefix = nprefix = 0; nprefix < 3; b++, nprefix++)
    {
      /* if these prefixes are followed by an 0x0f, then is a mandatory */
      /* prefix for a special opcode (of more than 1 byte)              */
      if((*b == 0xf2 || *b == 0xf3 || *b == 0x66) && (*(b + 1) == 0x0f))
      {
        mprefix = *b++;
        goto decode_op;
      }

      /* normal prefix processing ... */
      switch(*b)
      {
        case 0x66: /* G3: Operand-size override prefix                */
          wdsize = 2;
          break;
        case 0x67: /* G4: address-size override prefix                */
          adsize = 2;
          break;
        case 0xf2: /* G1: repne/repnz                                 */
        case 0xf3: /* G1: rep, repe/repz                              */
        case 0xf0: /* G1: lock                                        */
        case 0x2e: /* G2: segment override CS | Brach hint: not taken */
        case 0x36: /* G2: segment override SS                         */
        case 0x3e: /* G2: segment override DS | Brach hint: taken     */
        case 0x26: /* G2: segment override ES                         */
        case 0x64: /* G2: segment override FS                         */
        case 0x65: /* G2: segment override GS                         */
          break;
        default:
          goto decode_op;
      }
    }

    /* 3 prefixes and no jump to opcode decoding... bad instruction :( */
    fprintf(stderr, "error: Too much prefixes (max 3 prefixes).");
    return -1;
  }

  /* OPCODE DECODING */
decode_op:
  switch(mprefix)
  {
    case 0x80:
	printf("MOV 7\n");
	opsize = 7;
	break;
    case 0x00:
      if(*b == 0x0f)
      {
        /* 2 byte opcodes */
        switch(*++b)
        {
          case 0x06: /* CLTS */
            modrm  = 0;
            imm    = 0;
            break;

          case 0xbc: /* BSF mod_reg_r/m */
          case 0xbd: /* BSR mod_reg_r/m */
          case 0xa3: /* BT  mod_reg_r/m */
          case 0xbb: /* BTC mod_reg_r/m */
          case 0xb3: /* BTR mod_reg_r/m */
          case 0xab: /* BTS mod_reg_r/m */
          case 0x1f: /* NOP mod_reg_r/m */
            modrm = 1;
            break;

                     /* BT  ext_opcode imm8 */
                     /* BTC ext_opcode imm8 */
                     /* BTR ext_opcode imm8 */
          case 0xba: /* BTS ext_opcode imm8 */
            extra  = 1;
            modrm  = 0;
            imm    = 1;
            opsize = 0;
            break;

          default:
            if((*b & 0xf8) == 0xc8) /* BSWAP */
            {
              modrm = 0;
            } else
            if((*b & 0xfc) == 0x20) /* MOV DRx|CRx <-> x */
            {
              extra = 1;
            } else {                /* !!! CANNOT DECODE !!! */
              fprintf(stderr, "error: Bad 2 bytes opcode (%02x%02x).", 0x0f, *b);
              return -2;
            }
        }
      } else {
        /* 1 byte opcodes */
        switch(*b)
        {
          case 0x3f: /* AAS */
          case 0x98: /* CBW */
          case 0x99: /* CDQ */
          case 0xf9: /* CLC */
          case 0xfc: /* CLD */
          case 0xfa: /* CLI */
          case 0xf5: /* CMC */
          case 0x90: /* NOP */
          case 0xc3: /* RET */
            modrm  = 0;
            imm    = 0;
            break;

          case 0x37: /* AAA imm8 */
          case 0xd5: /* AAD imm8 */
          case 0xd4: /* AAM imm8 */
            modrm  = 0;
            imm    = 1;
            opsize = 0;
            break;

          case 0xe8: /* CALL imm */
            modrm  = 0;
            imm    = 1;
            opsize = 1;
            break;

          case 0xea: /* CALL selector:imm */
            modrm  = 0;
            extra  = 2;
            imm    = 1;
            opsize = 1;
            break;

          case 0x63: /* ARPL mod_reg_r16/m16 */
            modrm  = 1;
            wdsize = 2;
            break;

          case 0xff: /* CALL  mod_reg_r/m  */
                     /* PUSH  mod_reg_r/m  */
          case 0x8f: /* POP   mod_reg_r/m  */
          case 0x62: /* BOUND modA_reg_r/m */
          case 0x8d: /* LEA   modA_reg_r/m */
            modrm = 1;
            break;

          case 0x8e: /* MOV mod_sreg3_r/m */
          case 0x8c: /* MOV mod_sreg3_r/m */
            modrm  = 1;
            wdsize = 2;
            break;

          default:
            if((*b & 0xf8) == 0x58  /* POP  reg */
            || (*b & 0xf8) == 0x50) /* PUSH reg */
            {
              modrm = 0;
            } else
            if((*b & 0xfc) == 0x10  /* ADC mod_reg_r/m */
            || (*b & 0xfc) == 0x00  /* ADD mod_reg_r/m */
            || (*b & 0xfc) == 0x20  /* AND mod_reg_r/m */
            || (*b & 0xfc) == 0x88) /* MOV mod_reg_r/m */
            {
              modrm  = 1;
              opsize = (*b & 0x03) == 0x01;
            } else
            if((*b & 0xfc) == 0x80  /* ADC ext_opcode imm */
            || (*b & 0xfc) == 0x80  /* ADD ext_opcode imm */
            || (*b & 0xfc) == 0x80  /* AND ext_opcode imm */
            || (*b & 0xfe) == 0xc6) /* MOV ext_opcode imm */
            {
              extra = 1;
              imm   = 1;
              opsize = (*b & 0x03) == 0x01;
            } else
            if((*b & 0xfe) == 0x14  /* ADC  imm */
            || (*b & 0xfe) == 0x04  /* ADC  imm */
            || (*b & 0xfe) == 0x24  /* AND  imm */
            || (*b & 0xf0) == 0xb0  /* MOV  imm */
            || (*b & 0xfc) == 0x68) /* PUSH imm */
            {
              modrm  = 0;
              imm    = 1;
              opsize = 1;
            } else
            if((*b & 0xfc) == 0xa0  /* MOV [addr], x */
            || (*b & 0xfc) == 0xa0) /* MOV x, [addr] */
            {
              imm    = 0;
              opsize = 1;
            } else {                /* !!! CANNOT DECODE !!! */
              fprintf(stderr, "error: Bad opcode (%02x).", *b);
              return -2;
            }
        }
      }
      break;
    
    case 0x66:
      switch(*++b)
      {
      }
      break;

    case 0xf2:
      switch(*++b)
      {
      }
      break;

    case 0xf3:
      switch(*++b)
      {
      }
      break;

    default:
      fprintf(stderr, "error: Unknown mandatory prefix %02x.", *b);
      return -3;
  }
  b++;

  nbytes = b - p;
//printf("nbytes = %d\n  modrm = %d, imm = %d, opsize = %d, wdsize = %d, extra = %d, adsize = %d\n", nbytes, modrm, imm, opsize, wdsize, extra, adsize);

  /* DECODE MOD R/M */
  if(modrm)
  {
    nbytes++;

    /* mod 00 with reg=EBP => address offset */
    if((*b & 0xc7) == 0x06)
    {
//printf("  => offset address\n");
      //goto decode_sib;
      imm    = 1;
      opsize = 1;
      wdsize = adsize;
    } else
    /* mod 00 => only registers */
    if((*b & 0xc0) == 0x00)
    {
//printf("  => only regs\n");
      imm = 0;
    } else
    /* mod 01 => byte immediate */
    if((*b & 0xc0) == 0x40)
    {
//printf("  => byte\n");
      imm    = 1;
      opsize = 0;
    } else
    /* mod 01 => word immediate */
    if((*b & 0xc0) == 0x80)
    {
//printf("  => word\n");
      imm    = 1;
      opsize = 1;
    }

    /* CHECK FOR SIB <=> mod 00, mod 01 or mod 10 with reg=ESP */
    if(((*b & 0x07) == 0x04)
    && ((*b & 0xc0) == 0x80 || (*b & 0xc0) == 0x40 || (*b & 0xc0) == 0x00))
    {
//printf("  => decode SIB\n");
      nbytes++;
      b++;
      //goto decode_sib;
    }

    b++;
  }

  /* DECODE IMMEDIATES */
#if 0 //not used so far
decode_imm:
#endif
  if(imm)
    nbytes += opsize == 0 ? 1 : wdsize;

  /* LAST ADJUSTMENTS */
  nbytes += extra;

//printf("PUSH/POP!\n");
//printf("nbytes = %d\n  modrm = %d, imm = %d, opsize = %d, wdsize = %d, extra = %d, adsize = %d\n", nbytes, modrm, imm, opsize, wdsize, extra, adsize);

  return nbytes;
}

