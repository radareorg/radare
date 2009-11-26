/*
 * Copyright (C) 2007, 2008
 *       pancake <youterm.com>
 *       esteve <eslack.org>
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
#include "../../code.h"
#include <string.h>

/* arch_aop for x86 */

/* 64b dislen workaround */
#include "arch/x86/udis86/types.h"
#include "arch/x86/udis86/extern.h"

// CMP ARG1
// 837d0801        cmp dword [ebp+0x8], 0x1
// SET VAR_41c
// 8985e4fbffff    mov [ebp-41C],eax 
// GET VAR_41c
// 8b85e4fbffff    mov eax,[ebp-41C]
// 8b450c          mov eax,[ebp+C] 
// 8d85e8fbffff    lea eax,[ebp-418]
// c68405e7fbffff. mov byte ptr [ebp+eax-419],0x0


// NOTE: bytes should be at least 16 bytes!
// XXX addr should be off_t for 64 love
int arch_x86_aop(ut64 addr, const u8 *bytes, struct aop_t *aop)
{
	if (aop == NULL)
		return dislen((unsigned char *)bytes, 64);

	memset(aop, '\0', sizeof(struct aop_t));
	aop->type = AOP_TYPE_UNK;

	switch(bytes[0]) {
	case 0x8a:
	case 0x8b:
	case 0x03: //  034518          add eax, [ebp+0x18]
		switch(bytes[1]) {
		case 0x45:
		case 0x46:
		case 0x55:
			/* mov -0xc(%ebp, %eax */
			aop->ref = (ut64)(int)(-((char)bytes[2]));
			aop->stackop = AOP_STACK_LOCAL_GET;
			break;

		case 0x95:
			if (bytes[2]==0xe0) { // ebp
				aop->ref = (ut64)(-((int)(bytes[3]+(bytes[4]<<8)+(bytes[5]<<16)+(bytes[6]<<24))));
				aop->stackop = AOP_STACK_LOCAL_GET;
			}
			//aop->ref = -(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24));
			break;
		case 0xbd:
			aop->ref = (ut64)(-((int)(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24))));
			//aop->ref = -(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24));
			aop->stackop = AOP_STACK_LOCAL_GET;
			break;
		}
		break;
	case 0x88:
	case 0x89: // move
		switch(bytes[1]) {
		case 0x45:
		case 0x4d: //  894de0          mov [ebp-0x20], ecx 
		case 0x55:
			aop->stackop = AOP_STACK_LOCAL_SET;
			aop->ref = (ut64)-((char)bytes[2]);
			break;
		case 0x85:
			aop->stackop = AOP_STACK_LOCAL_SET;
			aop->ref = (ut64)-((int)(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24)));
			break;
		case 0x75:
			aop->stackop = AOP_STACK_LOCAL_GET;
			aop->ref = (ut64)(bytes[2]); //+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24));
			break;
		}
		aop->type   = AOP_TYPE_MOV;
		aop->length = 2;
		break;
	case 0xf4: // hlt
		aop->type   = AOP_TYPE_RET;
		aop->length = 1;
		aop->eob = 1;
		break;
	case 0xc3: // ret
	case 0xc2: // ret + 2 bytes
	case 0xcb: // lret
	case 0xcf: // iret
		aop->type   = AOP_TYPE_RET;
	//	aop->length = 1;
		aop->eob = 1;
		break;
	//case 0xea: // far jmp
	// TODO moar
	case 0x3b: //cmp
		aop->ref = (ut64)(-((char)bytes[2]));
		aop->stackop = AOP_STACK_LOCAL_GET;
	case 0x39:
	case 0x3c:
	case 0x3d:
	case 0x85:
		aop->type   = AOP_TYPE_CMP;
		aop->length = 2;
		break;
	case 0x90:
		aop->type   = AOP_TYPE_NOP;
		aop->length = 1;
		break;
	case 0x0f: // 3 byte nop
		//0fbe55ff        movsx edx, byte [ebp-0x1]
		if (bytes[1]==0xbe) {
			aop->ref = (ut64)(int)(-((char)bytes[3]));
			aop->stackop = AOP_STACK_LOCAL_GET;
		} else
		if (bytes[1]==0x31) {
			// RDTSC // colorize or sthg?
			aop->eob = 0;
		} else
		if (bytes[1]>=0x18 && bytes[1]<=0x1f) {
			aop->type = AOP_TYPE_NOP;
			aop->length = 3;
		} else
		if (bytes[1]>=0x80 && bytes[1]<=0x8f) {
			aop->type   = AOP_TYPE_CJMP;
			aop->jump   = addr+6+bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24);//((unsigned long)((bytes+2))+6);
			aop->fail   = addr+6;
			aop->length = 6;
			aop->eob    = 1;
		} 
		break;
	case 0xcc: // int3
		aop->eob = 1;
	case 0xf1: // int1
		aop->length = 1;
		aop->type   = AOP_TYPE_SWI;
		break;
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		aop->type = AOP_TYPE_MOV; //  bfdc054000      mov edi, 0x4005dc
		aop->ref = bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);//((unsigned long)((bytes+2))+6);
		break;
	case 0xcd:
		aop->length = 2;
		aop->type   = AOP_TYPE_SWI;
		break;
	case 0xe8: // call
		aop->type   = AOP_TYPE_CALL;
		aop->length = 5;
		//aop->jump   = addr+*ptr+5; //(unsigned long)((bytes+1)+5);
		aop->jump   = addr+5+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);//((unsigned long)((bytes+2))+6);
		aop->fail   = addr+5;
