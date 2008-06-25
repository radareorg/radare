#ifndef I386_DEBUG_H
#define I386_DEBUG_H

#include "../../list.h"

/* stack frame */
struct sf_t {
	unsigned long ret_addr;	/* return address */
	unsigned long ebp;	/* return address */
	unsigned long sz;	/* size of stack frame */
	unsigned long vars_sz;	/* size of vars region */
	struct list_head	next;
};


int arch_bp_arm_hw(struct bp_t *bp);

/* Native-dependent code for the i386.

   Copyright (C) 2001, 2004, 2005 Free Software Foundation, Inc.

   THIS CODE IS PART OF GDB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details. 
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */ 


#if 0
#define Debu_NADDR        4       /* The number of debug address registers.  */
#endif
#define DR_STATUS       6       /* Index of debug status register (DR6).  */
#define DR_CONTROL      7       /* Index of debug control register (DR7). */

/* DR7 Debug Control register fields.  */
   
/* How many bits to skip in DR7 to get to R/W and LEN fields.  */
#define DR_CONTROL_SHIFT        16
/* How many bits in DR7 per R/W and LEN field for each watchpoint.  */
#define DR_CONTROL_SIZE         4
   
/* Watchpoint/breakpoint read/write fields in DR7.  */
#define DR_RW_EXECUTE   (0x0)   /* Break on instruction execution.  */
#define DR_RW_WRITE     (0x1)   /* Break on data writes.  */
#define DR_RW_READ      (0x3)   /* Break on data reads or writes.  */

/* This is here for completeness.  No platform supports this
   functionality yet (as of March 2001).  Note that the DE flag in the
   CR4 register needs to be set to support this.  */
#ifndef DR_RW_IORW
#define DR_RW_IORW      (0x2)   /* Break on I/O reads or writes.  */
#endif

/* Watchpoint/breakpoint length fields in DR7.  The 2-bit left shift
   is so we could OR this with the read/write field defined above.  */
#define DR_LEN_1        (0x0 << 2) /* 1-byte region watch or breakpoint.  */
#define DR_LEN_2        (0x1 << 2) /* 2-byte region watch.  */
#define DR_LEN_4        (0x3 << 2) /* 4-byte region watch.  */
#define DR_LEN_8        (0x2 << 2) /* 8-byte region watch (AMD64).  */
/* Debug registers' indices.  */
#define DR_NADDR        4       /* The number of debug address registers.  */
#define DR_STATUS       6       /* Index of debug status register (DR6).  */
#define DR_CONTROL      7       /* Index of debug control register (DR7). */

/* DR7 Debug Control register fields.  */
   
/* How many bits to skip in DR7 to get to R/W and LEN fields.  */
#define DR_CONTROL_SHIFT        16
/* How many bits in DR7 per R/W and LEN field for each watchpoint.  */
#define DR_CONTROL_SIZE         4
   
/* Watchpoint/breakpoint read/write fields in DR7.  */
#define DR_RW_EXECUTE   (0x0)   /* Break on instruction execution.  */
#define DR_RW_WRITE     (0x1)   /* Break on data writes.  */
#define DR_RW_READ      (0x3)   /* Break on data reads or writes.  */

/* This is here for completeness.  No platform supports this
   functionality yet (as of March 2001).  Note that the DE flag in the
   CR4 register needs to be set to support this.  */
#ifndef DR_RW_IORW
#define DR_RW_IORW      (0x2)   /* Break on I/O reads or writes.  */
#endif

/* Watchpoint/breakpoint length fields in DR7.  The 2-bit left shift
   is so we could OR this with the read/write field defined above.  */
#define DR_LEN_1        (0x0 << 2) /* 1-byte region watch or breakpoint.  */
#define DR_LEN_2        (0x1 << 2) /* 2-byte region watch.  */
#define DR_LEN_4        (0x3 << 2) /* 4-byte region watch.  */
#define DR_LEN_8        (0x2 << 2) /* 8-byte region watch (AMD64).  */

#define DR_NADDR        4       /* The number of debug address registers.  */
#define DR_STATUS       6       /* Index of debug status register (DR6).  */
#define DR_CONTROL      7       /* Index of debug control register (DR7). */

#if 0
#define DR7 Debu_NADDR        4       /* The number of debug address registers.  */
#endif
#define DR_STATUS       6       /* Index of debug status register (DR6).  */
#define DR_CONTROL      7       /* Index of debug control register (DR7). */

/* DR7  Control register fields.  */
   
/* How many bits to skip in DR7 to get to R/W and LEN fields.  */
#define DR_CONTROL_SHIFT        16
/* How many bits in DR7 per R/W and LEN field for each watchpoint.  */
#define DR_CONTROL_SIZE         4
   
/* Watchpoint/breakpoint read/write fields in DR7.  */
#define DR_RW_EXECUTE   (0x0)   /* Break on instruction execution.  */
#define DR_RW_WRITE     (0x1)   /* Break on data writes.  */
#define DR_RW_READ      (0x3)   /* Break on data reads or writes.  */

/* This is here for completeness.  No platform supports this
   functionality yet (as of March 2001).  Note that the DE flag in the
   CR4 register needs to be set to support this.  */
#ifndef DR_RW_IORW
#define DR_RW_IORW      (0x2)   /* Break on I/O reads or writes.  */
#endif

