/* -----------------------------------------------------------------------------
 * syn-intel.c
 *
 * Copyright (c) 2002, 2003, 2004 Vivek Mohan <vivek@sig9.com>
 * All rights reserved. See (LICENSE)
 * -----------------------------------------------------------------------------
 */

#include "types.h"
#include "extern.h"
#include "decode.h"
#include "itab.h"
#include "syn.h"

/* -----------------------------------------------------------------------------
 * opr_cast() - Prints an operand cast.
 * -----------------------------------------------------------------------------
 */
static void 
opr_cast(struct ud* u, struct ud_operand* op)
{
  switch(op->size) {
	case  8: mkasm(u, "byte " ); break;
	case 16: mkasm(u, "word " ); break;
	case 32: mkasm(u, "dword "); break;
	case 64: mkasm(u, "qword "); break;
	case 80: mkasm(u, "tword "); break;
	default: break;
  }
  if (u->br_far)
	mkasm(u, "far "); 
  else if (u->br_near)
	mkasm(u, "near ");
}

/* -----------------------------------------------------------------------------
 * gen_operand() - Generates assembly output for each operand.
 * -----------------------------------------------------------------------------
 */
static void gen_operand(struct ud* u, struct ud_operand* op, int syn_cast)
{
  switch(op->type) {
	case UD_OP_REG:
		mkasm(u, ud_reg_tab[op->base - UD_R_AL]);
		break;

	case UD_OP_MEM: {

		int op_f = 0;

		if (syn_cast) 
			opr_cast(u, op);

		mkasm(u, "[");

		if (u->pfx_seg)
			mkasm(u, "%s:", ud_reg_tab[u->pfx_seg - UD_R_AL]);

		if (op->base) {
			mkasm(u, "%s", ud_reg_tab[op->base - UD_R_AL]);
			op_f = 1;
		}

		if (op->index) {
			if (op_f)
				mkasm(u, "+");
			mkasm(u, "%s", ud_reg_tab[op->index - UD_R_AL]);
			op_f = 1;
		}

		if (op->scale)
			mkasm(u, "*%d", op->scale);

		if (op->offset == 8) {
			if (op->lval.sbyte < 0)
				mkasm(u, "-0x%x", -op->lval.sbyte);
			else	mkasm(u, "%s0x%x", (op_f) ? "+" : "", op->lval.sbyte);
		}
		else if (op->offset == 16)
			mkasm(u, "%s0x%x", (op_f) ? "+" : "", op->lval.uword);
		else if (op->offset == 32) {
			if (u->adr_mode == 64) {
				if (op->lval.sdword < 0)
					mkasm(u, "-0x%x", -op->lval.sdword);
				else	mkasm(u, "%s0x%x", (op_f) ? "+" : "", op->lval.sdword);
			} 
			else {
				if (op->lval.sdword < 0)
					mkasm(u, "-0x%lx", -op->lval.udword);
				else	mkasm(u, "%s0x%lx", (op_f) ? "+" : "", op->lval.udword);
			}
		}
		else if (op->offset == 64) 
			mkasm(u, "%s0x" FMT64 "x", (op_f) ? "+" : "", op->lval.uqword);

		mkasm(u, "]");
		break;
	}
			
	case UD_OP_IMM:
		if (syn_cast) opr_cast(u, op);
		switch (op->size) {
			case  8: mkasm(u, "0x%x", op->lval.ubyte);    break;
			case 16: mkasm(u, "0x%x", op->lval.uword);    break;
			case 32: mkasm(u, "0x%lx", op->lval.udword);  break;
			case 64: mkasm(u, "0x" FMT64 "x", op->lval.uqword); break;
			default: break;
		}
		break;

	case UD_OP_JIMM:
{
	char *fmt = "0x%x";
	if (u->dis_mode==64) fmt = "0x"FMT64"x";
		switch (op->size) {
			case  8:
				mkasm(u, fmt, u->pc + op->lval.sbyte); 
				break;
			case 16:
				mkasm(u, fmt, u->pc + op->lval.sword);
				break;
			case 32:
				mkasm(u, fmt, u->pc + op->lval.sdword);
				break;
			default:break;
		}
}
		break;

	case UD_OP_PTR:
		switch (op->size) {
			case 32:
				mkasm(u, "word 0x%x:0x%x", op->lval.ptr.seg, 
					op->lval.ptr.off & 0xFFFF);
				break;
			case 48:
				mkasm(u, "dword 0x%x:0x%lx", op->lval.ptr.seg, 
					op->lval.ptr.off);
				break;
		}
		break;

	case UD_OP_CONST:
		if (syn_cast) opr_cast(u, op);
		mkasm(u, "%d", op->lval.udword);
		break;

	default: return;
  }
}

//#define C_RESET   "\e[0m"
//#define C_BWHITE  "\e[1;37m"
#define CHK_ARGS1 if ((u->operand[0].type == UD_NONE)) break;
#define CHK_ARGS2 if ((u->operand[0].type == UD_NONE) || (u->operand[1].type == UD_NONE)) break;
#define CHK_ARGS3 if ((u->operand[0].type == UD_NONE) || (u->operand[1].type == UD_NONE) || (u->operand[2].type == UD_NONE)) break;
#define ARG_1     if (u->operand[0].type != UD_NONE)\
					    gen_operand(u, &u->operand[0], u->c1);