//printf("addr: %08llx\n call %08llx \n ret %08llx\n", addr, aop->jump, aop->fail);
	//	aop->eob    = 1;
		break;
	case 0xe9: // jmp
		aop->type   = AOP_TYPE_JMP;
		aop->length = 5;
		//aop->jump   = (unsigned long)((bytes+1)+5);
		aop->jump   = addr+5+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);//((unsigned long)((bytes+2))+6);
		aop->fail   = 0L;
		aop->eob    = 1;
		break;
	case 0xeb: // short jmp 
		aop->type   = AOP_TYPE_JMP;
		aop->length = 2;
		aop->jump   = addr+((unsigned long)((char)bytes[1])+2);
		aop->fail   = 0L;
		aop->eob    = 1;
		break;
	case 0xf2: // repnz
	case 0xf3: // repz
		aop->type   = AOP_TYPE_REP;
		//aop->length = dislen((unsigned char *)&bytes); //instLength(bytes, 16, 0);
		aop->jump   = 0L;
		aop->fail   = 0L;
		break;
	case 0xff:
		if (bytes[1]== 0x75) {
			aop->type = AOP_TYPE_PUSH;
			aop->stackop = AOP_STACK_ARG_GET;
			aop->ref = 0LL;
			aop->ref = (ut64)(((char)(bytes[2])));
		} else
		if (bytes[1]== 0x45) {
			aop->type = AOP_TYPE_ADD;
			aop->stackop = AOP_STACK_LOCAL_SET;
			aop->ref = (ut64)(-((char)bytes[2]));
		} else
		if (bytes[1]>=0x50 && bytes[1]<=0x6f) {
			aop->type = AOP_TYPE_UJMP;
			aop->eob    = 1;
		} else
		if (bytes[1]>=0xd0 && bytes[1]<=0xd7) {
			aop->type = AOP_TYPE_CALL;
			aop->length = 2;
			aop->eob    = 1;
			aop->jump   = vm_arch_x86_regs[VM_X86_EAX+bytes[1]-0xd0];
			aop->fail   = addr+2;
		} else
		if (bytes[1]>=0xd8 && bytes[1]<=0xdf) {
			aop->type = AOP_TYPE_CALL;
			aop->length = 2;
			aop->eob    = 1;
			aop->jump   = vm_arch_x86_regs[VM_X86_EAX+bytes[1]-0xd8];
			aop->fail   = addr+2;
		} else
		if (bytes[1]>=0xe0 && bytes[1]<=0xe7) {
			aop->type = AOP_TYPE_UJMP;
			aop->length = 2;
			aop->jump   = vm_arch_x86_regs[VM_X86_EAX+bytes[1]-0xe0];
			aop->eob    = 1;
		} else
		if (bytes[1]>=0xe8 && bytes[1]<=0xef) {
			aop->type = AOP_TYPE_UJMP;
			aop->length = 2;
			aop->jump   = vm_arch_x86_regs[VM_X86_EAX+bytes[1]-0xe8];
			aop->eob    = 1;
		} else
		if (bytes[1]>=0xf0 && bytes[1]<=0xf7) {
			aop->type = AOP_TYPE_UPUSH;
			aop->ref = bytes[0]-0xf0; // TODO value of register here! get_offset
		}
		break;
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
		aop->type = AOP_TYPE_UPUSH;
		aop->ref = bytes[0]-0x50; //0; // TODO value of register here! get_offset
		break;
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		aop->type = AOP_TYPE_POP;
		aop->length = 1;
		break;
	case 0x68:
		aop->type = AOP_TYPE_PUSH;
		aop->ref = bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0x81:
		if (bytes[1] == 0xec) {
			/* sub $0x????????, $esp*/
  			// 81ece00d0000    sub esp, 0xde0 ; 
			aop->value = bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24);
			aop->stackop = AOP_STACK_INCSTACK;
			break;
		}
		aop->type = AOP_TYPE_ADD;
		break;
	case 0x83:
		switch(bytes[1]) {
		case 0xe4: // and
			aop->value = (ut64)(unsigned char)bytes[2];
			aop->type = AOP_TYPE_AND;
			break;
		case 0xc4:
			/* inc $0x????????, $esp*/
			aop->value = -(ut64)(unsigned char)bytes[2];
			aop->stackop = AOP_STACK_INCSTACK;
			break;
		case 0xec:
			/* sub $0x????????, $esp*/
			aop->value = (ut64)(unsigned char)bytes[2];
			aop->stackop = AOP_STACK_INCSTACK;
			break;
		case 0xbd: /* 837dfc02        cmp dword [ebp-0x4], 0x2 */
			switch(bytes[2]) {
			case 0xe0: // ebp
				if ((char)bytes[2]>0) {
					aop->stackop = AOP_STACK_ARG_GET;
					aop->value = bytes[3]+(bytes[4]<<8)+(bytes[5]<<16)+(bytes[6]<<24);
				} else {
					aop->stackop = AOP_STACK_LOCAL_GET;
					aop->value = bytes[3]+(bytes[4]<<8)+(bytes[5]<<16)+(bytes[6]<<24);
				}
				aop->type = AOP_TYPE_CMP;
				break;
			}
			break;
		case 0x7d: /* 837dfc02        cmp dword [ebp-0x4], 0x2 */
			if ((char)bytes[2]>0) {
				aop->stackop = AOP_STACK_ARG_GET;
				aop->value = (ut64)(char)bytes[2];
			} else {
				aop->stackop = AOP_STACK_LOCAL_GET;
				aop->value = (ut64)-(char)bytes[2];
			}
			aop->type = AOP_TYPE_CMP;
			break;
		}
		break;
	case 0x8d:
		/* LEA */
		if (bytes[1] == 0x85) {
			aop->ref = (ut64)(-((int)(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24))));
			aop->stackop = AOP_STACK_LOCAL_GET;
		}
		aop->type =AOP_TYPE_MOV;
		break;
	case 0xc7:
		/* mov dword [ebp-0xc], 0x0  ||  c7 45 f4 00000000 */
		switch(bytes[1]) {
		case 0x85:
			aop->stackop = AOP_STACK_LOCAL_SET;
			//aop->ref = (ut64)(((int)(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24))));
			aop->ref = ((int)(bytes[2]+(bytes[3]<<8)+(bytes[4]<<16)+(bytes[5]<<24)));
			break;
 			//c785 e4fbffff 00. mov dword [ebp+0xfffffbe4], 0x0
		case 0x45:
			aop->stackop = AOP_STACK_LOCAL_SET;
			aop->ref = (ut64)(-((char)bytes[2]));
			break;
		case 0x04:
			// c7042496850408    dword [esp] = 0x8048596 ; LOL
			aop->ref = (ut64)(((int)(bytes[3]+(bytes[4]<<8)+(bytes[5]<<16)+(bytes[6]<<24))));
			break;
		}
		aop->type = AOP_TYPE_STORE;
		break;
	case 0x82:
		aop->type = AOP_TYPE_ADD;
		break;
	case 0x29:
		aop->type = AOP_TYPE_SUB;
		break;
	case 0x31:
		aop->type = AOP_TYPE_XOR;
		break;
	case 0x32:
		aop->type = AOP_TYPE_AND;
		break;

	case 0xa1: // mov eax, [addr]
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_EAX] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		radare_read_at((ut64)vm_arch_x86_regs[VM_X86_EAX], (unsigned char *)&(vm_arch_x86_regs[VM_X86_EAX]), 4);
		break;
		
	// roll to a switch range case