/* Watchpoint/breakpoint length fields in DR7.  The 2-bit left shift
   is so we could OR this with the read/write field defined above.  */
#define DR_LEN_1        (0x0 << 2) /* 1-byte region watch or breakpoint.  */
#define DR_LEN_2        (0x1 << 2) /* 2-byte region watch.  */
#define DR_LEN_4        (0x3 << 2) /* 4-byte region watch.  */
#define DR_LEN_8        (0x2 << 2) /* 8-byte region watch (AMD64).  */

   
/* How many bits to skip in DR7 to get to R/W and LEN fields.  */
#define DR_CONTROL_SHIFT        16
/* How many bits in DR7 per R/W and LEN field for each watchpoint.  */
#define DR_CONTROL_SIZE         4
   
/* Watchpoint/breakpoint read/write fields in DR7.  */
#define DR_RW_EXECUTE   (0x0)   /* Break on instruction execution.  */
#define DR_RW_WRITE     (0x1)   /* Break on data writes.  */
#define DR_RW_READ      (0x3)   /* Break on data reads or writes.  */

/* This is here for completeness.  No platform supports this
   functionality yet (as of March 2001).  Note that the DE flag in the
   CR4 register needs to be set to support this.  */
#ifndef DR_RW_IORW
#define DR_RW_IORW      (0x2)   /* Break on I/O reads or writes.  */
#endif

/* Watchpoint/breakpoint length fields in DR7.  The 2-bit left shift
   is so we could OR this with the read/write field defined above.  */
#define DR_LEN_1        (0x0 << 2) /* 1-byte region watch or breakpoint.  */
#define DR_LEN_2        (0x1 << 2) /* 2-byte region watch.  */
#define DR_LEN_4        (0x3 << 2) /* 4-byte region watch.  */
#define DR_LEN_8        (0x2 << 2) /* 8-byte region watch (AMD64).  */

#define I386_DR_CONTROL_MASK    (~DR_CONTROL_RESERVED)

/* The I'th debug register is vacant if its Local and Global Enable
   bits are reset in the  Control register.  */
#define I386_DR_VACANT(control, i) \
  ((control & (3 << (DR_ENABLE_SIZE * (i)))) == 0)

/* Locally enable the break/watchpoint in the I'th debug register.  */
#define I386_DR_LOCAL_ENABLE(control, i) \
  control |= (1 << (DR_LOCAL_ENABLE_SHIFT + DR_ENABLE_SIZE * (i)))

/* Globally enable the break/watchpoint in the I'th debug register.  */ 
#define I386_DR_GLOBAL_ENABLE(control, i) \
  control |= (1 << (DR_GLOBAL_ENABLE_SHIFT + 

/* Disable the break/watchpoint in the I'th debug register.  */
#define I386_DR_DISABLE(control, i) \
  control &= ~(3 << (DR_ENABLE_SIZE * (i)))

/* Enable the break/watchpoint in the I'th debug register.  */
#define I386_DR_ENABLE(control, i) \
  control |= (3 << (DR_ENABLE_SIZE * (i)))
   
/* Set in DR7 the RW and LEN fields for the I'th debug register.  */
#define I386_DR_SET_RW_LEN(control, i,rwlen) \
  do { \
    control &= ~(0x0f << (DR_CONTROL_SHIFT+DR_CONTROL_SIZE*(i)));   \
    control |= ((rwlen) << (DR_CONTROL_SHIFT+DR_CONTROL_SIZE*(i))); \
  } while (0)
   
/* Get from DR7 the RW and LEN fields for the I'th debug register.  */
#define I386_DR_GET_RW_LEN(control, i) \
  ((control >> (DR_CONTROL_SHIFT + DR_CONTROL_SIZE * (i))) & 0x0f)

/* Local and Global Enable flags in DR7.

   When the Local Enable flag is set, the breakpoint/watchpoint is
   enabled only for the current task; the processor automatically
   clears this flag on every task switch.  When the Global Enable flag
   is set, the breakpoint/watchpoint is enabled for all tasks; the
   processor never clears this flag. 
   
   Currently, all watchpoint are locally enabled.  If you need to
   enable them globally, read the comment which pertains to this in
   i386_insert_aligned_watchpoint below.  */
#define DR_LOCAL_ENABLE_SHIFT   0 /* Extra shift to the local enable bit.  */
#define DR_GLOBAL_ENABLE_SHIFT  1 /* Extra shift to the global enable bit.  */
#define DR_ENABLE_SIZE          2 /* Two enable bits per debug register.  */

/* Local and global exact breakpoint enable flags (a.k.a. slowdown
   flags).  These are only required on i386, to allow detection of the
   exact instruction which caused a watchpoint to break; i486 and
   later processors do that automatically.  We set these flags for
   backwards compatibility.  */
#define DR_LOCAL_SLOWDOWN       (0x100)
#define DR_GLOBAL_SLOWDOWN      (0x200)

/* Fields reserved by Intel.  This includes the GD (General Detect
   Enable) flag, which causes a debug exception to be generated when a
   MOV instruction accesses one of the debug registers.

   FIXME: My Intel manual says we should use 0xF800, not 0xFC00.  */
#define DR_CONTROL_RESERVED     (0xFC00)

//#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

#endif
