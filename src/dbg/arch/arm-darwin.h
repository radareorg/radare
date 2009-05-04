#ifndef _INCLUDE_PPC_DARWIN_H_
#define _INCLUDE_PPC_DARWIN_H_

#include <mach/arm/thread_state.h>
#include <mach/arm/thread_status.h>

#ifndef regs_t
#define regs_t arm_thread_state_t
#endif
//#define regs_sizeof sizeof(regs_t)/4

#define R_EIP(x) x.r15
#define R_EFLAGS(x) x.r0
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

/* registers offset */

#define R_EIP_OFF offsetof(regs_t, r15)
#define R_EFLAGS_OFF offsetof(regs_t, r0)

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

#endif
