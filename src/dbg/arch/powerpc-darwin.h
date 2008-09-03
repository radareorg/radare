#ifndef _INCLUDE_PPC_DARWIN_H_
#define _INCLUDE_PPC_DARWIN_H_

#if __POWERPC__
#include <mach/ppc/_types.h>
#include <mach/ppc/thread_status.h>
#else
#include <mach/i386/_structs.h>
#include <mach/i386/thread_status.h>
#endif

//#define regs_t struct ppc_thread_state_t
#define regs_sizeof sizeof(regs_t)/4
//#define regs_t i386_thread_state_t

#define R_EIP(x) x.srr0 
#define R_EFLAGS(x) x.srr1
#define R_SRR0(x) x.srr0 /* program counter */
#define R_SRR1(x) x.srr1 /* status register  ( supervisor ) */
#define R_R0(x) x.r0
#define R_R1(x) x.r1
#define R_R2(x) x.r2
#define R_R3(x) x.r3
#define R_R4(x) x.r4
#define R_R5(x) x.r5
#define R_R6(x) x.r6
#define R_R7(x) x.r7
#define R_R8(x) x.r8
#define R_R9(x) x.r9
#define R_R10(x) x.r10
#define R_R11(x) x.r11
#define R_R12(x) x.r12
#define R_R13(x) x.r13
#define R_R14(x) x.r14
#define R_R15(x) x.r15
#define R_R16(x) x.r16
#define R_R17(x) x.r17
#define R_R18(x) x.r18
#define R_R19(x) x.r19
#define R_R20(x) x.r20
#define R_R21(x) x.r21
#define R_R22(x) x.r22
#define R_R23(x) x.r23
#define R_R24(x) x.r24
#define R_R25(x) x.r25
#define R_R26(x) x.r26
#define R_R27(x) x.r27
#define R_R28(x) x.r28
#define R_R29(x) x.r29
#define R_R30(x) x.r30
#define R_R31(x) x.r31
#define R_CR(x) x.cr /* condition registers */
#define R_XER(x) x.xer /* User integer exception register */
#define X_LR(x) x.lr /* link register */
#define X_CTR(x) x.ctr /* count register */
#define X_MQ(x) x.mq /* 601 only */
#define X_VRSAVE x.vrsave

/* registers offset */

#define R_EIP_OFF offsetof(regs_t, srr0)
#define R_EFLAGS_OFF offsetof(regs_t, srr1)
#define R_SRR0_OFF offsetof(regs_t, srr0)
#define R_SRR1_OFF offsetof(regs_t, srr1)
/* perl -e 'for $i(0..31){print "#define R_R".$i."_OFF offsetof(regs_t, r$i)\n";}' */
#define R_R0_OFF offsetof(regs_t, r0)
#define R_R1_OFF offsetof(regs_t, r1)
#define R_R2_OFF offsetof(regs_t, r2)
#define R_R3_OFF offsetof(regs_t, r3)
#define R_R4_OFF offsetof(regs_t, r4)
#define R_R5_OFF offsetof(regs_t, r5)
#define R_R6_OFF offsetof(regs_t, r6)
#define R_R7_OFF offsetof(regs_t, r7)
#define R_R8_OFF offsetof(regs_t, r8)
#define R_R9_OFF offsetof(regs_t, r9)
#define R_R10_OFF offsetof(regs_t, r10)
#define R_R11_OFF offsetof(regs_t, r11)
#define R_R12_OFF offsetof(regs_t, r12)
#define R_R13_OFF offsetof(regs_t, r13)
#define R_R14_OFF offsetof(regs_t, r14)
#define R_R15_OFF offsetof(regs_t, r15)
#define R_R16_OFF offsetof(regs_t, r16)
#define R_R17_OFF offsetof(regs_t, r17)
#define R_R18_OFF offsetof(regs_t, r18)
#define R_R19_OFF offsetof(regs_t, r19)
#define R_R20_OFF offsetof(regs_t, r20)
#define R_R21_OFF offsetof(regs_t, r21)
#define R_R22_OFF offsetof(regs_t, r22)
#define R_R23_OFF offsetof(regs_t, r23)
#define R_R24_OFF offsetof(regs_t, r24)
#define R_R25_OFF offsetof(regs_t, r25)
#define R_R26_OFF offsetof(regs_t, r26)
#define R_R27_OFF offsetof(regs_t, r27)
#define R_R28_OFF offsetof(regs_t, r28)
#define R_R29_OFF offsetof(regs_t, r29)
#define R_R30_OFF offsetof(regs_t, r30)
#define R_R31_OFF offsetof(regs_t, r31)
#define R_CR_OFF offsetof(regs_t, cr /* condition registers */
#define R_XER_OFF offsetof(regs_t, xer /* User integer exception register */
#define X_LR_OFF offsetof(regs_t, lr /* link register */
#define X_CTR_OFF offsetof(regs_t, ctr /* count register */
#define X_MQ_OFF offsetof(regs_t, mq /* 601 only */
#define X_VRSAVE_OFF offsetof(regs_t, vrsave)

#endif
