/*
 * Copyright (C) 2007, 2008
 *       esteve <youterm.com>
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* code analysis functions */

#include "../../main.h"
#include "../../radare.h"
#include "../../code.h"
#include "arm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static inline unsigned int disarm_branch_offset ( unsigned int pc, unsigned int insoff )
{
	unsigned int add;
		
	add = insoff << 2;
	/// zero extend si el bit mes alt es 1, --> 0x02000000
	if ( (add & 0x02000000) == 0x02000000 )
		add = add | 0xFC000000 ;
	add = add + pc + 8;

	return add;
}

static inline int anal_is_B  ( int inst )
{
	if  ( ( inst & ARM_BRANCH_I_MASK  ) == ARM_BRANCH_I )
		return 1;
	 return 0;
}

static inline int anal_is_BL  ( int inst )
{
	
	if ( anal_is_B(inst) && ( inst & ARM_BRANCH_LINK ) == ARM_BRANCH_LINK  )
		return 1;
	return 0;
}


static inline int anal_is_return ( int inst )
{
	if ( ( inst & ( ARM_DTM_I_MASK | ARM_DTM_LOAD  | ( 1 << 15 ) ) ) == ( ARM_DTM_I | ARM_DTM_LOAD  | ( 1 << 15 ) )  )
		return 1;
	return 0;
}

static inline int anal_is_unkjmp ( int inst )
{
	if ( (inst & ( ARM_DTX_I_MASK | ARM_DTX_LOAD  | ( ARM_DTX_RD_MASK ) ) ) == ( ARM_DTX_LOAD | ARM_DTX_I | ( ARM_PC << 12 ) ) )
		return 1;
	return 0;
}

static inline int anal_is_condAL ( int inst )
{
	if ( ( inst &ARM_COND_MASK ) == ARM_COND_AL )
		return 1;
	return 0;
}

static inline int anal_is_exitpoint ( int inst )
{
	return ( anal_is_B ( inst )  || anal_is_return ( inst ) || anal_is_unkjmp ( inst ) );
}

extern int arm_mode;

int arch_arm_aop(u64 addr, const u8 *codeA, struct aop_t *aop)
{
	unsigned int i=0;
	unsigned int* code=codeA;

	unsigned int branch_dst_addr;
	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;
#if 0
	fprintf(stderr, "CODE %02x %02x %02x %02x\n",
		codeA[0], codeA[1], codeA[2], codeA[3]);
#endif

	if ( anal_is_exitpoint ( code[i] ) )
	{
		branch_dst_addr =  disarm_branch_offset ( addr, code[i]&0x00FFFFFF ) ;

		if ( anal_is_BL ( code[i] )  ) {
			if ( anal_is_B ( code[i] ) ) {
				aop->type = AOP_TYPE_CALL;
				aop->jump = branch_dst_addr;
				aop->fail = addr + 4 ;
				aop->eob  = 1;
			} else {
				//aop->type = AOP_TYPE_UJMP;
				//aop->eob = 1;
				return (arm_mode==16)?2:4;
			}
		} else {
			if ( anal_is_B ( code[i] ) ) {
				if ( anal_is_condAL (code[i] )  ) {
					aop->type = AOP_TYPE_JMP;
					aop->jump = branch_dst_addr;
					aop->eob = 1;
				} else {
					aop->type = AOP_TYPE_CJMP;
					aop->jump = branch_dst_addr;
					aop->fail = addr + 4;
					aop->eob  = 1;
				}
			} else {
				//unknown jump o return
				aop->type = AOP_TYPE_UJMP;
				aop->eob = 1;
				return (arm_mode==16)?2:4;
			}
		}
	}

	return (arm_mode==16)?2:4;
}



// NOTE: bytes should be at least 16 bytes!
#if 0
int arch_arm_aop(unsigned long addr, const unsigned char *codeA, struct aop_t *aop)
{
	unsigned int i=0;
	unsigned int* code=codeA;

	unsigned int branch_dst_addr;
	memset(aop, '\0', sizeof(struct aop_t));

	// unknown jump
	if ( (code[i] & ( ARM_DTX_I_MASK | ARM_DTX_LOAD
	| ( ARM_DTX_RD_MASK ) ) ) == ( ARM_DTX_LOAD | ARM_DTX_I
	| ( ARM_PC << 12 ) ) ) {
		if ( (code[i] & ARM_COND_MASK ) == ARM_COND_AL ) {
			//aop->type = AOP_TYPE_UJMP;
			aop->eob = 1;
			return 4;
		}
	}

	// ret
	if ( (code[i]& ( ARM_DTM_I_MASK | ARM_DTM_LOAD 
	|  ( 1 << 15 ) ) ) == ( ARM_DTM_I | ARM_DTM_LOAD
	|  ( 1 << 15 ) )  ) {
		if ( (code[i] & ARM_COND_MASK ) == ARM_COND_AL ) {
			aop->type = AOP_TYPE_RET;
			aop->eob  = 1;
			return 4;
		}
	}

	if ( (code[i]& ARM_BRANCH_I  ) == ARM_BRANCH_I  ) {
		// CALL
		if ( (code[i]&ARM_BRANCH_LINK ) == ARM_BRANCH_LINK ) {	// BL {
			branch_dst_addr =  disarm_branch_offset ( addr*4, code[i]&0x00FFFFFF ) ;
			if ( branch_dst_addr !=  (addr*4 ) ) {
				aop->type = AOP_TYPE_CALL;
				aop->jump = branch_dst_addr;
				aop->fail = addr + 4;
				aop->eob  = 1;
			}
		} else {
			// B
			if ( (code[i]&ARM_COND_MASK ) == ARM_COND_AL ) {
				branch_dst_addr =  disarm_branch_offset ( addr*4, code[i]&0x00FFFFFF ) ;
				if ( branch_dst_addr !=  (addr*4 ) ) {
					aop->type   = AOP_TYPE_JMP;
					aop->jump = branch_dst_addr;
					aop->eob = 1;
				}
			} else {
				branch_dst_addr =  disarm_branch_offset ( addr*4, code[i]&0x00FFFFFF ) ;
				if ( branch_dst_addr !=  (addr*4 ) ) {
					aop->type = AOP_TYPE_CJMP;
					aop->jump = branch_dst_addr;
					aop->fail = addr + 4;
					aop->eob = 1;
				}
			}
		}
	}

	return 4;
}

#endif
