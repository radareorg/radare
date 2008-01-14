/* -----------------------------------------------------------------------------
 * syn-intel.c
 *
 * Copyright (c) 2002, 2003, 2004 Vivek Mohan <vivek@sig9.com>
 * All rights reserved. See (LICENSE)
 * -----------------------------------------------------------------------------
 */

#include "types.h"
#include "extern.h"
#include "opcmap.h"
#include "syn.h"
#include "../../../utils.h"

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
	case 80: mkasm(u, "tbyte "); break;
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
			else	mkasm(u, "%s0x%lx", (op_f) ? "+" : "", op->lval.udword);
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
		if (syn_cast) opr_cast(u, op);
		switch (op->size) {
			case  8:
				mkasm(u, "0x" FMT64 "x", u->pc + op->lval.sbyte); 
				break;
			case 16:
				mkasm(u, "0x" FMT64 "x", u->pc + op->lval.sword);
				break;
			case 32:
				mkasm(u, "0x" FMT64 "x", u->pc + op->lval.sdword);
				break;
			default:break;
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

#define C_RESET   "\e[0m"
#define C_BWHITE  "\e[1;37m"
extern int udis86_color;

/* =============================================================================
 * translates to intel syntax 
 * =============================================================================
 */
extern void ud_translate_intel(struct ud* u)
{
  /* -- prefixes -- */
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
		case UD_Ijp:
		case UD_Ijo:
		case UD_Ija:
		case UD_Ijb:
		case UD_Ijae:
		case UD_Ijbe:
		case UD_Ijle:
		case UD_Ijge:
		case UD_Ijz:
		case UD_Ijnz:
		case UD_Ijs:
		case UD_Ijns:
		case UD_Ijmp:
		case UD_Icall:
			mkasm(u, "\e[32m");
			break;
		case UD_Iint:
			mkasm(u, "\e[31m");
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

  /* check if P_O32 prefix is used */
  if (! P_O32(u->mapen->prefix) && u->pfx_opr) {
	switch (u->dis_mode) {
		case 16: 
			mkasm(u, "o32 ");
			break;
		case 32:
		case 64:
 			mkasm(u, "o16 ");
			break;
	}
  }

  /* check if P_A32 prefix was used */
  if (! P_A32(u->mapen->prefix) && u->pfx_adr) {
	switch (u->dis_mode) {
		case 16: 
			mkasm(u, "a32 ");
			break;
		case 32:
 			mkasm(u, "a16 ");
			break;
		case 64:
 			mkasm(u, "a32 ");
			break;
	}
  }

  if (u->pfx_lock)
	mkasm(u, "lock ");
  if (u->pfx_rep)
	mkasm(u, "rep ");
  if (u->pfx_repne)
	mkasm(u, "repne ");

  /* print the instruction mnemonic */
  mkasm(u, "%s ", ud_lookup_mnemonic(u->mnemonic));

  /* operand 1 */
  if (u->operand[0].type != UD_NONE) {
	gen_operand(u, &u->operand[0], u->c1);
  }
  /* operand 2 */
  if (u->operand[1].type != UD_NONE) {
	mkasm(u, ", ");
	gen_operand(u, &u->operand[1], u->c2);
  }

  /* operand 3 */
  if (u->operand[2].type != UD_NONE) {
	mkasm(u, ", ");
	gen_operand(u, &u->operand[2], u->c3);
  }
if (udis86_color)
	pprintf("\e[0m");

}