#define ARG_2     if (u->operand[1].type != UD_NONE)\
					    gen_operand(u, &u->operand[1], u->c2);
#define ARG_3     if (u->operand[2].type != UD_NONE)\
					    gen_operand(u, &u->operand[2], u->c3);

extern int udis86_color;

/* =============================================================================
 * translates to pseudo syntax 
 * =============================================================================
 */
extern void ud_translate_pseudo(struct ud* u)
{
	struct ud_operand *op = &u->operand[0];

#if 0
	if (udis86_color) switch(u->mnemonic) {
		case UD_Itest:
		case UD_Icmp:
			mkasm(u, "\e[36m");
			break;
		case UD_Ipush:
		case UD_Ipop:
			mkasm(u, "\e[33m");
			break;
		case UD_Iret:
		case UD_Inop:
		case UD_Irdtsc:
			mkasm(u, C_BWHITE);
			break;
		case UD_Iint:
			mkasm(u, "\e[31m");
			break;
		case UD_Ijp:
		case UD_Ijo:
		case UD_Ijg:
		case UD_Ijge:
		case UD_Ijae:
		case UD_Ijbe:
		case UD_Ija:
		case UD_Ijb:
		case UD_Ijle:
		case UD_Ijz:
		case UD_Ijnz:
		case UD_Ijs:
		case UD_Ijns:
		case UD_Ijmp:
		case UD_Icall:
			mkasm(u, "\e[32m");
			break;
		case UD_Iinvalid:
			mkasm(u, "\e[31m");
			break;
		case UD_Ifstp:
		case UD_Ifld:
		case UD_Ifsub:
		case UD_Ifcom: /* fp */
		case UD_Ifcomi:
		case UD_Ifcomip:
		case UD_Ifcomp:
		case UD_Ificom: /* integer */
		case UD_Ificomp:
		case UD_Ifucom: /* unordered */
		case UD_Ifucomi:
		case UD_Ifucomp:
		case UD_Ifucomip:
		case UD_Iftst:
		case UD_Ifcompp:
		case UD_Ifucompp:
		case UD_Ifxam:
		case UD_Isahf:
		case UD_Ifild:
		case UD_Ifist:
		case UD_Ifistp:
		case UD_Ifbld:
		case UD_Ifbstp:
		case UD_Ifabs:
		case UD_Ifadd:
		case UD_Ifaddp:
		case UD_Ifchs:
		case UD_Ifmul:
		case UD_Ifmulp:
		case UD_Ifdivp:
		case UD_Ifdivr:
		case UD_Ifdivrp:
			mkasm(u, "\e[36m");
		default:
			break;
	}
#endif
	/* -- prefixes -- */

	if (u->pfx_lock)
		mkasm(u, "lock ");
	if (u->pfx_rep || u->pfx_repne)
		mkasm(u, "@");
	// END OF TRASH !! //

	// print real opcode
	switch(u->mnemonic) {
		case UD_Ipush:
			mkasm(u, "  push "); ARG_1
			break;
		case UD_Irol:
		case UD_Ishl:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " <<= ");
			if (u->operand[1].type != UD_NONE) ARG_2
				break;
			if (u->mnemonic == UD_Ishl)
				mkasm(u, " (zerofill)");
		case UD_Ishr:
		case UD_Iror:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " >>= ");
			if (u->operand[1].type != UD_NONE) ARG_2
				if (u->mnemonic == UD_Ishr)
					mkasm(u, " (zerofill)");
			break;
		case UD_Ixchg:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " <=> "); ARG_2
				break;
		case UD_Ilea:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " (lea)= ");   ARG_2
				break;
		case UD_Imov:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " = ");   ARG_2
				break;
		case UD_Iinc:
			CHK_ARGS1 mkasm(u, "  "); ARG_1 mkasm(u, "++ ");
			break;
		case UD_Idec:
			CHK_ARGS1 mkasm(u, "  "); ARG_1 mkasm(u, "-- ");
			break;
		case UD_Ior:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " |= "); ARG_2
			break;
		case UD_Iand:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " &= "); ARG_2
			break;
		case UD_Iadd:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " += "); ARG_2
				// TODO: if arg2 == 0 then i'm a nop!
			break;
		case UD_Ijmp: {
			if (op->lval.sbyte < 0) {
				CHK_ARGS1 mkasm(u, "^ goto ");
			} else {
				CHK_ARGS1 mkasm(u, "v goto ");
			}
			if (u->br_far)
				mkasm(u,"far ");
			ARG_1
			}
			break;
		case UD_Icall: {
			long long l = (long long)(op->lval.sdword);
			if (l< 0) {
				CHK_ARGS1 mkasm(u, "^ call ");
			} else {
				CHK_ARGS1 mkasm(u, "v call ");
			} ARG_1 }
			break;
		case UD_Ixor:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " ^= "); ARG_2
			break;
		case UD_Isub:
			CHK_ARGS2 mkasm(u, "  "); ARG_1 mkasm(u, " -= "); ARG_2
			break;
		case UD_Ileave:
			mkasm(u, " leave ;--");
			CHK_ARGS2 
			break;
		case UD_Icbw:
			mkasm(u, "  ax = (short) al");
			break;
		case UD_Icwde:
			mkasm(u, "  eax = (int) ax");
			break;
		case UD_Iret:
		case UD_Iretf:
			mkasm(u, " ret ;--"); ARG_1
				break;
			/* THE FPU */
		case UD_Ifstp:
			mkasm(u, "f fstp "); ARG_1 
				break;
		case UD_Ifld:
			mkasm(u, "f fld "); ARG_1 
				break;
		case UD_Ifcom: /* fp */
		case UD_Ifcomi:
		case UD_Ifcomip:
		case UD_Ifcomp:
		case UD_Ificom: /* integer */
		case UD_Ificomp:
		case UD_Ifucom: /* unordered */
		case UD_Ifucomi:
		case UD_Ifucomp:
		case UD_Ifucomip:
			mkasm(u, "f fcmp ST(0), "); ARG_1
				if ((u->mnemonic == UD_Ifcomp)
						|| (u->mnemonic == UD_Ifcomip)
						|| (u->mnemonic == UD_Ifucomp)
						|| (u->mnemonic == UD_Ifucomip)
						|| (u->mnemonic == UD_Ificomp))
					mkasm(u, " && pop ST(0)");
			break;
		case UD_Iftst:
			mkasm(u, "f fcmp ST(0), 0f");
			break;
		case UD_Ifcompp:
		case UD_Ifucompp:
			mkasm(u, "f fcmp ST(0), ST(1) && pop ST(0,1)");
			break;
		case UD_Ifxam:
			// TODO
			mkasm(u, "f examine ST(0)");
			break;
		case UD_Isahf:
			mkasm(u, "f fpu->cpu eflags");
			break;
		case UD_Ifild:
			mkasm(u, "f fload (int)"); ARG_1
				break;
		case UD_Ifist:
		case UD_Ifistp:
			mkasm(u, "f fstore (int)"); ARG_1
				if (u->mnemonic == UD_Ifistp)
					mkasm(u, " && pop ST(0)");
			break;
		case UD_Ifbld:
			mkasm(u, "f fload (bcd)"); ARG_1
				break;
		case UD_Ifbstp:
			mkasm(u, "f fstore (bcd)"); ARG_1
				break;
		case UD_Ifabs:
			mkasm(u, "f abs( ST(0) )");
			break;
		case UD_Ifsub:
			mkasm(u, "f ");
			ARG_1 mkasm(u, " - ST(0)"); 
			break;
		case UD_Ifadd:
			mkasm(u, "f ");
			ARG_1 mkasm(u, " + ST(0)"); 
			break;
		case UD_Ifaddp:
			ARG_1 mkasm(u, " + "); ARG_2
				mkasm(u, " && pop ST(0)");
			break;
		case UD_Ifchs:
			mkasm(u, "f ST(0) = -ST(0)");
			break;
		case UD_Ifmul:
		case UD_Ifmulp:
			mkasm(u, "f ");
			ARG_1 mkasm(u, " * "); ARG_2
				if (u->mnemonic == UD_Ifaddp)
					mkasm(u, " && pop ST(0)");
			break;
		case UD_Ifdivp:
			mkasm(u, "f ");
			ARG_1 mkasm(u, " * "); ARG_2
				mkasm(u, " && pop ST(0)");
			break;
		case UD_Ifdivr:
		case UD_Ifdivrp:
			mkasm(u, "f ");
			ARG_1 mkasm(u, " / "); ARG_2
				if (u->mnemonic == UD_Ifdivp)
					mkasm(u, " && pop ST(0)");
			break;
		case UD_Irep:
			mkasm(u, "@ rep"); ARG_1;
			break;
		case UD_Ijle:
		case UD_Ijge:
		case UD_Ija:
		case UD_Ijb:
		case UD_Ijnz:
		case UD_Ijz:
			if (op->lval.sbyte < 0) {
				mkasm(u, "^ %s ", ud_lookup_mnemonic(u->mnemonic));
				ARG_1;
			} else {
				mkasm(u, "v %s ", ud_lookup_mnemonic(u->mnemonic));
				ARG_1;
			}
			break;
		default:
			switch(u->mnemonic) {
				case UD_Ijmp:
					mkasm(u, "= ");
					break;
				case UD_Ijle:
				case UD_Ijge:
				case UD_Ija:
				case UD_Ijb:
				case UD_Ijnz:
				case UD_Ijz:
					mkasm(u, "- ");
					break;
				default:
					mkasm(u, "  ");
			}
			ud_translate_intel(u);
			break;
	}
	if (udis86_color)
		mkasm(u, "\e[36m");
}