#if 0
	case 0xb8: // mov eax, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_EAX] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0xb9: // mov ecx, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_ECX] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0xba: // mov edx, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_EDX] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0xbb: // mov ebx, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_EBX] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0xbc: // mov esp, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_ESP] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
	case 0xbd: // mov esp, <inmedate>
		aop->type = AOP_TYPE_MOV;
		vm_arch_x86_regs[VM_X86_EBP] = addr+bytes[1]+(bytes[2]<<8)+(bytes[3]<<16)+(bytes[4]<<24);
		break;
#endif
#if 0
	case0xF
		/* conditional jump */
		if (bytes[1]>=0x80&&bytes[1]<=0x8F) {
			aop->type   = AOP_TYPE_CJMP;
			aop->length = 6;
			aop->jump   = (unsigned long)((bytes+2)+6);
			aop->fail   = addr+6;
			aop->eob    = 1;
			return 5;
		}
		break;
#endif
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f: {
		int bo = (int)((char) bytes[1]);
		/* conditional jump */
		//if (bytes[1]>=0x80&&bytes[1]<=0x8F) {
			aop->type   = AOP_TYPE_CJMP;
			aop->length = 2;
		//	aop->jump   = (unsigned long)((bytes+2)+6);
			aop->jump   = addr+bo+2; //(unsigned long)((bytes+1)+5);
			aop->fail   = addr+2;
			aop->eob    = 1;
		}
		break;
	//default:
		//aop->type = AOP_TYPE_UNK;
	}
		
   	static ud_t disasm_obj;
   	ud_set_mode(&disasm_obj, config_get_i("asm.bits"));
   	ud_set_pc(&disasm_obj, addr);
   	ud_set_input_buffer(&disasm_obj, (unsigned char *)bytes, 64);
   	ud_disassemble(&disasm_obj);
	aop->length = ud_insn_len(&disasm_obj);
	//if (aop->length == 0)
		//aop->length = dislen((unsigned char *)bytes, 64); //instLength(bytes, 16, 0);
		//aop->length = instLength(bytes, 16, 0);
	//if (!(aop->jump>>33))
	//	aop->jump &= 0xFFFFFFFF; // XXX may break on 64 bits here

	return aop->length;
}
