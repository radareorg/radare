/*
** Copyright 2003,2004,2005,2006 by Todd Allen.  All Rights Reserved.
** Permission to use, copy, modify, distribute, and sell this software and
** its documentation for any purpose is hereby granted without fee, provided
** that the above copyright notice appear in all copies and that both the
** copyright notice and this permission notice appear in supporting
** documentation.
**
** No representations are made about the suitability of this software for any
** purpose.  It is provided ``as is'' without express or implied warranty,
** including but not limited to the warranties of merchantability, fitness
** for a particular purpose, and noninfringement.  In no event shall Todd
** Allen be liable for any claim, damages, or other liability, whether in
** action of contract, tort, or otherwise, arising from, out of, or in
** connection with this software.
*/
#if 0

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/major.h>
#define _GNU_SOURCE
#include <getopt.h>

typedef int   boolean;
#define TRUE  1
#define FALSE 0

typedef char*              string;
typedef const char*        cstring;
typedef const char* const  ccstring;
#define SAME  0

#define STR(x)   #x
#define XSTR(x)  STR(x)

#define MAX(l,r)            ((l) > (r) ? (l) : (r))

#define LENGTH(array, type) (sizeof(array) / sizeof(type))

#define BPI  32
#define POWER2(power) \
   (1 << (power))
#define RIGHTMASK(width) \
   (((width) >= BPI) ? ~0 : POWER2(width)-1)
#define BIT_EXTRACT_LE(value, start, after) \
   (((value) & RIGHTMASK(after)) >> start)

const char*  program = NULL;

typedef struct {
   ccstring      name;
   unsigned int  low_bit;
   unsigned int  high_bit;
   ccstring*     images;
} named_item;

#define NIL_IMAGES  (ccstring*)NULL

static unsigned int
get_max_len (named_item    names[],
             unsigned int  length)
{
   unsigned int  result = 0;
   unsigned int  i;

   for (i = 0; i < length; i++) {
      result = MAX(result, strlen(names[i].name));
   }

   return result;
}

static void
print_names(unsigned int  value,
            named_item    names[],
            unsigned int  length,
            unsigned int  max_len)
{
   unsigned int  i;

   if (max_len == 0) {
      max_len = get_max_len(names, length);
   }

   for (i = 0; i < length; i++) {
      unsigned int  field = BIT_EXTRACT_LE(value,
                                           names[i].low_bit,
                                           names[i].high_bit + 1);
      if (names[i].images != NIL_IMAGES
          && names[i].images[field] != NULL) {
         printf("      %-*s = %s\n",
                max_len,
                names[i].name,
                names[i].images[field]);
      } else {
         printf("      %-*s = 0x%0x (%u)\n",
                max_len,
                names[i].name,
                field,
                field);
      }
   }
}

static ccstring  bools[] = { "false",
                             "true" };

typedef enum {
   VENDOR_UNKNOWN,
   VENDOR_INTEL,
   VENDOR_AMD,
   VENDOR_CYRIX,
   VENDOR_VIA,
   VENDOR_TRANSMETA,
   VENDOR_UMC,
   VENDOR_NEXGEN,
   VENDOR_RISE
} vendor_t;

#define __F(v)     ((v) & 0x00000f00)
#define __FM(v)    ((v) & 0x00000ff0)
#define __FMS(v)   ((v) & 0x00000fff)
#define __XF(v)    ((v) & 0x0ff00f00)
#define __XFM(v)   ((v) & 0x0ff00ff0)
#define __XFMS(v)  ((v) & 0x0ff00fff)
#define __XFXM(v)  ((v) & 0x0fff0ff0)
#define __XFXMS(v) ((v) & 0x0fff0fff)

#define __TF(v)    ((v) & 0x00003f00)
#define __TFM(v)   ((v) & 0x00003ff0)
#define __TFMS(v)  ((v) & 0x00003fff)
#define __TXF(v)   ((v) & 0x0ff03f00)
#define __TXFM(v)  ((v) & 0x0ff03ff0)
#define __TXFMS(v) ((v) & 0x0ff03fff)

#define _T(v)      ((v) << 12)
#define _F(v)      ((v) << 8)
#define _M(v)      ((v) << 4)
#define _S(v)      (v)
#define _XF(v)     ((v) << 20)
#define _XM(v)     ((v) << 16)

#define __B(v)     ((v) & 0x000000ff)
#define _B(v)      (v)

#define __XB(v)    ((v) & 0x00000fff)
#define _XB(v)     (v)

#define START \
   if (0)
#define F(f,str) \
   else if (__F(val)     ==                       _F(f)                    ) printf(str)
#define FM(f,m,str) \
   else if (__FM(val)    ==                       _F(f) +_M(m)             ) printf(str)
#define FMS(f,m,s,str) \
   else if (__FMS(val)   ==                       _F(f) +_M(m)+_S(s)       ) printf(str)
#define XF(xf,str) \
   else if (__XF(val)    ==       _XF(xf)        +_F(15)                   ) printf(str)
#define XFM(xf,m,str) \
   else if (__XFM(val)   ==       _XF(xf)        +_F(15)+_M(m)             ) printf(str)
#define XFMS(xf,m,s,str) \
   else if (__XFMS(val)  ==       _XF(xf)        +_F(15)+_M(m)+_S(s)       ) printf(str)
#define XFXM(xf,xm,m,str) \
   else if (__XFXM(val)  ==       _XF(xf)+_XM(xm)+_F(15)+_M(m)             ) printf(str)
#define XFXMS(xf,xm,m,s,str) \
   else if (__XFXMS(val) ==       _XF(xf)+_XM(xm)+_F(15)+_M(m)+_S(s)       ) printf(str)
#define TF(t,f,str) \
   else if (__TF(val)    == _T(t)                +_F(f)                    ) printf(str)
#define TFM(t,f,m,str) \
   else if (__TFM(val)   == _T(t)                +_F(f) +_M(m)             ) printf(str)
#define TFMS(t,f,m,s,str) \
   else if (__TFMS(val)  == _T(t)                +_F(f) +_M(m)+_S(s)       ) printf(str)
#define TXF(t,xf,str) \
   else if (__TXF(val)   == _T(t)+_XF(xf)        +_F(15)                   ) printf(str)
#define TXFM(t,xf,m,str) \
   else if (__TXFM(val)  == _T(t)+_XF(xf)        +_F(15)+_M(m)             ) printf(str)
#define TXFMS(t,xf,m,s,str) \
   else if (__TXFMS(val) == _T(t)+_XF(xf)        +_F(15)+_M(m)+_S(s)       ) printf(str)
#define TXFXM(t,xf,xm,m,str) \
   else if (__TXFXM(val)  == _T(t)+_XF(xf)+_XM(xm)+_F(15)+_M(m)            ) printf(str)
#define TXFXMS(t,xf,xm,m,s,str) \
   else if (__TXFXMS(val) == _T(t)+_XF(xf)+_XM(xm)+_F(15)+_M(m)+_S(s)      ) printf(str)
#define FC(f,m,c,str) \
   else if (__F(val)    ==                 _F(f)               && cd == (c)) printf(str)
#define FMC(f,m,c,str) \
   else if (__FM(val)   ==                 _F(f) +_M(m)        && cd == (c)) printf(str)
#define FMSC(f,m,s,c,str) \
   else if (__FMS(val)  ==                 _F(f) +_M(m)+_S(s)  && cd == (c)) printf(str)
#define XFC(xf,c,str) \
   else if (__XF(val)   == _XF(xf)        +_F(15)              && cd == (c)) printf(str)
#define XFMC(xf,m,c,str) \
   else if (__XFM(val)  == _XF(xf)        +_F(15)+_M(m)        && cd == (c)) printf(str)
#define XFMSC(xf,m,s,c,str) \
   else if (__XFMS(val) == _XF(xf)        +_F(15)+_M(m)+_S(s)  && cd == (c)) printf(str)
#define XFXMC(xf,xm,m,c,str) \
   else if (__XFXM(val)  == _XF(xf)+_XM(xm)+_F(15)+_M(m)       && cd == (c)) printf(str)
#define XFXMSC(xf,xm,m,s,c,str) \
   else if (__XFXMS(val) == _XF(xf)+_XM(xm)+_F(15)+_M(m)+_S(s) && cd == (c)) printf(str)
#define DEFAULT(str) \
   else                                                                      printf(str)

/*
** d? = think "desktop"
** s? = think "server" (multiprocessor)
** M? = think "mobile"
** D? = think "dual core"
**
** ?P = think Pentium
** ?X = think Xeon
** ?M = think Xeon MP
** ?C = think Celeron
** ?c = think Core
** ?A = think Athlon
** ?D = think Duron
** ?S = think Sempron
** ?O = think Opteron
** ?T = think Turion
*/
typedef enum {
       /* ==========Intel==========  ============AMD============    Transmeta */
   UN, /*        Unknown                                                      */
   dP, /*        Pentium                                                      */
   MP, /* Mobile Pentium                                                      */
   sX, /*        Xeon                                                         */
   sI, /*        Xeon (Irwindale)                                             */
   s7, /*        Xeon (Series 7000)                                           */
   sM, /*        Xeon MP                                                      */
   sP, /*        Xeon MP (Potomac)                                            */
   MM, /* Mobile Pentium M                                                    */
   dC, /*        Celeron                                                      */
   MC, /* Mobile Celeron                                                      */
   nC, /* not    Celeron                                                      */
   dc, /*        CPU (often Core Solo)                                        */
   Dc, /*        Core Duo                                                     */
   Da, /*        Core Duo (Allendale)                                         */
   dO, /*                                      Opteron                        */
   d8, /*                                      Opteron (8xx)                  */
   dX, /*                                      Athlon XP                      */
   dt, /*                                      Athlon XP (Thorton)            */
   MX, /*                            mobile    Athlon XP-M                    */
   ML, /*                            mobile    Athlon XP-M (LV)               */
   dA, /*                                      Athlon(64)                     */
   dm, /*                                      Athlon(64) (Manchester)        */
   sA, /*                                      Athlon MP                      */
   MA, /*                            mobile    Athlon(64)                     */
   dF, /*                                      Athlon 64 FX                   */
   dD, /*        Pentium D                     Duron                          */
   sD, /*                                      Duron MP                       */
   MD, /*                            mobile    Duron                          */
   dS, /*                                      Sempron                        */
   MS, /*                            mobile    Sempron                        */
   DO, /*                            Dual Core Opteron                        */
   D8, /*                            Dual Core Opteron (8xx)                  */
   MT, /*                            mobile    Turion                         */
   t2, /*                                                           TMx200    */
   t4, /*                                                           TMx400    */
   t5, /*                                                           TMx500    */
   t6, /*                                                           TMx600    */
   t8, /*                                                           TMx800    */
} code_t;

typedef struct {
   vendor_t       vendor;
   unsigned int   val_1_eax;
   unsigned int   val_1_ebx;
   unsigned int   val_1_ecx;
   unsigned int   val_1_edx;
   unsigned int   val_4_eax;
   unsigned int   val_80000001_ebx;
   unsigned int   val_80000001_ecx;
   unsigned int   val_80000008_ecx;
   unsigned int   transmeta_proc_rev;
   char           brand[48];
   char           transmeta_info[48];

   struct mp {
      boolean        ok;
      unsigned int   cores;
      unsigned int   hyperthreads;
   } mp;

                                /* ==============implications============== */
                                /* PII (F6, M5)            PIII (F6, M7)    */
                                /* ----------------------  ---------------  */
   boolean       L2_4w_1Mor2M;  /* Xeon                    Xeon             */
   boolean       L2_4w_512K;    /* normal, Mobile or Xeon  normal or Xeon   */
   boolean       L2_4w_256K;    /* Mobile                   -               */
   boolean       L2_8w_1Mor2M;  /*  -                      Xeon             */
   boolean       L2_8w_512K;    /*  -                      normal           */
   boolean       L2_8w_256K;    /*  -                      normal or Xeon   */
                 /* none */     /* Celeron                  -               */

   boolean       L2_2M;         /* Nocona lacks, Irwindale has */
                                /* Conroe has more, Allendale has this */
   boolean       L3;            /* Cranford lacks, Potomac has */

   boolean       L2_256K;       /* Barton has more, Thorton has this */
   boolean       L2_512K;       /* Toledo has more, Manchester E6 has this */
} code_stash_t;

#define NIL_STASH { VENDOR_UNKNOWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", "",  \
                    { FALSE, 0, 0 }, \
                    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, \
                    FALSE, FALSE, \
                    FALSE, FALSE }

static void
stash_intel_cache(code_stash_t*  stash,
                  unsigned char  value)
{
   switch (value) {
   case 0x42: stash->L2_4w_256K   = TRUE; break;
   case 0x43: stash->L2_4w_512K   = TRUE; break;
   case 0x44: stash->L2_4w_1Mor2M = TRUE; break;
   case 0x45: stash->L2_4w_1Mor2M = TRUE; break;
   case 0x82: stash->L2_8w_256K   = TRUE; break;
   case 0x83: stash->L2_8w_512K   = TRUE; break;
   case 0x84: stash->L2_8w_1Mor2M = TRUE; break;
   case 0x85: stash->L2_8w_1Mor2M = TRUE; break;
   }

   switch (value) {
   case 0x45:
   case 0x85:
   case 0x88: 
      stash->L2_2M = TRUE; 
      break;
   }

   switch (value) {
   case 0x22:
   case 0x23:
   case 0x25:
   case 0x29:
   case 0x46:
   case 0x47:
   case 0x49:
   case 0x4a:
   case 0x4b:
   case 0x4c:
   case 0x4d:
   case 0x88:
   case 0x89:
   case 0x8a:
   case 0x8d:
      stash->L3 = TRUE;
      break;
   }

   switch (value) {
   case 0x3c:
   case 0x42:
   case 0x7a:
   case 0x7e:
   case 0x82:
      stash->L2_256K = TRUE;
      break;
   }

   switch (value) {
   case 0x3e:
   case 0x43:
   case 0x7b:
   case 0x7f:
   case 0x83:
   case 0x86:
      stash->L2_512K = TRUE;
      break;
   }
}

static code_t
decode_transmeta_rev_cache (const code_stash_t*  stash)
{
   /* TODO: Add codes for Transmeta Crusoe TM5700/TM5900 */
   /* TODO: Add codes for Transmeta Efficeon */
   switch (stash->transmeta_proc_rev & 0xffff0000) {
   case 0x01010000: 
      return t2;
   case 0x01020000: 
      return t4;
   case 0x01030000: 
      if ((stash->transmeta_proc_rev & 0xffffff00) == 0x00000000) {
         if (stash->L2_4w_256K) {
            return t4;
         } else if (stash->L2_4w_512K) {
            return t6;
         } else {
            return UN;
         }
      } else {
         return UN;
      }
   case 0x01040000: 
   case 0x01050000: 
      if (stash->L2_4w_256K) {
         return t5;
      } else if (stash->L2_4w_512K) {
         return t8;
      } else {
         return UN;
      }
   default:
      return UN;
   }
}

static code_t
decode_rev_cache (const code_stash_t*  stash)
{
   switch (stash->vendor) {
   case VENDOR_INTEL:
      switch (__FM(stash->val_1_eax)) {
      case _F(6) + _M(5):
         if (stash->L2_4w_1Mor2M) {
            return sX;
         } else if (stash->L2_4w_512K) {
            return nC;
         } else if (stash->L2_4w_256K) {
            return MP;
         } else {
            return dC;
         }
      case _F(6) + _M(7):
         if (stash->L2_4w_1Mor2M || stash->L2_8w_1Mor2M) {
            return sX;
         } else if (stash->L2_8w_512K) {
            return dP;
         } else {
            return UN;
         }
      default:
         return UN;
      }
   case VENDOR_TRANSMETA:
      switch (__FM(stash->val_1_eax)) {
      case _F(5) + _M(4):
         return decode_transmeta_rev_cache(stash);
      default:
         return UN;
      }
   default:
      return UN;
   }
}

static code_t
decode_brand(const code_stash_t*  stash)
{
   const char*  brand = stash->brand;

   switch (stash->vendor) {
   case VENDOR_INTEL:
      if (strstr(brand, "Mobile") != NULL) {
         if (strstr(brand, "Celeron") != NULL) {
            return MC;
         } else if (strstr(brand, "Pentium") != NULL) {
            return MP;
         }
      } else {
         if (strstr(brand, "Xeon MP") != NULL
             || strstr(brand, "Xeon(TM) MP") != NULL) {
            return sM;
         } else if (strstr(brand, "Xeon") != NULL) {
            return sX;
         } else if (strstr(brand, "Celeron") != NULL) {
            return dC;
         } else if (strstr(brand, "Pentium(R) M") != NULL) {
            return MM;
         } else if (strstr(brand, "Pentium(R) D") != NULL) {
            return dD;
         } else if (strstr(brand, "Pentium") != NULL) {
            return dP;
         } else if (strstr(brand, "Genuine Intel(R) CPU") != NULL
                    || strstr(brand, "Intel(R) Core(TM)2 CPU") != NULL) {
            return dc;
         }
      }
      return UN;
   case VENDOR_AMD:
      if (strstr(brand, "mobile") != NULL) {
         if (strstr(brand, "Athlon(tm) XP-M (LV)") != NULL) {
            return ML;
         } else if (strstr(brand, "Athlon(tm) XP-M") != NULL) {
            return MX;
         } else if (strstr(brand, "Duron") != NULL) {
            return MD;
         } else if (strstr(brand, "Athlon") != NULL) {
            return MA;
         }
      } else if (strstr(brand, "Mobile") != NULL) {
         if (strstr(brand, "Athlon(tm) XP") != NULL) {
            return MX;
         } else if (strstr(brand, "Athlon(tm) 64") != NULL) {
            return MA;
         } else if (strstr(brand, "Sempron") != NULL) {
            return MS;
         }
      } else {
         if (strstr(brand, "Dual Core") != NULL) {
            return DO;
         } else if (strstr(brand, "Athlon(tm) XP") != NULL
                    || strstr(brand, "Athlon(TM) XP") != NULL) {
            return dX;
         } else if (strstr(brand, "Athlon(tm) 64 FX") != NULL) {
            return dF;
         } else if (strstr(brand, "Athlon(tm) MP") != NULL) {
            return sA;
         } else if (strstr(brand, "Duron(tm) MP") != NULL) {
            return sD;
         } else if (strstr(brand, "Duron") != NULL) {
            return dD;
         } else if (strstr(brand, "Athlon") != NULL) {
            return dA;
         } else if (strstr(brand, "Sempron") != NULL) {
            return dS;
         } else if (strstr(brand, "Opteron") != NULL) {
            return dO;
         } else if (strstr(brand, "Turion") != NULL) {
            return MT;
         }
      }
      return UN;
   default:
      return UN;
   }
}

static code_t
decode_brand_code(const code_stash_t*  stash)
{
   unsigned int  val_1_eax = stash->val_1_eax;
   unsigned int  val_1_ebx = stash->val_1_ebx;

   switch (__B(val_1_ebx)) {
   case _B(0):  return UN;
   case _B(1):  return dC;
   case _B(2):  return dP;
   case _B(3):  return __FMS(val_1_eax)  == _F(6) + _M(11) + _S(1) ? dC : sX;
   case _B(4):  return dP;
   case _B(6):  return dP;
   case _B(7):  return dC;
   case _B(8):  return dP;
   case _B(9):  return dP;
   case _B(10): return dC;
   case _B(11): return __XFMS(val_1_eax) <= _XF(0) + _M(1) + _S(2) ? sM : sX;
   case _B(12): return sM;
   case _B(14): return __XFMS(val_1_eax) <= _XF(0) + _M(1) + _S(3) ? sX : MM;
   case _B(15): return __XFM(val_1_eax)  == _XF(0) + _M(2)         ? MM : MC;
   case _B(16): return dC;
   case _B(17): return MP;
   case _B(18): return dC;
   case _B(19): return MC;
   case _B(20): return dC;
   case _B(21): return MP;
   case _B(22): return dP;
   case _B(23): return MC;
   default:     return UN;
   }
}

static code_t
specialize_intel_mp(code_t               result,
                    const code_stash_t*  stash)
{
   if (stash->vendor == VENDOR_INTEL) {
      switch (result) {
      case dc:
         if (stash->mp.cores == 4) {
            // This can happen for pre-production Woodcrest cores
            result = sX;
         } else if (stash->mp.cores == 2) {
            result = Dc;
         }
         break;
      default:
         /* DO NOTHING */
         break;
      }
   }

   return result;
}

static code_t
specialize_intel_cache(code_t               result,
                       const code_stash_t*  stash)
{
   if (stash->vendor == VENDOR_INTEL) {
      switch (result) {
      case sX:
         switch (__XFMS(stash->val_1_eax)) {
         case _XF(0) + _F(15) + _M(4) + _S(3):
            if (stash->L3) {
               /* to distinguish Irwindale from Nocona */
               result = sI;
            }
         }
         break;
      case sM:
         switch (__XFMS(stash->val_1_eax)) {
         case _XF(0) + _F(15) + _M(4) + _S(1):
            if (stash->L3) {
               /* to distinguish Potomac from Cranford */
               result = sP;
            }
         }
         break;
      case Dc:
         switch (__FMS(stash->val_1_eax)) {
         case _F(6) + _M(15) + _S(5):
         case _F(6) + _M(15) + _S(6):
            if (stash->L2_2M) {
               result = Da;
            }
         }
         break;
      default:
         /* DO NOTHING */
         break;
      }
   }

   return result;
}

#define IS_VMX(val_1_ecx)  (BIT_EXTRACT_LE((val_1_ecx), 5, 6))

static code_t
specialize_intel_VT(code_t               result,
                    const code_stash_t*  stash)
{
   if (stash->vendor == VENDOR_INTEL) {
      switch (result) {
      case sX:
         switch (__XFMS(stash->val_1_eax)) {
         case _XF(0) + _F(15) + _M(4) + _S(8):
            /*
            ** Is this the best way to distinguish these two processors?
            **    Dual-Core Xeon (Paxville A0)
            **    Dual-Core Xeon Processor 7000 (Paxville A0)
            ** Empirically, the significant differences are the VMX flag and
            ** the "execution disable" flag.  The VMX flag is in an
            ** Intel-defined CPUID function, so it's used.
            */
            if (IS_VMX(stash->val_1_ecx)) {
               /* to distinguish Paxville Series 7000 from Paxville 2.80 GHz */
               result = s7;
            }
         }
         break;
      default:
         /* DO NOTHING */
         break;
      }
   }

   return result;
}

static code_t
specialize_amd_model(code_t               result,
                     const code_stash_t*  stash)
{
   if (stash->vendor == VENDOR_AMD) {
      switch (result) {
      case dO:
      case DO:
         switch (__XFXM(stash->val_1_eax)) {
         case _XF(0) + _F(15) + _XM(2) + _M(1): /* Italy/Egypt */
         case _XF(0) + _F(15) + _XM(2) + _M(5): /* Troy/Athens */
            {
               unsigned int  msb;

               if (__XB(stash->val_80000001_ebx) != 0) {
                  msb = BIT_EXTRACT_LE(__XB(stash->val_80000001_ebx), 6, 12);
               } else if (_B(stash->val_1_ebx) != 0) {
                  /* 
                  ** Nothing explains how the msb of this brand should be
                  ** interpreted.  I concluded it must be shifted left by 2
                  ** with a little reverse engineering.  That's the only way
                  ** it gets the right answer for the machine aero, which has
                  ** brand 0x83, and is known to be a 244.
                  */
                  msb = BIT_EXTRACT_LE(__B(stash->val_1_ebx), 5, 8) << 2;
               } else {
                  return result;
               }

               switch (msb) {
               case 0x10:
               case 0x11:
               case 0x12:
               case 0x13:
               case 0x2a:
               case 0x30:
               case 0x31:
               case 0x39:
               case 0x32:
               case 0x33:
                  /* It's a 2xx */
                  /* DO NOTHING */
                  break;
               case 0x14:
               case 0x15:
               case 0x16:
               case 0x17:
               case 0x2b:
               case 0x34:
               case 0x35:
               case 0x3a:
               case 0x36:
               case 0x37:
                  /* It's an 8xx */
                  switch (result) {
                  case dO:
                     result = d8; /* to distinguish Athens from Troy */
                     break;
                  case DO:
                     result = D8; /* to distinguish Egypt from Italy */
                     break;
                  default:
                     /* Shouldn't happen */
                     break;
                  }
               }
            }
         }
         break;
      default:
         /* DO NOTHING */
         break;
      }
   }

   return result;
}

static code_t
specialize_amd_cache(code_t               result,
                     const code_stash_t*  stash)
{
   if (stash->vendor == VENDOR_AMD) {
      switch (result) {
      case dX:
         switch (__FMS(stash->val_1_eax)) {
         case _F(6) + _M(10) + _S(0):
            if (stash->L2_256K) {
               /* to distinguish Thorton from Barton */
               result = dt;
            }
         }
         break;
      case dA:
         switch (__XFXM(stash->val_1_eax)) {
         case _XF(0) + _F(15) + _XM(2) + _M(3):
            if (stash->L2_512K) {
               /* to distinguish Manchester E6 from Toledo */
               result = dm;
            }
         }
         break;
      default:
         /* DO NOTHING */
         break;
      }
   }

   return result;
}

static code_t
decode(const code_stash_t*  stash)
{
   code_t  result;

   result = decode_brand_code(stash);
   if (result == UN) {
      result = decode_brand(stash);
   }
   if (result == UN) {
      result = decode_rev_cache(stash);
   }

   result = specialize_intel_mp(result, stash);
   result = specialize_intel_cache(result, stash);
   result = specialize_intel_VT(result, stash);
   result = specialize_amd_model(result, stash);
   result = specialize_amd_cache(result, stash);

   return result;
}

static void
print_synth_intel(const char*   name,
                  unsigned int  val,  /* val_1_eax */
                  code_t        cd)
{
   printf(name);
   START;
   FM   (   4,  0,         "Intel i80486DX-25/33");
   FM   (   4,  1,         "Intel i80486DX-50");
   FM   (   4,  2,         "Intel i80486SX");
   FM   (   4,  3,         "Intel i80486DX/2");
   FM   (   4,  4,         "Intel i80486SL");
   FM   (   4,  5,         "Intel i80486SX/2");
   FM   (   4,  7,         "Intel i80486DX/2-WB");
   FM   (   4,  8,         "Intel i80486DX/4");
   FM   (   4,  9,         "Intel i80486DX/4-WB");
   F    (   4,             "Intel i80486 (unknown model)");
   FM   (   5,  0,         "Intel Pentium 60/66 A-step");
   TFM  (1, 5,  1,         "Intel Pentium 60/66 OverDrive for P5");
   FMS  (   5,  1,  3,     "Intel Pentium 60/66 (B1)");
   FMS  (   5,  1,  5,     "Intel Pentium 60/66 (C1)");
   FMS  (   5,  1,  7,     "Intel Pentium 60/66 (D1)");
   FM   (   5,  1,         "Intel Pentium 60/66");
   TFM  (1, 5,  2,         "Intel Pentium 75 - 200 OverDrive for P54C");
   FMS  (   5,  2,  1,     "Intel Pentium P54C 75 - 200 (B1)");
   FMS  (   5,  2,  2,     "Intel Pentium P54C 75 - 200 (B3)");
   FMS  (   5,  2,  4,     "Intel Pentium P54C 75 - 200 (B5)");
   FMS  (   5,  2,  5,     "Intel Pentium P54C 75 - 200 (C2/mA1)");
   FMS  (   5,  2,  6,     "Intel Pentium P54C 75 - 200 (E0)");
   FMS  (   5,  2, 11,     "Intel Pentium P54C 75 - 200 (cB1)");
   FMS  (   5,  2, 12,     "Intel Pentium P54C 75 - 200 (cC0)");
   FM   (   5,  2,         "Intel Pentium P54C 75 - 200");
   TFM  (1, 5,  3,         "Intel Pentium OverDrive for i486 (P24T)");
   TFM  (1, 5,  4,         "Intel Pentium OverDrive for P54C");
   FMS  (   5,  4,  3,     "Intel Pentium MMX P55C (B1)");
   FMS  (   5,  4,  4,     "Intel Pentium MMX P55C (A3)");
   FM   (   5,  4,         "Intel Pentium MMX P55C");
   FMS  (   5,  7,  0,     "Intel Pentium MMX P54C 75 - 200 (A4)");
   FM   (   5,  7,         "Intel Pentium MMX P54C 75 - 200");
   FMS  (   5,  8,  1,     "Intel Pentium MMX P55C (A0), .25um");
   FMS  (   5,  8,  2,     "Intel Pentium MMX P55C (B2), .25um");
   FM   (   5,  8,         "Intel Pentium MMX P55C, .25um");
   F    (   5,             "Intel Pentium (unknown model)");
   FM   (   6,  0,         "Intel Pentium Pro A-step");
   FMS  (   6,  1,  1,     "Intel Pentium Pro (B0)");
   FMS  (   6,  1,  2,     "Intel Pentium Pro (C0)");
   FMS  (   6,  1,  6,     "Intel Pentium Pro (sA0)");
   FMS  (   6,  1,  7,     "Intel Pentium Pro (sA1)");
   FMS  (   6,  1,  9,     "Intel Pentium Pro (sB1)");
   FM   (   6,  1,         "Intel Pentium Pro");
   TFM  (1, 6,  3,         "Intel Pentium II OverDrive");
   FMS  (   6,  3,  3,     "Intel Pentium II (Klamath C0), .28um");
   FMS  (   6,  3,  4,     "Intel Pentium II (Klamath C1), .28um");
   FM   (   6,  3,         "Intel Pentium II (Klamath), .28um");
   FM   (   6,  4,         "Intel Pentium P55CT OverDrive (Deschutes)");
   FMSC (   6,  5,  0, MP, "Intel Mobile Pentium II (Deschutes A0), .25um");
   FMSC (   6,  5,  0, sX, "Intel Pentium II Xeon (Deschutes A0), .25um");
   FMSC (   6,  5,  0, dC, "Intel Celeron (Deschutes A0), .25um");
   FMSC (   6,  5,  0, nC, "Intel Pentium II / Pentium II Xeon / Mobile Pentium II (Deschutes A0), .25um");
   FMS  (   6,  5,  0,     "Intel Pentium II / Pentium II Xeon / Celeron / Mobile Pentium II (Deschutes A0), .25um");
   FMSC (   6,  5,  1, sX, "Intel Pentium II Xeon (Deschutes A1), .25um");
   FMSC (   6,  5,  1, dC, "Intel Celeron (Deschutes A1), .25um");
   FMSC (   6,  5,  1, nC, "Intel Pentium II / Pentium II Xeon (Deschutes A1), .25um");
   FMS  (   6,  5,  1,     "Intel Pentium II / Pentium II Xeon / Celeron (Deschutes A1), .25um");
   FMSC (   6,  5,  2, MP, "Intel Mobile Pentium II (Deschutes B0), .25um");
   FMSC (   6,  5,  2, sX, "Intel Pentium II Xeon (Deschutes B0), .25um");
   FMSC (   6,  5,  2, dC, "Intel Celeron (Deschutes B0), .25um");
   FMSC (   6,  5,  2, nC, "Intel Pentium II / Pentium II Xeon / Mobile Pentium II (Deschutes B0), .25um");
   FMS  (   6,  5,  2,     "Intel Pentium II / Pentium II Xeon / Celeron / Mobile Pentium II (Deschutes B0), .25um");
   FMSC (   6,  5,  3, sX, "Intel Pentium II Xeon (Deschutes B1), .25um");
   FMSC (   6,  5,  3, dC, "Intel Celeron (Deschutes B1), .25um");
   FMSC (   6,  5,  3, nC, "Intel Pentium II / Pentium II Xeon (Deschutes B1), .25um");
   FMS  (   6,  5,  3,     "Intel Pentium II / Pentium II Xeon / Celeron (Deschutes B1), .25um");
   FMC  (   6,  5,     MP, "Intel Mobile Pentium II (Deschutes), .25um");
   FMC  (   6,  5,     sX, "Intel Pentium II Xeon (Deschutes), .25um");
   FMC  (   6,  5,     dC, "Intel Celeron (Deschutes), .25um");
   FMC  (   6,  5,     nC, "Intel Pentium II / Pentium II Xeon / Mobile Pentium II (Deschutes), .25um");
   FM   (   6,  5,         "Intel Pentium II / Pentium II Xeon / Celeron / Mobile Pentium II (Deschutes), .25um");
   FMSC (   6,  6,  0, dP, "Intel Pentium II (Mendocino A0), L2 cache");
   FMSC (   6,  6,  0, dC, "Intel Celeron (Mendocino A0), L2 cache");
   FMS  (   6,  6,  0,     "Intel Pentium II (Mendocino A0) / Celeron (Mendocino A0), L2 cache");
   FMSC (   6,  6,  5, dP, "Intel Pentium II (Mendocino B0), L2 cache");
   FMSC (   6,  6,  5, dC, "Intel Celeron (Mendocino B0), L2 cache");
   FMS  (   6,  6,  5,     "Intel Pentium II (Mendocino B0) / Celeron (Mendocino B0), L2 cache");
   FMS  (   6,  6, 10,     "Intel Mobile Pentium II (Mendocino A0), L2 cache");
   FM   (   6,  6,         "Intel Pentium II (Mendocino), L2 cache");
   FMSC (   6,  7,  2, dP, "Intel Pentium III (Katmai B0), .25um");
   FMSC (   6,  7,  2, sX, "Intel Pentium III Xeon (Katmai B0), .25um");
   FMS  (   6,  7,  2,     "Intel Pentium III / Pentium III Xeon (Katmai B0), .25um");
   FMSC (   6,  7,  3, dP, "Intel Pentium III (Katmai C0), .25um");
   FMSC (   6,  7,  3, sX, "Intel Pentium III Xeon (Katmai C0), .25um");
   FMS  (   6,  7,  3,     "Intel Pentium III / Pentium III Xeon (Katmai C0), .25um");
   FMC  (   6,  7,     dP, "Intel Pentium III (Katmai), .25um");
   FMC  (   6,  7,     sX, "Intel Pentium III Xeon (Katmai), .25um");
   FM   (   6,  7,         "Intel Pentium III / Pentium III Xeon (Katmai), .25um");
   FMSC (   6,  8,  1, dP, "Intel Pentium III (Coppermine A2), .18um");
   FMSC (   6,  8,  1, MP, "Intel Mobile Pentium III (Coppermine A2), .18um");
   FMSC (   6,  8,  1, sX, "Intel Pentium III Xeon (Coppermine A2), .18um");
   FMSC (   6,  8,  1, dC, "Intel Celeron (Coppermine A2), .18um");
   FMSC (   6,  8,  1, MC, "Intel Mobile Celeron (Coppermine A2), .18um");
   FMS  (   6,  8,  1,     "Intel Pentium III / Pentium III Xeon / Celeron / Mobile Pentium III (Coppermine A2) / Mobile Celeron (Coppermine A2), .18um");
   FMSC (   6,  8,  3, dP, "Intel Pentium III (Coppermine B0), .18um");
   FMSC (   6,  8,  3, MP, "Intel Mobile Pentium III (Coppermine B0), .18um");
   FMSC (   6,  8,  3, sX, "Intel Pentium III Xeon (Coppermine B0), .18um");
   FMSC (   6,  8,  3, dC, "Intel Celeron (Coppermine B0), .18um");
   FMSC (   6,  8,  3, MC, "Intel Mobile Celeron (Coppermine B0), .18um");
   FMS  (   6,  8,  3,     "Intel Pentium III / Pentium III Xeon / Celeron / Mobile Pentium III (Coppermine B0) / Mobile Celeron (Coppermine B0), .18um");
   FMSC (   6,  8,  6, dP, "Intel Pentium III (Coppermine C0), .18um");
   FMSC (   6,  8,  6, MP, "Intel Mobile Pentium III (Coppermine C0), .18um");
   FMSC (   6,  8,  6, sX, "Intel Pentium III Xeon (Coppermine C0), .18um");
   FMSC (   6,  8,  6, dC, "Intel Celeron (Coppermine C0), .18um");
   FMSC (   6,  8,  6, MC, "Intel Mobile Celeron (Coppermine C0), .18um");
   FMS  (   6,  8,  6,     "Intel Pentium III / Pentium III Xeon / Celeron / Mobile Pentium III (Coppermine C0) / Mobile Celeron (Coppermine C0), .18um");
   FMSC (   6,  8, 10, dP, "Intel Pentium III (Coppermine D0), .18um");
   FMSC (   6,  8, 10, MP, "Intel Mobile Pentium III (Coppermine D0), .18um");
   FMSC (   6,  8, 10, sX, "Intel Pentium III Xeon (Coppermine D0), .18um");
   FMSC (   6,  8, 10, dC, "Intel Celeron (Coppermine D0), .18um");
   FMSC (   6,  8, 10, MC, "Intel Mobile Celeron (Coppermine D0), .18um");
   FMS  (   6,  8, 10,     "Intel Pentium III / Pentium III Xeon / Celeron / Mobile Pentium III (Coppermine D0) / Mobile Celeron (Coppermine D0), .18um");
   FMC  (   6,  8,     dP, "Intel Pentium III (Coppermine), .18um");
   FMC  (   6,  8,     MP, "Intel Mobile Pentium III (Coppermine), .18um");
   FMC  (   6,  8,     sX, "Intel Pentium III Xeon (Coppermine), .18um");
   FMC  (   6,  8,     dC, "Intel Celeron (Coppermine), .18um");
   FMC  (   6,  8,     MC, "Intel Mobile Celeron (Coppermine), .18um");
   FM   (   6,  8,         "Intel Pentium III / Pentium III Xeon / Celeron / Mobile Pentium III (Coppermine) / Mobile Celeron (Coppermine), .18um");
   FMSC (   6,  9,  5, dP, "Intel Pentium M (Banias B1), .13um");
   FMSC (   6,  9,  5, dC, "Intel Celeron M (Banias B1), .13um");
   FMS  (   6,  9,  5,     "Intel Pentium M / Celeron M (Banias B1), .13um");
   FMC  (   6,  9,     dP, "Intel Pentium M (Banias), .13um");
   FMC  (   6,  9,     dC, "Intel Celeron M (Banias), .13um");
   FM   (   6,  9,         "Intel Pentium M / Celeron M (Banias), .13um");
   FMS  (   6, 10,  0,     "Intel Pentium III Xeon (Cascades A0), .18um");
   FMS  (   6, 10,  1,     "Intel Pentium III Xeon (Cascades A1), .18um");
   FMS  (   6, 10,  4,     "Intel Pentium III Xeon (Cascades B0), .18um");
   FM   (   6, 10,         "Intel Pentium III Xeon (Cascades), .18um");
   FMSC (   6, 11,  1, dP, "Intel Pentium III (Tualatin A1), .13um");
   FMSC (   6, 11,  1, dC, "Intel Celeron (Tualatin A1), .13um");
   FMSC (   6, 11,  1, MC, "Intel Mobile Celeron (Tualatin A1), .13um");
   FMS  (   6, 11,  1,     "Intel Pentium III / Celeron / Mobile Celeron (Tualatin A1), .13um");
   FMSC (   6, 11,  4, dP, "Intel Pentium III (Tualatin B1), .13um");
   FMSC (   6, 11,  4, dC, "Intel Celeron (Tualatin B1), .13um");
   FMSC (   6, 11,  4, MC, "Intel Mobile Celeron (Tualatin B1), .13um");
   FMS  (   6, 11,  4,     "Intel Pentium III / Celeron / Mobile Celeron (Tualatin B1), .13um");
   FMC  (   6, 11,     dP, "Intel Pentium III (Tualatin), .13um");
   FMC  (   6, 11,     dC, "Intel Celeron (Tualatin), .13um");
   FMC  (   6, 11,     MC, "Intel Mobile Celeron (Tualatin), .13um");
   FM   (   6, 11,         "Intel Pentium III / Celeron / Mobile Celeron (Tualatin), .13um");
   FMSC (   6, 13,  6, dP, "Intel Pentium M (Dothan B1), 90nm");
   FMSC (   6, 13,  6, dC, "Intel Celeron M (Dothan B1), 90nm");
   FMS  (   6, 13,  6,     "Intel Pentium M (Dothan B1) / Celeron M (Dothan B1), 90nm");
   FMSC (   6, 13,  8, dP, "Intel Pentium M (Dothan C0), 90nm");
   FMSC (   6, 13,  8, dC, "Intel Celeron M (Dothan C0), 90nm");
   FMS  (   6, 13,  8,     "Intel Pentium M (Dothan C0) / Celeron M (Dothan C0), 90nm");
   FM   (   6, 13,         "Intel Pentium M (Dothan) / Celeron M (Dothan), 90nm");
   /*
   ** Is MP the correct code for:
   **    Intel Core Solo (Yonah C0) / Core Duo (Yonah C0)?
   */
   FMSC (   6, 14,  8, dc, "Intel Core Solo (Yonah C0), 65nm");
   FMSC (   6, 14,  8, Dc, "Intel Core Duo (Yonah C0), 65nm");
   FMSC (   6, 14,  8, dC, "Intel Celeron (Yonah C0), 65nm");
   FMSC (   6, 14,  8, sX, "Intel Xeon Processor LV (Sossaman C0), 65nm");
   FMS  (   6, 14,  8,     "Intel Core Solo (Yonah C0) / Core Duo (Yonah C0) / Xeon Processor LV (Sossaman C0) / Celeron (Yonah C0), 65nm");
   FMSC (   6, 14, 12, dc, "Intel Core Solo (Yonah D0), 65nm");
   FMSC (   6, 14, 12, Dc, "Intel Core Duo (Yonah D0), 65nm");
   FMS  (   6, 14, 12,     "Intel Core Solo (Yonah D0) / Core Duo (Yonah D0), 65nm");
   FMC  (   6, 14,     dc, "Intel Core Solo (Yonah), 65nm");
   FMC  (   6, 14,     Dc, "Intel Core Duo (Yonah), 65nm");
   FMC  (   6, 14,     sX, "Intel Xeon Processor LV (Sossaman), 65nm");
   FMC  (   6, 14,     dC, "Intel Celeron (Yonah), 65nm");
   FM   (   6, 14,         "Intel Core Solo (Yonah) / Core Duo (Yonah) / Xeon Processor LV (Sossaman) / Celeron (Yonah), 65nm");
   /*
   ** How to distinguish?
   **    Core 2 Duo (Conroe B1)
   **    Core 2 Extreme Processor (Conroe B1)
   */
   FMSC (   6, 15,  5, Dc, "Intel Core 2 Duo (Conroe B1) / Core 2 Extreme Processor (Conroe B1)");
   FMSC (   6, 15,  5, Da, "Intel Core 2 Duo (Allendale B1)");
   FMSC (   6, 15,  5, sX, "Intel Dual-Core Xeon Processor 5100 (Woodcrest B1) (pre-production), 65nm");
   FMS  (   6, 15,  5,     "Intel Core 2 Duo (Conroe/Allendale B1) / Core 2 Extreme Processor (Conroe B1)");
   /*
   ** How to distinguish?
   **    Core 2 Duo (Conroe B2)
   **    Core 2 Extreme Processor (Conroe B2)
   */
   FMSC (   6, 15,  6, Dc, "Intel Core 2 Duo (Conroe B2) / Core 2 Extreme Processor (Conroe B2)");
   FMSC (   6, 15,  6, Da, "Intel Core 2 Duo (Allendale B2)");
   FMS  (   6, 15,  6,     "Intel Core 2 Duo (Conroe/Allendale B2) / Core 2 Extreme Processor (Conroe B2) / Dual-Core Xeon Processor 5100 (Woodcrest B2), 65nm");
   FMC  (   6, 15,     Dc, "Intel Core 2 Duo (Conroe) / Core 2 Extreme Processor (Conroe)");
   FMC  (   6, 15,     Da, "Intel Core 2 Duo (Allendale)");
   FMC  (   6, 15,     sX, "Intel Dual-Core Xeon Processor 5100 (Woodcrest), 65nm");
   FM   (   6, 15,         "Intel Core 2 Duo (Conroe/Allendale) / Core 2 Extreme Processor (Conroe) / Dual-Core Xeon Processor 5100 (Woodcrest), 65nm");
   F    (   6,             "Intel Pentium II / Pentium III / Pentium M / Celeron / Mobile Celeron / Celeron M / Core Solo / Core Duo / Core 2 / Core 2 Extreme Processor / Xeon Processor LV / Xeon Processor 5100 (unknown model)");
   FMS  (   7,  6,  4,     "Intel Itanium (C0)");
   FMS  (   7,  7,  4,     "Intel Itanium (C1)");
   FMS  (   7,  8,  4,     "Intel Itanium (C2)");
   F    (   7,             "Intel Itanium (unknown model)");
   XFMS (   0,  0,  7,     "Intel Pentium 4 (Willamette B2), .18um");
   XFMSC(   0,  0, 10, dP, "Intel Pentium 4 (Willamette C1), .18um");
   XFMSC(   0,  0, 10, sX, "Intel Xeon (Foster C1), .18um");
   XFMS (   0,  0, 10,     "Intel Pentium 4 (Willamette C1) / Xeon (Foster C1), .18um");
   XFMC (   0,  0,     dP, "Intel Pentium 4 (Willamette), .18um");
   XFMC (   0,  0,     sX, "Intel Xeon (Foster), .18um");
   XFM  (   0,  0,         "Intel Pentium 4 (Willamette) / Xeon (Foster), .18um");
   XFMS (   0,  1,  1,     "Intel Xeon MP (Foster C0), .18um");
   XFMSC(   0,  1,  2, dP, "Intel Pentium 4 (Willamette D0), .18um");
   XFMSC(   0,  1,  2, sX, "Intel Xeon (Foster D0), .18um");
   XFMS (   0,  1,  2,     "Intel Pentium 4 (Willamette D0) / Xeon (Foster D0), .18um");
   XFMSC(   0,  1,  3, dP, "Intel Pentium 4(Willamette E0), .18um");
   XFMSC(   0,  1,  3, dC, "Intel Celeron 478-pin (Willamette E0), .18um");
   XFMS (   0,  1,  3,     "Intel Pentium 4 / Celeron (Willamette E0), .18um");
   XFMC (   0,  1,     dP, "Intel Pentium 4 (Willamette), .18um");
   XFMC (   0,  1,     sX, "Intel Xeon (Foster), .18um");
   XFM  (   0,  1,         "Intel Pentium 4 (Willamette) / Xeon (Foster), .18um");
   XFMS (   0,  2,  2,     "Intel Xeon MP (Gallatin A0), .13um");
   XFMSC(   0,  2,  4, dP, "Intel Pentium 4 (Northwood B0), .13um");
   XFMSC(   0,  2,  4, sX, "Intel Xeon (Prestonia B0), .13um");
   XFMSC(   0,  2,  4, MM, "Intel Mobile Pentium 4 Processor-M (Northwood B0), .13um");
   XFMSC(   0,  2,  4, MC, "Intel Mobile Celeron (Northwood B0), .13um");
   XFMS (   0,  2,  4,     "Intel Pentium 4 (Northwood B0) / Xeon (Prestonia B0) / Mobile Pentium 4 Processor-M (Northwood B0) / Mobile Celeron (Northwood B0), .13um");
   XFMSC(   0,  2,  5, dP, "Intel Pentium 4 (Northwood B1/M0), .13um");
   XFMSC(   0,  2,  5, sX, "Intel Xeon (Prestonia B1), .13um");
   XFMSC(   0,  2,  5, sM, "Intel Xeon MP (Gallatin B1), .13um");
   XFMS (   0,  2,  5,     "Intel Pentium 4 (Northwood B1/M0) / Xeon (Prestonia B1) / Xeon MP (Gallatin B1), .13um");
   XFMS (   0,  2,  6,     "Intel Xeon MP (Gallatin C0), .13um");
   XFMSC(   0,  2,  7, dP, "Intel Pentium 4 (Northwood C1), .13um");
   XFMSC(   0,  2,  7, sX, "Intel Xeon (Prestonia C1), .13um");
   XFMSC(   0,  2,  7, MM, "Intel Mobile Pentium 4 Processor-M (Northwood C1), .13um");
   XFMSC(   0,  2,  7, dC, "Intel Celeron 478-pin (Northwood C1), .13um");
   XFMSC(   0,  2,  7, MC, "Intel Mobile Celeron (Northwood C1), .13um");
   XFMS (   0,  2,  7,     "Intel Pentium 4 (Northwood C1) / Xeon (Prestonia C1) / Mobile Pentium 4 Processor-M (Northwood C1) / Celeron 478-Pin (Northwood C1) / Mobile Celeron (Northwood C1), .13um");
   XFMSC(   0,  2,  9, dP, "Intel Pentium 4 (Northwood D1), .13um");
   XFMSC(   0,  2,  9, MP, "Intel Mobile Pentium 4 (Northwood D1), .13um");
   XFMSC(   0,  2,  9, sX, "Intel Xeon (Prestonia D1), .13um");
   XFMSC(   0,  2,  9, MM, "Intel Mobile Pentium 4 Processor-M (Northwood D1), .13um");
   XFMSC(   0,  2,  9, dC, "Intel Celeron 478-pin (Northwood D1), .13um");
   XFMSC(   0,  2,  9, MC, "Intel Mobile Celeron (Northwood D1), .13um");
   XFMS (   0,  2,  9,     "Intel Pentium 4 (Northwood D1) / Xeon (Prestonia D1) / Mobile Pentium 4 (Northwood D1) / Mobile Pentium 4 Processor-M (Northwood D1) / Celeron 478-pin, .13um");
   XFMC (   0,  2,     dP, "Intel Pentium 4 (Northwood), .13um");
   XFMC (   0,  2,     sX, "Intel Xeon (Prestonia), .13um");
   XFMC (   0,  2,     sM, "Intel Xeon MP (Gallatin), .13um");
   XFM  (   0,  2,         "Intel Pentium 4 (Northwood) / Xeon (Prestonia) / Xeon MP (Gallatin) / Mobile Pentium 4 / Mobile Pentium 4 Processor-M / Celeron 478-pin, .13um");
   XFMSC(   0,  3,  3, dP, "Intel Pentium 4 (Prescott C0), 90nm");
   XFMSC(   0,  3,  3, dC, "Intel Celeron D (Prescott C0), 90nm");
   XFMS (   0,  3,  3,     "Intel Pentium 4 (Prescott C0) / Celeron D (Prescott C0), 90nm");
   XFMSC(   0,  3,  4, dP, "Intel Pentium 4 (Prescott D0), 90nm");
   XFMSC(   0,  3,  4, MP, "Intel Mobile Pentium 4 (Prescott D0), 90nm");
   XFMSC(   0,  3,  4, dC, "Intel Celeron D (Prescott D0), 90nm");
   XFMSC(   0,  3,  4, sX, "Intel Xeon (Nocona D0), 90nm");
   XFMS (   0,  3,  4,     "Intel Pentium 4 (Prescott D0) / Xeon (Nocona D0) / Mobile Pentium 4 (Prescott D0), 90nm");
   XFMC (   0,  3,     dP, "Intel Pentium 4 (Prescott), 90nm");
   XFMC (   0,  3,     MP, "Intel Mobile Pentium 4 (Prescott), 90nm");
   XFMC (   0,  3,     sX, "Intel Xeon (Nocona), 90nm");
   XFM  (   0,  3,         "Intel Pentium 4 (Prescott) / Xeon (Nocona) / Mobile Pentium 4 (Prescott), 90nm");
   XFMSC(   0,  4,  1, dP, "Intel Pentium 4 (Prescott E0), 90nm");
   XFMSC(   0,  4,  1, dC, "Intel Celeron D (Prescott E0), 90nm");
   XFMSC(   0,  4,  1, MP, "Intel Mobile Pentium 4 (Prescott E0), 90nm");
   XFMSC(   0,  4,  1, sX, "Intel Xeon (Nocona E0), 90nm");
   XFMSC(   0,  4,  1, sM, "Intel Xeon MP (Cranford A0), 90nm");
   XFMSC(   0,  4,  1, sP, "Intel Xeon MP (Potomac C0), 90nm");
   XFMS (   0,  4,  1,     "Intel Pentium 4 (Prescott E0) / Xeon (Nocona E0) / Xeon MP (Cranford A0 / Potomac C0) / Celeron D (Prescott E0 ) / Mobile Pentium 4 (Prescott E0), 90nm");
   XFMSC(   0,  4,  3, dP, "Intel Pentium 4 (Prescott N0), 90nm");
   XFMSC(   0,  4,  3, sX, "Intel Xeon (Nocona N0), 90nm");
   XFMSC(   0,  4,  3, sI, "Intel Xeon (Irwindale N0), 90nm");
   XFMS (   0,  4,  3,     "Intel Pentium 4 (Prescott N0) / Xeon (Nocona N0 / Irwindale N0), 90nm");
   XFMSC(   0,  4,  4, dD, "Intel Pentium D Processor 8x0 (Smithfield A0), 90nm");
   XFMSC(   0,  4,  4, dc, "Intel Pentium Extreme Edition Processor 840 (Smithfield A0), 90nm");
   XFMS (   0,  4,  4,     "Intel Pentium D Processor 8x0 (Smithfield A0) / Pentium Extreme Edition Processor 840 (Smithfield A0), 90nm");
   XFMSC(   0,  4,  7, dD, "Intel Pentium D Processor 8x0 (Smithfield B0), 90nm");
   XFMSC(   0,  4,  7, dc, "Pentium Extreme Edition Processor 840 (Smithfield B0), 90nm");
   XFMS (   0,  4,  7,     "Intel Pentium D Processor 8x0 (Smithfield B0) / Pentium Extreme Edition Processor 840 (Smithfield B0), 90nm");
   XFMSC(   0,  4,  8, sX, "Intel Dual-Core Xeon (Paxville A0), 90nm");
   XFMSC(   0,  4,  8, s7, "Intel Dual-Core Xeon Processor 7000 (Paxville A0), 90nm");
   XFMS (   0,  4,  8,     "Intel Dual-Core Xeon (Paxville A0) / Dual-Core Xeon Processor 7000 (Paxville A0), 90nm");
   XFMSC(   0,  4,  9, dP, "Intel Pentium 4 (Prescott G1), 90nm");
   XFMSC(   0,  4,  9, sM, "Intel Xeon MP (Cranford B0), 90nm");
   XFMSC(   0,  4,  9, dC, "Intel Celeron D (Prescott G1), 90nm");
   XFMS (   0,  4,  9,     "Intel Pentium 4 (Prescott G1) / Xeon MP (Cranford B0) / Celeron D (Prescott G1), 90nm");
   XFMSC(   0,  4, 10, dP, "Intel Pentium 4 (Prescott R0), 90nm");
   XFMSC(   0,  4, 10, sX, "Intel Xeon (Nocona R0), 90nm");
   XFMSC(   0,  4, 10, sI, "Intel Xeon (Irwindale R0), 90nm");
   XFMS (   0,  4, 10,     "Intel Pentium 4 (Prescott R0) / Xeon (Nocona R0 / Irwindale R0), 90nm");
   XFMC (   0,  4,     dP, "Intel Pentium 4 (Prescott) / Pentium Extreme Edition (Smithfield A0), 90nm");
   XFMC (   0,  4,     dD, "Intel Pentium D (Smithfield A0), 90nm");
   XFMC (   0,  4,     dC, "Intel Celeron D (Prescott), 90nm");
   XFMC (   0,  4,     MP, "Intel Mobile Pentium 4 (Prescott), 90nm");
   XFMC (   0,  4,     sX, "Intel Xeon (Nocona), 90nm");
   XFMC (   0,  4,     sI, "Intel Xeon (Irwindale), 90nm");
   XFMC (   0,  4,     sM, "Intel Xeon MP (Nocona), 90nm");
   XFM  (   0,  4,         "Intel Pentium 4 (Prescott) / Xeon (Nocona / Irwindale) / Pentium D (Smithfield A0) / Pentium Extreme Edition (Smithfield A0) / Mobile Pentium 4 (Prescott) / Xeon MP (Nocona) / Xeon MP (Cranford / Potomac) / Celeron D (Prescott) / Dual-Core Xeon (Paxville A0) / Dual-Core Xeon Processor 7000 (Paxville A0), 90nm");
   /*
   ** How to distinguish?
   **    Pentium 4 Processor 6x1 (Cedar Mill)
   **    Pentium Extreme Edition Processor 955 (Presler)
   */
   XFMSC(   0,  6,  2, dD, "Intel Pentium D Processor 9xx (Presler B1), 65nm");
   XFMSC(   0,  6,  2, dP, "Intel Pentium 4 Processor 6x1 (Cedar Mill B1) / Pentium Extreme Edition Processor 955 (Presler B1)");
   XFMS (   0,  6,  2,     "Intel Pentium 4 Processor 6x1 (Cedar Mill B1) / Pentium Extreme Edition Processor 955 (Presler B1) / Pentium D Processor 900 (Presler B1), 65nm");
   XFMSC(   0,  6,  4, dD, "Intel Pentium D Processor 9xx (Presler C1), 65nm");
   XFMSC(   0,  6,  4, dP, "Intel Pentium 4 Processor 6x1 (Cedar Mill C1) / Pentium Extreme Edition Processor 955 (Presler C1)");
   XFMSC(   0,  6,  4, dC, "Intel Celeron D Processor 3xx (Cedar Mill C1), 65nm");
   XFMSC(   0,  6,  4, sX, "Intel Xeon Processor 5000 (Dempsey C1), 65nm");
   XFMS (   0,  6,  4,     "Intel Pentium 4 Processor 6x1 (Cedar Mill C1) / Pentium Extreme Edition Processor 955 (Presler C1) / Intel Pentium D Processor 9xx (Presler C1), 65nm / Intel Xeon Processor 5000 (Dempsey C1) / Celeron D Processor 3xx (Cedar Mill C1), 65nm");
   XFMS (   0,  6,  8,     "Intel Xeon Processor 71x0 (Tulsa B0), 65nm");
   XFMC (   0,  6,     dD, "Intel Pentium D Processor 9xx (Presler), 65nm");
   XFMC (   0,  6,     dP, "Intel Pentium 4 Processor 6x1 (Cedar Mill) / Pentium Extreme Edition Processor 955 (Presler)");
   XFMC (   0,  6,     dC, "Intel Celeron D Processor 3xx (Cedar Mill), 65nm");
   XFMC (   0,  6,     sX, "Intel Xeon Processor 5000 (Dempsey) / Xeon Processor 71x0 (Tulsa), 65nm");
   XFM  (   0,  6,         "Intel Pentium 4 Processor 6x1 (Cedar Mill) / Pentium Extreme Edition Processor 955 (Presler) / Pentium D Processor 900 (Presler) / Intel Xeon Processor 5000 (Dempsey) / Xeon Processor 71x0 (Tulsa) / Celeron D Processor 3xx (Cedar Mill), 65nm");
   XFC  (   0,         dP, "Intel Pentium 4 (unknown model)");
   XFC  (   0,         sX, "Intel Xeon (unknown model)");
   XFC  (   0,         sM, "Intel Xeon MP (unknown model)");
   XFC  (   0,         sP, "Intel Xeon MP (unknown model)");
   XF   (   0,             "Intel Pentium 4 / Xeon / Xeon MP / Mobile Pentium 4 / Celeron / Celeron D / Mobile Celeron / Dual-Core Xeon / Dual-Core Xeon Processor 7000 (unknown model)");
   XFMS (   1,  0,  7,     "Intel Itanium2 (McKinley B3), .18um");
   XFM  (   1,  0,         "Intel Itanium2 (McKinley), .18um");
   XFMS (   1,  1,  5,     "Intel Itanium2 (Madison/Deerfield B1), .13um");
   XFM  (   1,  1,         "Intel Itanium2 (Madison/Deerfield), .13um");
   XFMS (   1,  2,  1,     "Intel Itanium2 (Madison A1), .13um");
   XFMS (   1,  2,  2,     "Intel Itanium2 (Madison A2), .13um");
   XFM  (   1,  2,         "Intel Itanium2 (Madison), .13um");
   XFMS (   2,  0,  5,     "Intel Itanium2 (Montecito C1), 90nm");
   XFM  (   2,  0,         "Intel Itanium2 (Montecito), 90nm");
   XF   (   1,             "Intel Itanium2 (unknown model)");
   DEFAULT                ("unknown");
   printf("\n");
}

static void
print_synth_amd_model(const code_stash_t*  stash)
{
   unsigned int  msb;
   unsigned int  NN;

   if (stash == NULL) return;

   if (__XB(stash->val_80000001_ebx) != 0) {
      msb = BIT_EXTRACT_LE(__XB(stash->val_80000001_ebx), 6, 12);
      NN  = BIT_EXTRACT_LE(__XB(stash->val_80000001_ebx), 0,  6);
   } else if (_B(stash->val_1_ebx) != 0) {
      /* 
      ** Nothing explains how the msb of this brand should be interpreted.  I
      ** concluded it must be shifted left by 2 with a little reverse
      ** engineering.  That's the only way it gets the right answer for the
      ** machine aero, which has brand 0x83, and is known to be a 244.
      */
      msb = BIT_EXTRACT_LE(__B(stash->val_1_ebx), 5, 8) << 2;
      NN  = BIT_EXTRACT_LE(__B(stash->val_1_ebx), 0, 5);
   } else {
      return;
   }

#define XX  (22 + NN)
#define YY  (38 + 2*NN)
#define ZZ  (24 + NN)
#define TT  (24 + NN)
#define RR  (45 + 5*NN)
#define EE  ( 9 + NN)

   switch (msb) {
   case 0x04:
   case 0x05:
   case 0x08:
   case 0x09:
   case 0x1d:
   case 0x1e:
   case 0x20:
      printf(" Processor %02d00+", XX);
      break;
   case 0x0a:
      printf(" ML-%02d", XX);
      break;
   case 0x0b:
      printf(" MT-%02d", XX);
      break;
   case 0x0c:
   case 0x0d:
      printf(" Processor 1%02d", YY);
      break;
   case 0x0e:
      printf(" Processor 1%02d HE", YY);
      break;
   case 0x0f:
      printf(" Processor 1%02d EE", YY);
      break;
   case 0x10:
   case 0x11:
      printf(" Processor 2%02d", YY);
      break;
   case 0x12:
      printf(" Processor 2%02d HE", YY);
      break;
   case 0x13:
      printf(" Processor 2%02d EE", YY);
      break;
   case 0x14:
   case 0x15:
      printf(" Processor 8%02d", YY);
      break;
   case 0x16:
      printf(" Processor 8%02d HE", YY);
      break;
   case 0x17:
      printf(" Processor 8%02d EE", YY);
      break;
   case 0x18:
      printf(" Processor %02d00+", EE);
   case 0x21:
   case 0x22:
   case 0x23:
   case 0x26:
      printf(" Processor %02d00+", TT);
      break;
   case 0x24:
      printf("-%02d", ZZ);
      break;
   case 0x29:
   case 0x2c:
   case 0x2d:
   case 0x38:
      printf(" Processor 1%02d", RR);
      break;
   case 0x2e:
      printf(" Processor 1%02d HE", RR);
      break;
   case 0x2f:
      printf(" Processor 1%02d EE", RR);
      break;
   case 0x2a:
   case 0x30:
   case 0x31:
   case 0x39:
      printf(" Processor 2%02d", RR);
      break;
   case 0x32:
      printf(" Processor 2%02d HE", RR);
      break;
   case 0x33:
      printf(" Processor 2%02d EE", RR);
      break;
   case 0x2b:
   case 0x34:
   case 0x35:
   case 0x3a:
      printf(" Processor 8%02d", RR);
      break;
   case 0x36:
      printf(" Processor 8%02d HE", RR);
      break;
   case 0x37:
      printf(" Processor 8%02d EE", RR);
      break;
   }

#undef XX
#undef YY
#undef ZZ
#undef TT
#undef RR
#undef EE
}

static void
print_synth_amd(const char*          name,
                unsigned int         val,
                code_t               cd,
                const code_stash_t*  stash)
{
   printf(name);
   START;
   FM    (4,     3,         "AMD 80486DX2");
   FM    (4,     7,         "AMD 80486DX2WB");
   FM    (4,     8,         "AMD 80486DX4");
   FM    (4,     9,         "AMD 80486DX4WB");
   FM    (4,    14,         "AMD 5x86");
   FM    (4,    15,         "AMD 5xWB");
   F     (4,                "AMD 80486 / 5x (unknown model)");
   FM    (5,     0,         "AMD SSA5 (PR75, PR90, PR100)");
   FM    (5,     1,         "AMD 5k86 (PR120, PR133)");
   FM    (5,     2,         "AMD 5k86 (PR166)");
   FM    (5,     3,         "AMD 5k86 (PR200)");
   FM    (5,     6,         "AMD K6, .30um");
   FM    (5,     7,         "AMD K6 (Little Foot), .25um");
   FMS   (5,     8,  0,     "AMD K6-2 (Chomper A)");
   FMS   (5,     8, 12,     "AMD K6-2 (Chomper A)");
   FM    (5,     8,         "AMD K6-2 (Chomper)");
   FMS   (5,     9,  1,     "AMD K6-III (Sharptooth B)");
   FM    (5,     9,         "AMD K6-III (Sharptooth)");
   FM    (5,    13,         "AMD K6-2+, K6-III+");
   F     (5,                "AMD 5k86 / K6 (unknown model)");
   FM    (6,     1,         "AMD Athlon, .25um");
   FM    (6,     2,         "AMD Athlon (K75 / Pluto / Orion), .18um");
   FMS   (6,     3,  0,     "AMD Duron / mobile Duron (Spitfire A0)");
   FMS   (6,     3,  1,     "AMD Duron / mobile Duron (Spitfire A2)");
   FM    (6,     3,         "AMD Duron / mobile Duron (Spitfire)");
   FMS   (6,     4,  2,     "AMD Athlon (Thunderbird A4-A7)");
   FMS   (6,     4,  4,     "AMD Athlon (Thunderbird A9)");
   FM    (6,     4,         "AMD Athlon (Thunderbird)");
   FMSC  (6,     6,  0, dA, "AMD Athlon (Palomino A0)");
   FMSC  (6,     6,  0, sA, "AMD Athlon MP (Palomino A0)");
   FMSC  (6,     6,  0, MA, "AMD mobile Athlon 4 (Palomino A0)");
   FMSC  (6,     6,  0, sD, "AMD Duron MP (Palomino A0)");
   FMSC  (6,     6,  0, MD, "AMD mobile Duron (Palomino A0)");
   FMS   (6,     6,  0,     "AMD Athlon / Athlon MP mobile Athlon 4 / mobile Duron (Palomino A0)");
   FMSC  (6,     6,  1, dA, "AMD Athlon (Palomino A2)");
   FMSC  (6,     6,  1, sA, "AMD Athlon MP (Palomino A2)");
   FMSC  (6,     6,  1, dD, "AMD Duron (Palomino A2)");
   FMSC  (6,     6,  1, MA, "AMD mobile Athlon 4 (Palomino A2)");
   FMSC  (6,     6,  1, sD, "AMD Duron MP (Palomino A2)");
   FMSC  (6,     6,  1, MD, "AMD mobile Duron (Palomino A2)");
   FMS   (6,     6,  1,     "AMD Athlon / Athlon MP / Duron / mobile Athlon / mobile Duron (Palomino A2)");
   FMSC  (6,     6,  2, sA, "AMD Athlon MP (Palomino A5)");
   FMSC  (6,     6,  2, dX, "AMD Athlon XP (Palomino A5)");
   FMSC  (6,     6,  2, dD, "AMD Duron (Palomino A5)");
   FMSC  (6,     6,  2, MA, "AMD mobile Athlon 4 (Palomino A5)");
   FMSC  (6,     6,  2, sD, "AMD Duron MP (Palomino A5)");
   FMSC  (6,     6,  2, MD, "AMD mobile Duron (Palomino A5)");
   FMS   (6,     6,  2,     "AMD Athlon MP / Athlon XP / Duron / Duron MP / mobile Athlon / mobile Duron (Palomino A5)");
   FMC   (6,     6,     dA, "AMD Athlon (Palomino)");
   FMC   (6,     6,     dX, "AMD Athlon XP (Palomino)");
   FMC   (6,     6,     dD, "AMD Duron (Palomino)");
   FMC   (6,     6,     MA, "AMD mobile Athlon (Palomino)");
   FMC   (6,     6,     MD, "AMD mobile Duron (Palomino)");
   FM    (6,     6,         "AMD Athlon / Athlon MP / Athlon XP / Duron / Duron MP / mobile Athlon / mobile Duron (Palomino)");
   FMSC  (6,     7,  0, dD, "AMD Duron (Morgan A0)");
   FMSC  (6,     7,  0, sD, "AMD Duron MP (Morgan A0)");
   FMSC  (6,     7,  0, MD, "AMD mobile Duron (Morgan A0)");
   FMS   (6,     7,  0,     "AMD Duron / Duron MP / mobile Duron (Morgan A0)");
   FMSC  (6,     7,  1, dD, "AMD Duron (Morgan A1)");
   FMSC  (6,     7,  1, sD, "AMD Duron MP (Morgan A1)");
   FMSC  (6,     7,  1, MD, "AMD mobile Duron (Morgan A1)");
   FMS   (6,     7,  1,     "AMD Duron / Duron MP / mobile Duron (Morgan A1)");
   FMC   (6,     7,     dD, "AMD Duron (Morgan)");
   FMC   (6,     7,     sD, "AMD Duron MP (Morgan)");
   FMC   (6,     7,     MD, "AMD mobile Duron (Morgan)");
   FM    (6,     7,         "AMD Duron / Duron MP / mobile Duron (Morgan)");
   FMSC  (6,     8,  0, dX, "AMD Athlon XP (Thoroughbred A0)");
   FMSC  (6,     8,  0, sA, "AMD Athlon MP (Thoroughbred A0)");
   FMSC  (6,     8,  0, dD, "AMD Duron (Applebred A0)");
   FMSC  (6,     8,  0, sD, "AMD Duron MP (Applebred A0)");
   FMSC  (6,     8,  0, dS, "AMD Sempron (Thoroughbred A0)");
   FMSC  (6,     8,  0, MX, "AMD mobile Athlon XP (Thoroughbred A0)");
   FMS   (6,     8,  0,     "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred A0)");
   FMSC  (6,     8,  1, dX, "AMD Athlon XP (Thoroughbred B0)");
   FMSC  (6,     8,  1, sA, "AMD Athlon MP (Thoroughbred B0)");
   FMSC  (6,     8,  1, dS, "AMD Sempron (Thoroughbred B0)");
   FMSC  (6,     8,  1, dD, "AMD Duron (Thoroughbred B0)");
   FMSC  (6,     8,  1, sD, "AMD Duron MP (Thoroughbred B0)");
   FMS   (6,     8,  1,     "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred B0)");
   FMC   (6,     8,     dX, "AMD Athlon XP (Thoroughbred)");
   FMC   (6,     8,     sA, "AMD Athlon MP (Thoroughbred)");
   FMC   (6,     8,     dS, "AMD Sempron (Thoroughbred)");
   FMC   (6,     8,     sD, "AMD Duron MP (Thoroughbred)");
   FMC   (6,     8,     MX, "AMD mobile Athlon XP (Thoroughbred)");
   FM    (6,     8,         "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred)");
   FMSC  (6,    10,  0, dX, "AMD Athlon XP (Barton A2)");
   FMSC  (6,    10,  0, dt, "AMD Athlon XP (Thorton A2)");
   FMSC  (6,    10,  0, sA, "AMD Athlon MP (Barton A2)");
   FMSC  (6,    10,  0, dS, "AMD Sempron (Barton A2)");
   FMSC  (6,    10,  0, MX, "AMD mobile Athlon XP-M (Barton A2)");
   FMSC  (6,    10,  0, ML, "AMD mobile Athlon XP-M (LV) (Barton A2)");
   FMS   (6,    10,  0,     "AMD Athlon XP / Athlon MP / Sempron / mobile Athlon XP-M / mobile Athlon XP-M (LV) (Barton A2)");
   FMC   (6,    10,     dX, "AMD Athlon XP (Barton)");
   FMC   (6,    10,     sA, "AMD Athlon MP (Barton)");
   FMC   (6,    10,     dS, "AMD Sempron (Barton)");
   FMC   (6,    10,     MX, "AMD mobile Athlon XP-M (Barton)");
   FMC   (6,    10,     ML, "AMD mobile Athlon XP-M (LV) (Barton)");
   FM    (6,    10,         "AMD Athlon XP / Athlon MP / Sempron / mobile Athlon XP-M / mobile Athlon XP-M (LV) (Barton)");
   F     (6,                "AMD Athlon / Athlon XP / Athlon MP / Duron / Duron MP / Sempron / mobile Athlon / mobile Athlon XP-M / mobile Athlon XP-M (LV) / mobile Duron (unknown model)");
   F     (7,                "AMD Opteron (unknown model)");
   XFXMS (0, 0,  4,  0,     "AMD Athlon 64 (SledgeHammer SH7-B0), .13um");
   XFXMSC(0, 0,  4,  8, dA, "AMD Athlon 64 (SledgeHammer SH7-C0), 754-pin, .13um");
   XFXMSC(0, 0,  4,  8, MA, "AMD mobile Athlon 64 (SledgeHammer SH7-C0), 754-pin, .13um");
   XFXMSC(0, 0,  4,  8, MX, "AMD mobile Athlon XP-M (SledgeHammer SH7-C0), 754-pin, .13um");
   XFXMS (0, 0,  4,  8,     "AMD Athlon 64 (SledgeHammer SH7-C0) / mobile Athlon 64 (SledgeHammer SH7-C0) / mobile Athlon XP-M (SledgeHammer SH7-C0), 754-pin, .13um");
   XFXMSC(0, 0,  4, 10, dA, "AMD Athlon 64 (SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMSC(0, 0,  4, 10, MA, "AMD mobile Athlon 64 (SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMSC(0, 0,  4, 10, MX, "AMD mobile Athlon XP-M (SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMS (0, 0,  4, 10,     "AMD Athlon 64 (SledgeHammer SH7-CG) / mobile Athlon 64 (SledgeHammer SH7-CG) / mobile Athlon XP-M (SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMC (0, 0,  4,     dA, "AMD Athlon 64 (SledgeHammer SH7), .13um");
   XFXMC (0, 0,  4,     MA, "AMD mobile Athlon 64 (SledgeHammer SH7), .13um");
   XFXMC (0, 0,  4,     MX, "AMD mobile Athlon XP-M (SledgeHammer SH7), .13um");
   XFXM  (0, 0,  4,         "AMD Athlon 64 (SledgeHammer SH7) / mobile Athlon 64 (SledgeHammer SH7) / mobile Athlon XP-M (SledgeHammer SH7), .13um");
   XFXMS (0, 0,  5,  0,     "AMD Opteron (DP SledgeHammer SH7-B0), 940-pin, .13um");
   XFXMS (0, 0,  5,  1,     "AMD Opteron (DP SledgeHammer SH7-B3), 940-pin, .13um");
   XFXMSC(0, 0,  5,  8, dO, "AMD Opteron (DP SledgeHammer SH7-C0), 940-pin, .13um");
   XFXMSC(0, 0,  5,  8, dF, "AMD Athlon 64 FX (DP SledgeHammer SH7-C0), 940-pin, .13um");
   XFXMS (0, 0,  5,  8,     "AMD Opteron (DP SledgeHammer SH7-C0) / Athlon 64 FX (DP SledgeHammer SH7-C0), 940-pin, .13um");
   XFXMSC(0, 0,  5, 10, dO, "AMD Opteron (DP SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMSC(0, 0,  5, 10, dF, "AMD Athlon 64 FX (DP SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMS (0, 0,  5, 10,     "AMD Opteron (DP SledgeHammer SH7-CG) / Athlon 64 FX (DP SledgeHammer SH7-CG), 940-pin, .13um");
   XFXMC (0, 0,  5,     dO, "AMD Opteron (SledgeHammer SH7), 940-pin, .13um");
   XFXMC (0, 0,  5,     dF, "AMD Athlon 64 FX (SledgeHammer SH7), 940-pin, .13um");
   XFXM  (0, 0,  5,         "AMD Opteron (SledgeHammer SH7) / Athlon 64 (SledgeHammer SH7) FX, 940-pin, .13um");
   XFXMSC(0, 0,  7, 10, dA, "AMD Athlon 64 (DP SledgeHammer SH7-CG), 939-pin, .13um");
   XFXMSC(0, 0,  7, 10, dF, "AMD Athlon 64 FX (DP SledgeHammer SH7-CG), 939-pin, .13um");
   XFXMS (0, 0,  7, 10,     "AMD Athlon 64 (DP SledgeHammer SH7-CG) / Athlon 64 FX (DP SledgeHammer SH7-CG), 939-pin, .13um");
   XFXMC (0, 0,  7,     dA, "AMD Athlon 64 (DP SledgeHammer SH7), 939-pin, .13um");
   XFXMC (0, 0,  7,     dF, "AMD Athlon 64 FX (DP SledgeHammer SH7), 939-pin, .13um");
   XFXM  (0, 0,  7,         "AMD Athlon 64 (DP SledgeHammer SH7) / Athlon 64 FX (DP SledgeHammer SH7), 939-pin, .13um");
   XFXMSC(0, 0,  8,  2, dA, "AMD Athlon 64 (ClawHammer CH7-CG), 754-pin, .13um");
   XFXMSC(0, 0,  8,  2, MA, "AMD mobile Athlon 64 (Odessa CH7-CG), 754-pin, .13um");
   XFXMSC(0, 0,  8,  2, MS, "AMD mobile Sempron (ClawHammer CH7-CG), 754-pin, .13um");
   XFXMSC(0, 0,  8,  2, MX, "AMD mobile Athlon XP-M (ClawHammer CH7-CG), 754-pin, .13um");
   XFXMS (0, 0,  8,  2,     "AMD Athlon 64 (ClawHammer CH7-CG) / mobile Athlon 64 (Odessa CH7-CG) / mobile Sempron (ClawHammer CH7-CG) / mobile Athlon XP-M (ClawHammer CH7-CG), 754-pin, .13um");
   XFXMC (0, 0,  8,     dA, "AMD Athlon 64 (ClawHammer CH7), 754-pin, .13um");
   XFXMC (0, 0,  8,     MA, "AMD mobile Athlon 64 (Odessa CH7), 754-pin, .13um");
   XFXMC (0, 0,  8,     MS, "AMD mobile Sempron (Odessa CH7), 754-pin, .13um");
   XFXMC (0, 0,  8,     MX, "AMD mobile Athlon XP-M (Odessa CH7), 754-pin, .13um");
   XFXM  (0, 0,  8,         "AMD Athlon 64 (ClawHammer CH7) / mobile Athlon 64 (Odessa CH7) / mobile Sempron (Odessa CH7) / mobile Athlon XP-M (Odessa CH7), 754-pin, .13um");
   XFXMS (0, 0, 11,  2,     "AMD Athlon 64 (ClawHammer CH7-CG), 939-pin, .13um");
   XFXM  (0, 0, 11,         "AMD Athlon 64 (ClawHammer CH7), 939-pin, .13um");
   XFXMSC(0, 0, 12,  0, dA, "AMD Athlon 64 (NewCastle DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 12,  0, MA, "AMD mobile Athlon 64 (Dublin DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 12,  0, dS, "AMD Sempron (Paris DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 12,  0, MS, "AMD mobile Sempron (Sonora DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 12,  0, MX, "AMD mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXMS (0, 0, 12,  0,     "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG) / mobile Sempron (Sonora DH7-CG) / mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXMC (0, 0, 12,     dA, "AMD Athlon 64 (NewCastle DH7), 754-pin, .13um");
   XFXMC (0, 0, 12,     MA, "AMD mobile Athlon 64 (Dublin DH7), 754-pin, .13um");
   XFXMC (0, 0, 12,     dS, "AMD Sempron (Paris DH7), 754-pin, .13um");
   XFXMC (0, 0, 12,     MS, "AMD mobile Sempron (Sonora DH7), 754-pin, .13um");
   XFXMC (0, 0, 12,     MX, "AMD mobile Athlon XP-M (NewCastle DH7), 754-pin, .13um");
   XFXM  (0, 0, 12,         "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7) / mobile Sempron (Sonora DH7) / mobile Athlon XP-M (Dublin DH7), 754-pin, .13um");
   XFXMSC(0, 0, 14,  0, dA, "AMD Athlon 64 (NewCastle DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 14,  0, MA, "AMD mobile Athlon 64 (Dublin DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 14,  0, dS, "AMD Sempron (Paris DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 14,  0, MS, "AMD mobile Sempron (Sonora DH7-CG), 754-pin, .13um");
   XFXMSC(0, 0, 14,  0, MX, "AMD mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXMS (0, 0, 14,  0,     "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG) / mobile Sempron (Sonora DH7-CG) / mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXMC (0, 0, 14,     dA, "AMD Athlon 64 (NewCastle DH7), 754-pin, .13um");
   XFXMC (0, 0, 14,     MA, "AMD mobile Athlon 64 (Dublin DH7), 754-pin, .13um");
   XFXMC (0, 0, 14,     dS, "AMD Sempron (Paris DH7), 754-pin, .13um");
   XFXMC (0, 0, 14,     MS, "AMD mobile Sempron (Sonora DH7), 754-pin, .13um");
   XFXMC (0, 0, 14,     MX, "AMD mobile Athlon XP-M (Dublin DH7), 754-pin, .13um");
   XFXM  (0, 0, 14,         "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7) / mobile Sempron (Sonora DH7) / mobile Athlon XP-M (Dublin DH7), 754-pin, .13um");
   XFXMSC(0, 0, 15,  0, dA, "AMD Athlon 64 (NewCastle DH7-CG), 939-pin, .13um");
   XFXMSC(0, 0, 15,  0, MA, "AMD mobile Athlon 64 (Dublin DH7-CG), 939-pin, .13um");
   XFXMSC(0, 0, 15,  0, dS, "AMD Sempron (Paris DH7-CG), 939-pin, .13um");
   XFXMS (0, 0, 15,  0,     "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG), 939-pin, .13um");
   XFXMC (0, 0, 15,     dA, "AMD Athlon 64 (NewCastle DH7), 939-pin, .13um");
   XFXMC (0, 0, 15,     MA, "AMD mobile Athlon 64 (Dublin DH7), 939-pin, .13um");
   XFXMC (0, 0, 15,     dS, "AMD Sempron (Paris DH7), 939-pin, .13um");
   XFXM  (0, 0, 15,         "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7), 939-pin, .13um");
   XFXMSC(0, 1,  4,  0, dA, "AMD Athlon 64 (Winchester SH7-D0), 754-pin, 90nm");
   XFXMSC(0, 1,  4,  0, MA, "AMD mobile Athlon 64 (Oakville SH7-D0), 754-pin, 90nm");
   XFXMSC(0, 1,  4,  0, MX, "AMD mobile Athlon XP-M (Oakville SH7-D0), 754-pin, 90nm");
   XFXMS (0, 1,  4,  0,     "AMD Athlon 64 (Winchester SH7-D0) / mobile Athlon 64 (Oakville SH7-D0) / mobile Athlon XP-M (Oakville SH7-D0), 754-pin, 90nm");
   XFXMC (0, 1,  4,     dA, "AMD Athlon 64 (Winchester SH7), 754-pin, 90nm");
   XFXMC (0, 1,  4,     MA, "AMD mobile Athlon 64 (Winchester SH7), 754-pin, 90nm");
   XFXMC (0, 1,  4,     MX, "AMD mobile Athlon XP-M (Winchester SH7), 754-pin, 90nm");
   XFXM  (0, 1,  4,         "AMD Athlon 64 (Winchester SH7) / mobile Athlon 64 (Winchester SH7) / mobile Athlon XP-M (Winchester SH7), 754-pin, 90nm");
   XFXMSC(0, 1,  5,  0, dO, "AMD Opteron (Winchester SH7-D0), 940-pin, 90nm");
   XFXMSC(0, 1,  5,  0, dF, "AMD Athlon 64 FX (Winchester SH7-D0), 940-pin, 90nm");
   XFXMS (0, 1,  5,  0,     "AMD Opteron (Winchester SH7-D0) / Athlon 64 FX (Winchester SH7-D0), 940-pin, 90nm");
   XFXMC (0, 1,  5,     dO, "AMD Opteron (Winchester SH7), 940-pin, 90nm");
   XFXMC (0, 1,  5,     dF, "AMD Athlon 64 FX (Winchester SH7), 940-pin, 90nm");
   XFXM  (0, 1,  5,         "AMD Opteron (Winchester SH7) / Athlon 64 FX (Winchester SH7), 940-pin, 90nm");
   XFXMSC(0, 1,  7,  0, dA, "AMD Athlon 64 (Winchester SH7-D0), 939-pin, 90nm");
   XFXMSC(0, 1,  7,  0, dF, "AMD Athlon 64 FX (Winchester SH7-D0), 939-pin, 90nm");
   XFXMS (0, 1,  7,  0,     "AMD Athlon 64 (Winchester SH7-D0) / Athlon 64 FX (Winchester SH7-D0), 939-pin, 90nm");
   XFXMC (0, 1,  7,     dA, "AMD Athlon 64 (Winchester SH7), 939-pin, 90nm");
   XFXMC (0, 1,  7,     dF, "AMD Athlon 64 FX (Winchester SH7), 939-pin, 90nm");
   XFXM  (0, 1,  7,         "AMD Athlon 64 (Winchester SH7) / Athlon 64 FX (Winchester SH7), 939-pin, 90nm");
   XFXMSC(0, 1,  8,  0, dA, "AMD Athlon 64 (Winchester CH-D0), 754-pin, 90nm");
   XFXMSC(0, 1,  8,  0, MA, "AMD mobile Athlon 64 (Oakville CH-D0), 754-pin, 90nm");
   XFXMSC(0, 1,  8,  0, MS, "AMD mobile Sempron (Palermo CH-D0), 754-pin, 90nm");
   XFXMSC(0, 1,  8,  0, MX, "AMD mobile Athlon XP-M (Oakville CH-D0), 754-pin, 90nm");
   XFXMS (0, 1,  8,  0,     "AMD Athlon 64 (Winchester CH-D0) / mobile Athlon 64 (Oakville CH-D0) / mobile Sempron (Palermo CH-D0) / mobile Athlon XP-M (Oakville CH-D0), 754-pin, 90nm");
   XFXMC (0, 1,  8,     dA, "AMD Athlon 64 (Winchester CH), 754-pin, 90nm");
   XFXMC (0, 1,  8,     MA, "AMD mobile Athlon 64 (Winchester CH), 754-pin, 90nm");
   XFXMC (0, 1,  8,     MS, "AMD mobile Sempron (Palermo CH), 754-pin, 90nm");
   XFXMC (0, 1,  8,     MX, "AMD mobile Athlon XP-M (Winchester CH), 754-pin, 90nm");
   XFXM  (0, 1,  8,         "AMD Athlon 64 (Winchester CH) / mobile Athlon 64 (Winchester CH) / mobile Sempron (Palermo CH) / mobile Athlon XP-M (Winchester CH), 754-pin, 90nm");
   XFXMS (0, 1, 11,  0,     "AMD Athlon 64 (Winchester CH-D0), 939-pin, 90nm");
   XFXM  (0, 1, 11,         "AMD Athlon 64 (Winchester CH), 939-pin, 90nm");
   XFXMSC(0, 1, 12,  0, dA, "AMD Athlon 64 (Winchester DH8-D0), 754-pin, 90nm");
   XFXMSC(0, 1, 12,  0, MA, "AMD mobile Athlon 64 (Oakville DH8-D0), 754-pin, 90nm");
   XFXMSC(0, 1, 12,  0, dS, "AMD Sempron (Palermo DH8-D0), 754-pin, 90nm");
   XFXMSC(0, 1, 12,  0, MS, "AMD mobile Sempron (Palermo DH8-D0), 754-pin, 90nm");
   XFXMSC(0, 1, 12,  0, MX, "AMD Athlon XP-M (Winchester DH8-D0), 754-pin, 90nm");
   XFXMS (0, 1, 12,  0,     "AMD Athlon 64 (Winchester DH8-D0) / Sempron (Palermo DH8-D0) / mobile Athlon 64 (Oakville DH8-D0) / mobile Sempron (Palermo DH8-D0) / mobile Athlon XP-M (Winchester DH8-D0), 754-pin, 90nm");
   XFXMC (0, 1, 12,     dA, "AMD Athlon 64 (Winchester DH8), 754-pin, 90nm");
   XFXMC (0, 1, 12,     MA, "AMD mobile Athlon 64 (Winchester DH8), 754-pin, 90nm");
   XFXMC (0, 1, 12,     dS, "AMD Sempron (Palermo DH8), 754-pin, 90nm");
   XFXMC (0, 1, 12,     MS, "AMD mobile Sempron (Palermo DH8), 754-pin, 90nm");
   XFXMC (0, 1, 12,     MX, "AMD Athlon XP-M (Winchester DH8), 754-pin, 90nm");
   XFXM  (0, 1, 12,         "AMD Athlon 64 (Winchester DH8) / Sempron (Palermo DH8) / mobile Athlon 64 (Winchester DH8) / mobile Sempron (Palermo DH8) / mobile Athlon XP-M (Winchester DH8), 754-pin, 90nm");
   XFXMSC(0, 1, 15,  0, dA, "AMD Athlon 64 (Winchester DH8-D0), 939-pin, 90nm");
   XFXMSC(0, 1, 15,  0, dS, "AMD Sempron (Palermo DH8-D0), 939-pin, 90nm");
   XFXMS (0, 1, 15,  0,     "AMD Athlon 64 (Winchester DH8-D0) / Sempron (Palermo DH8-D0), 939-pin, 90nm");
   XFXMC (0, 1, 15,     dA, "AMD Athlon 64 (Winchester DH8), 939-pin, 90nm");
   XFXMC (0, 1, 15,     dS, "AMD Sempron (Palermo DH8), 939-pin, 90nm");
   XFXM  (0, 1, 15,         "AMD Athlon 64 (Winchester DH8) / Sempron (Palermo DH8), 939-pin, 90nm");
   XFXMSC(0, 2,  1,  0, DO, "AMD Dual Core Opteron (Italy JH-E1), 940-pin, 90nm");
   XFXMSC(0, 2,  1,  0, D8, "AMD Dual Core Opteron (Egypt JH-E1), 940-pin, 90nm");
   XFXMS (0, 2,  1,  0,     "AMD Dual Core Opteron (Italy/Egypt JH-E1), 940-pin, 90nm");
   XFXMSC(0, 2,  1,  2, DO, "AMD Dual Core Opteron (Italy JH-E6), 940-pin, 90nm");
   XFXMSC(0, 2,  1,  2, D8, "AMD Dual Core Opteron (Egypt JH-E6), 940-pin, 90nm");
   XFXMS (0, 2,  1,  2,     "AMD Dual Core Opteron (Italy/Egypt JH-E6), 940-pin, 90nm");
   XFXM  (0, 2,  1,         "AMD Dual Core Opteron (Italy/Egypt JH), 940-pin, 90nm");
   XFXMSC(0, 2,  3,  2, DO, "AMD Dual Core Opteron (Denmark JH-E6), 939-pin, 90nm");
   XFXMSC(0, 2,  3,  2, dA, "AMD Athlon 64 X2 (Toledo JH-E6), 939-pin, 90nm");
   XFXMSC(0, 2,  3,  2, dm, "AMD Athlon 64 X2 (Manchester JH-E6), 939-pin, 90nm");
   XFXMS (0, 2,  3,  2,     "AMD Dual Core Opteron (Denmark JH-E6) / Athlon 64 X2 (Toledo JH-E6), 939-pin, 90nm");
   XFXMC (0, 2,  3,     DO, "AMD Dual Core Opteron (Denmark JH), 939-pin, 90nm");
   XFXMC (0, 2,  3,     dA, "AMD Athlon 64 X2 (Toledo JH), 939-pin, 90nm");
   XFXMC (0, 2,  3,     dm, "AMD Athlon 64 X2 (Manchester JH), 939-pin, 90nm");
   XFXM  (0, 2,  3,         "AMD Dual Core Opteron (Denmark JH) / Athlon 64 X2 (Toledo JH / Manchester JH), 939-pin, 90nm");
   XFXMSC(0, 2,  4,  2, MA, "AMD mobile Athlon 64 (Newark SH-E5), 754-pin, 90nm");
   XFXMSC(0, 2,  4,  2, MT, "AMD mobile Turion (Lancaster SH-E5), 754-pin, 90nm");
   XFXMS (0, 2,  4,  2,     "AMD mobile Athlon 64 (Newark SH-E5) / mobile Turion (Lancaster SH-E5), 754-pin, 90nm");
   XFXMC (0, 2,  4,     MA, "AMD mobile Athlon 64 (Newark SH), 754-pin, 90nm");
   XFXMC (0, 2,  4,     MT, "AMD mobile Turion (Lancaster SH), 754-pin, 90nm");
   XFXM  (0, 2,  4,         "AMD mobile Athlon 64 (Newark SH) / mobile Turion (Lancaster SH), 754-pin, 90nm");
   XFXMC (0, 2,  5,     dO, "AMD Opteron (Troy SH-E4), 940-pin, 90nm");
   XFXMC (0, 2,  5,     d8, "AMD Opteron (Athens SH-E4), 940-pin, 90nm");
   XFXM  (0, 2,  5,         "AMD Opteron (Troy/Athens SH-E4), 940-pin, 90nm");
   XFXMSC(0, 2,  7,  1, dO, "AMD Opteron (Venus SH-E4), 939-pin, 90nm");
   XFXMSC(0, 2,  7,  1, dA, "AMD Athlon 64 (San Diego SH-E4), 939-pin, 90nm");
   XFXMSC(0, 2,  7,  1, dF, "AMD Athlon 64 FX (San Diego SH-E4), 939-pin, 90nm");
   XFXMS (0, 2,  7,  1,     "AMD Opteron (Venus SH-E4) / Athlon 64 (San Diego SH-E4) / Athlon 64 FX (San Diego SH-E4), 939-pin, 90nm");
   XFXMC (0, 2,  7,     dO, "AMD Opteron (San Diego SH), 939-pin, 90nm");
   XFXMC (0, 2,  7,     dA, "AMD Athlon 64 (San Diego SH), 939-pin, 90nm");
   XFXMC (0, 2,  7,     dF, "AMD Athlon 64 FX (San Diego SH), 939-pin, 90nm");
   XFXM  (0, 2,  7,         "AMD Opteron (San Diego SH) / Athlon 64 (San Diego SH) / Athlon 64 FX (San Diego SH), 939-pin, 90nm");
   XFXM  (0, 2, 11,         "AMD Athlon 64 X2 (Manchester BH-E4), 939-pin, 90nm");
   XFXMS (0, 2, 12,  0,     "AMD Sempron (Palermo DH-E3), 754-pin, 90nm");
   XFXMSC(0, 2, 12,  2, dS, "AMD Sempron (Palermo DH-E6), 754-pin, 90nm");
   XFXMSC(0, 2, 12,  2, MS, "AMD mobile Sempron (Palermo DH-E6), 754-pin, 90nm");
   XFXMS (0, 2, 12,  2,     "AMD Sempron (Palermo DH-E6) / mobile Sempron (Palermo DH-E6), 754-pin, 90nm");
   XFXMC (0, 2, 12,     dS, "AMD Sempron (Palermo DH), 754-pin, 90nm");
   XFXMC (0, 2, 12,     MS, "AMD mobile Sempron (Palermo DH), 754-pin, 90nm");
   XFXM  (0, 2, 12,         "AMD Sempron (Palermo DH) / mobile Sempron (Palermo DH), 754-pin, 90nm");
   XFXMSC(0, 2, 15,  0, dA, "AMD Athlon 64 (Venice DH-E3), 939-pin, 90nm");
   XFXMSC(0, 2, 15,  0, dS, "AMD Sempron (Palermo DH-E3), 939-pin, 90nm");
   XFXMS (0, 2, 15,  0,     "AMD Athlon 64 (Venice DH-E3) / Sempron (Palermo DH-E3), 939-pin, 90nm");
   XFXMSC(0, 2, 15,  2, dA, "AMD Athlon 64 (Venice DH-E6), 939-pin, 90nm");
   XFXMSC(0, 2, 15,  2, dS, "AMD Sempron (Palermo DH-E6), 939-pin, 90nm");
   XFXMS (0, 2, 15,  2,     "AMD Athlon 64 (Venice DH-E6) / Sempron (Palermo DH-E6), 939-pin, 90nm");
   XFXMC (0, 2, 15,     dA, "AMD Athlon 64 (Venice DH), 939-pin, 90nm");
   XFXMC (0, 2, 15,     dS, "AMD Sempron (Palermo DH), 939-pin, 90nm");
   XFXM  (0, 2, 15,         "AMD Athlon 64 (Venice DH) / Sempron (Palermo DH), 939-pin, 90nm");
   XF    (0,                "AMD Opteron / Athlon 64 / Athlon 64 FX / Sempron / Dual Core Opteron / Athlon 64 X2 / mobile Athlon 64 / mobile Sempron / mobile Athlon XP-M (DP) (unknown model)");
   DEFAULT                ("unknown");

   /*
   ** I can't get any consistent answers on what values are appropriate for the 
   ** following:
   **   Georgetown/Sonora (nearly identical mobile Semprons variants of what?)
   **   Albany/Roma       (nearly identical mobile Semprons variants of what?)
   */

   print_synth_amd_model(stash);
   
   printf("\n");
}

static void
print_synth_cyrix(const char*   name,
                  unsigned int  val)
{
   printf(name);
   START;
   FM (4,  4,     "Cyrix Media GX / GXm");
   FM (4,  9,     "Cyrix 5x86");
   F  (4,         "Cyrix 5x86 (unknown model)");
   FM (5,  2,     "Cyrix M1 6x86");
   FM (5,  4,     "Cyrix M1 WinChip (C6)");
   FM (5,  8,     "Cyrix M1 WinChip 2 (C6-2)");
   FM (5,  9,     "Cyrix M1 WinChip 3 (C6-2)");
   F  (5,         "Cyrix M1 (unknown model)");
   FM (6,  0,     "Cyrix M2 6x86MX");
   FM (6,  5,     "Cyrix M2");
   F  (6,         "Cyrix M2 (unknown model)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_via(const char*   name,
                unsigned int  val)
{
   printf(name);
   START;
   FM (6,  6,     "VIA C3 (Samuel WinChip C5A core)");
   FM (6,  6,     "VIA C3 (Samuel WinChip C5A core)");
   FMS(6,  7,  0, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  1, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  2, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  3, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  4, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  5, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  6, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FMS(6,  7,  7, "VIA C3 (Samuel 2 WinChip C5B core) / Eden ESP 4000/5000/6000");
   FM (6,  7,     "VIA C3 (Ezra WinChip C5C core)");
   FM (6,  8,     "VIA C3 (Ezra-T WinChip C5N core)");
   FMS(6,  9,  0, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  1, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  2, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  3, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  4, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  5, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  6, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FMS(6,  9,  7, "VIA C3 / Eden ESP 7000/8000/10000 (Nehemiah WinChip C5XL core)");
   FM (6,  9,     "VIA C3 / C3-M / Eden-N (Nehemiah WinChip C5P core)");
   FM (6, 10,     "VIA C7 / C7-M (Esther WinChip C5J core)");
   F  (6,         "VIA C3 / C3-M / C7 / C7-M / Eden ESP 7000/8000/10000 (unknown model)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_umc(const char*   name,
                unsigned int  val)
{
   printf(name);
   START;
   FM (4,  1,     "UMC U5D (486DX)");
   FMS(4,  2,  3, "UMC U5S (486SX)");
   FM (4,  2,     "UMC U5S (486SX) (unknown stepping)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_nexgen(const char*   name,
                   unsigned int  val)
{
   printf(name);
   START;
   FMS(5,  0,  4, "NexGen P100");
   FMS(5,  0,  6, "NexGen P120 (E2/C0)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_rise(const char*   name,
                 unsigned int  val)
{
   printf(name);
   START;
   FM (5,  0,     "Rise mP6 iDragon, .25u");
   FM (5,  2,     "Rise mP6 iDragon, .18u");
   FM (5,  8,     "Rise mP6 iDragon II, .25u");
   FM (5,  9,     "Rise mP6 iDragon II, .18u");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_transmeta(const char*   name,
                      unsigned int  val,
                      code_t        cd)
{
   /* TODO: Add code-based detail for Transmeta Crusoe TM5700/TM5900 */
   /* TODO: Add code-based detail for Transmeta Efficeon */
   printf(name);
   START;
   FMSC(5,  4,  2, t2, "Transmeta Crusoe TM3200");
   FMS (5,  4,  2,     "Transmeta Crusoe TM3x00 (unknown model)");
   FMSC(5,  4,  3, t4, "Transmeta Crusoe TM5400");
   FMSC(5,  4,  3, t5, "Transmeta Crusoe TM5500 / Crusoe SE TM55E");
   FMSC(5,  4,  3, t6, "Transmeta Crusoe TM5600");
   FMSC(5,  4,  3, t8, "Transmeta Crusoe TM5800 / Crusoe SE TM58E");
   FMS (5,  4,  3,     "Transmeta Crusoe TM5x00 (unknown model)");
   FM  (5,  4,         "Transmeta Crusoe");
   F   (5,             "Transmeta Crusoe (unknown model)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_x_synth_amd(unsigned int  val)
{
   printf("      (simple synth) = ");
   START;
   FM   (4,     3,     "AMD 80486DX2");
   FM   (4,     7,     "AMD 80486DX2WB");
   FM   (4,     8,     "AMD 80486DX4");
   FM   (4,     9,     "AMD 80486DX4WB");
   FM   (4,    14,     "AMD 5x86");
   FM   (4,    15,     "AMD 5xWB");
   F    (4,            "AMD 80486 / 5x (unknown model)");
   FM   (5,     0,     "AMD SSA5 (PR75, PR90, PR100)");
   FM   (5,     1,     "AMD 5k86 (PR120, PR133)");
   FM   (5,     2,     "AMD 5k86 (PR166)");
   FM   (5,     3,     "AMD 5k86 (PR200)");
   F    (5,            "AMD 5k86 (unknown model)");
   FM   (6,     6,     "AMD K6, .30um");
   FM   (6,     7,     "AMD K6 (Little Foot), .25um");
   FMS  (6,     8,  0, "AMD K6-2 (Chomper A)");
   FMS  (6,     8, 12, "AMD K6-2 (Chomper A)");
   FM   (6,     8,     "AMD K6-2 (Chomper)");
   FMS  (6,     9,  1, "AMD K6-III (Sharptooth B)");
   FM   (6,     9,     "AMD K6-III (Sharptooth)");
   FM   (6,    13,     "AMD K6-2+, K6-III+");
   F    (6,            "AMD K6 (unknown model)");
   FM   (7,     1,     "AMD Athlon, .25um");
   FM   (7,     2,     "AMD Athlon (K75 / Pluto / Orion), .18um");
   FMS  (7,     3,  0, "AMD Duron / mobile Duron (Spitfire A0)");
   FMS  (7,     3,  1, "AMD Duron / mobile Duron (Spitfire A2)");
   FM   (7,     3,     "AMD Duron / mobile Duron (Spitfire)");
   FMS  (7,     4,  2, "AMD Athlon (Thunderbird A4-A7)");
   FMS  (7,     4,  4, "AMD Athlon (Thunderbird A9)");
   FM   (7,     4,     "AMD Athlon (Thunderbird)");
   FMS  (7,     6,  0, "AMD Athlon / Athlon MP mobile Athlon 4 / mobile Duron (Palomino A0)");
   FMS  (7,     6,  1, "AMD Athlon / Athlon MP / Duron / mobile Athlon / mobile Duron (Palomino A2)");
   FMS  (7,     6,  2, "AMD Athlon MP / Athlon XP / Duron / Duron MP / mobile Athlon / mobile Duron (Palomino A5)");
   FM   (7,     6,     "AMD Athlon / Athlon MP / Athlon XP / Duron / Duron MP / mobile Athlon / mobile Duron (Palomino)");
   FMS  (7,     7,  0, "AMD Duron / Duron MP / mobile Duron (Morgan A0)");
   FMS  (7,     7,  1, "AMD Duron / Duron MP / mobile Duron (Morgan A1)");
   FM   (7,     7,     "AMD Duron / Duron MP / mobile Duron (Morgan)");
   FMS  (7,     8,  0, "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred A0)");
   FMS  (7,     8,  1, "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred B0)");
   FM   (7,     8,     "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP (Thoroughbred)");
   FMS  (7,    10,  0, "AMD Athlon XP / Athlon MP / Sempron / mobile Athlon XP-M / mobile Athlon XP-M (LV) (Barton A2)");
   FM   (7,    10,     "AMD Athlon XP / Athlon MP / Sempron / mobile Athlon XP-M / mobile Athlon XP-M (LV) (Barton)");
   F    (7,            "AMD Athlon XP / Athlon MP / Sempron / Duron / Duron MP / mobile Athlon / mobile Athlon XP-M / mobile Athlon XP-M (LV) / mobile Duron (unknown model)");
   XFXMS(0, 0,  4,  0, "AMD Athlon 64 (SledgeHammer SH7-B0), .13um");
   XFXMS(0, 0,  4,  8, "AMD Athlon 64 (SledgeHammer SH7-C0) / mobile Athlon 64 (SledgeHammer SH7-C0) / mobile Athlon XP-M (SledgeHammer SH7-C0), 754-pin, .13um");
   XFXMS(0, 0,  4, 10, "AMD Athlon 64 (SledgeHammer SH7-CG) / mobile Athlon 64 (SledgeHammer SH7-CG) / mobile Athlon XP-M (SledgeHammer SH7-CG), 940-pin, .13um");
   XFXM (0, 0,  4,     "AMD Athlon 64 (SledgeHammer SH7) / mobile Athlon 64 (SledgeHammer SH7) / mobile Athlon XP-M (SledgeHammer SH7), .13um");
   XFXMS(0, 0,  5,  0, "AMD Opteron (DP SledgeHammer SH7-B0), 940-pin, .13um");
   XFXMS(0, 0,  5,  1, "AMD Opteron (DP SledgeHammer SH7-B3), 940-pin, .13um");
   XFXMS(0, 0,  5,  8, "AMD Opteron (DP SledgeHammer SH7-C0) / Athlon 64 FX (DP SledgeHammer SH7-C0), 940-pin, .13um");
   XFXMS(0, 0,  5, 10, "AMD Opteron (DP SledgeHammer SH7-CG) / Athlon 64 FX (DP SledgeHammer SH7-CG), 940-pin, .13um");
   XFXM (0, 0,  5,     "AMD Opteron (SledgeHammer SH7) / Athlon 64 (SledgeHammer SH7) FX, 940-pin, .13um");
   XFXMS(0, 0,  7, 10, "AMD Athlon 64 (DP SledgeHammer SH7-CG) / Athlon 64 FX (DP SledgeHammer SH7-CG), 939-pin, .13um");
   XFXM (0, 0,  7,     "AMD Athlon 64 (DP SledgeHammer SH7) / Athlon 64 FX (DP SledgeHammer SH7), 939-pin, .13um");
   XFXMS(0, 0,  8,  2, "AMD Athlon 64 (ClawHammer CH7-CG) / mobile Athlon 64 (Odessa CH7-CG) / mobile Sempron (ClawHammer CH7-CG) / mobile Athlon XP-M (ClawHammer CH7-CG), 754-pin, .13um");
   XFXM (0, 0,  8,     "AMD Athlon 64 (ClawHammer CH7) / mobile Athlon 64 (Odessa CH7) / mobile Sempron (Odessa CH7) / mobile Athlon XP-M (Odessa CH7), 754-pin, .13um");
   XFXMS(0, 0, 11,  2, "AMD Athlon 64 (ClawHammer CH7-CG), 939-pin, .13um");
   XFXM (0, 0, 11,     "AMD Athlon 64 (ClawHammer CH7), 939-pin, .13um");
   XFXMS(0, 0, 12,  0, "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG) / mobile Sempron (Sonora DH7-CG) / mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXM (0, 0, 12,     "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7) / mobile Sempron (Sonora DH7) / mobile Athlon XP-M (Dublin DH7), 754-pin, .13um");
   XFXMS(0, 0, 14,  0, "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG) / mobile Sempron (Sonora DH7-CG) / mobile Athlon XP-M (Dublin DH7-CG), 754-pin, .13um");
   XFXM (0, 0, 14,     "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7) / mobile Sempron (Sonora DH7) / mobile Athlon XP-M (Dublin DH7), 754-pin, .13um");
   XFXMS(0, 0, 15,  0, "AMD Athlon 64 (NewCastle DH7-CG) / Sempron (Paris DH7-CG) / mobile Athlon 64 (Dublin DH7-CG), 939-pin, .13um");
   XFXM (0, 0, 15,     "AMD Athlon 64 (NewCastle DH7) / Sempron (Paris DH7) / mobile Athlon 64 (Dublin DH7), 939-pin, .13um");
   XFXMS(0, 1,  4,  0, "AMD Athlon 64 (Winchester SH7-D0) / mobile Athlon 64 (Oakville SH7-D0) / mobile Athlon XP-M (Oakville SH7-D0), 754-pin, 90nm");
   XFXM (0, 1,  4,     "AMD Athlon 64 (Winchester SH7) / mobile Athlon 64 (Winchester SH7) / mobile Athlon XP-M (Winchester SH7), 754-pin, 90nm");
   XFXMS(0, 1,  5,  0, "AMD Opteron (Winchester SH7-D0) / Athlon 64 FX (Winchester SH7-D0), 940-pin, 90nm");
   XFXM (0, 1,  5,     "AMD Opteron (Winchester SH7) / Athlon 64 FX (Winchester SH7), 940-pin, 90nm");
   XFXMS(0, 1,  7,  0, "AMD Athlon 64 (Winchester SH7-D0) / Athlon 64 FX (Winchester SH7-D0), 939-pin, 90nm");
   XFXM (0, 1,  7,     "AMD Athlon 64 (Winchester SH7) / Athlon 64 FX (Winchester SH7), 939-pin, 90nm");
   XFXMS(0, 1,  8,  0, "AMD Athlon 64 (Winchester CH-D0) / mobile Athlon 64 (Oakville CH-D0) / mobile Sempron (Palermo CH-D0) / mobile Athlon XP-M (Oakville CH-D0), 754-pin, 90nm");
   XFXM (0, 1,  8,     "AMD Athlon 64 (Winchester CH) / mobile Athlon 64 (Winchester CH) / mobile Sempron (Palermo CH) / mobile Athlon XP-M (Winchester CH), 754-pin, 90nm");
   XFXMS(0, 1, 11,  0, "AMD Athlon 64 (Winchester CH-D0), 939-pin, 90nm");
   XFXM (0, 1, 11,     "AMD Athlon 64 (Winchester CH), 939-pin, 90nm");
   XFXMS(0, 1, 12,  0, "AMD Athlon 64 (Winchester DH8-D0) / Sempron (Palermo DH8-D0) / mobile Athlon 64 (Oakville DH8-D0) / mobile Sempron (Palermo DH8-D0) / mobile Athlon XP-M (Winchester DH8-D0), 754-pin, 90nm");
   XFXM (0, 1, 12,     "AMD Athlon 64 (Winchester DH8) / Sempron (Palermo DH8) / mobile Athlon 64 (Winchester DH8) / mobile Sempron (Palermo DH8) / mobile Athlon XP-M (Winchester DH8), 754-pin, 90nm");
   XFXMS(0, 1, 15,  0, "AMD Athlon 64 (Winchester DH8-D0) / Sempron (Palermo DH8-D0), 939-pin, 90nm");
   XFXM (0, 1, 15,     "AMD Athlon 64 (Winchester DH8) / Sempron (Palermo DH8), 939-pin, 90nm");
   XFXMS(0, 2,  1,  0, "AMD Dual Core Opteron (Italy/Egypt JH-E1), 940-pin, 90nm");
   XFXMS(0, 2,  1,  2, "AMD Dual Core Opteron (Italy/Egypt JH-E6), 940-pin, 90nm");
   XFXM (0, 2,  1,     "AMD Dual Core Opteron (Italy/Egypt JH), 940-pin, 90nm");
   XFXMS(0, 2,  3,  2, "AMD Dual Core Opteron (Denmark JH-E6) / Athlon 64 X2 (Toledo JH-E6 / Manchester JH-E6), 939-pin, 90nm");
   XFXM (0, 2,  3,     "AMD Dual Core Opteron (Denmark JH) / Athlon 64 X2 (Toledo JH / Manchester JH), 939-pin, 90nm");
   XFXMS(0, 2,  4,  2, "AMD mobile Athlon 64 (Newark SH-E5) / mobile Turion (Lancaster SH-E5), 754-pin, 90nm");
   XFXM (0, 2,  4,     "AMD mobile Athlon 64 (Newark SH) / mobile Turion (Lancaster SH), 754-pin, 90nm");
   XFXM (0, 2,  5,     "AMD Opteron (Troy/Athens SH-E4), 940-pin, 90nm");
   XFXMS(0, 2,  7,  1, "AMD Opteron (Troy/Athens SH-E4) / Opteron (Venus SH-E4) / Athlon 64 (San Diego SH-E4) / Athlon 64 FX (San Diego SH-E4), 939-pin, 90nm");
   XFXM (0, 2,  7,     "AMD Opteron (San Diego SH) / Athlon 64 (San Diego SH) / Athlon 64 FX (San Diego SH), 939-pin, 90nm");
   XFXM (0, 2, 11,     "AMD Athlon 64 X2 (Manchester BH-E4), 939-pin, 90nm");
   XFXMS(0, 2, 12,  0, "AMD Sempron (Palermo DH-E3), 754-pin, 90nm");
   XFXMS(0, 2, 12,  2, "AMD Sempron (Venice DH-E6) / mobile Sempron (Palermo DH-E6), 754-pin, 90nm");
   XFXM (0, 2, 12,     "AMD Sempron (Venice DH) / mobile Sempron (Palermo DH), 754-pin, 90nm");
   XFXMS(0, 2, 15,  0, "AMD Athlon 64 (Venice DH-E3) / Sempron (Palermo DH-E3), 939-pin, 90nm");
   XFXMS(0, 2, 15,  2, "AMD Athlon 64 (Toledo DH-E6) / Sempron (Palermo DH-E6), 939-pin, 90nm");
   XFXM (0, 2, 15,     "AMD Athlon 64 (Toledo DH) / Sempron (Palermo DH), 939-pin, 90nm");
   XF   (0,            "AMD Opteron / Athlon 64 / Sempron / Turion / Athlon 64 FX / Athlon XP-M / mobile Athlon 64 / mobile Sempron / mobile Athlon XP-M (unknown)");
   DEFAULT           ("unknown");
   printf("\n");
}

static void
print_x_synth_via(unsigned int  val)
{
   printf("      (simple synth) = ");
   START;
   FM (6,  6,     "VIA C3 (WinChip C5A)");
   FM (6,  6,     "VIA C3 (WinChip C5A)");
   FMS(6,  7,  0, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  1, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  2, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  3, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  4, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  5, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  6, "VIA C3 (WinChip C5B)");
   FMS(6,  7,  7, "VIA C3 (WinChip C5B)");
   FM (6,  7,     "VIA C3 (WinChip C5C)");
   FM (6,  8,     "VIA C3 (WinChip C5N)");
   FM (6,  9,     "VIA C3 (WinChip C5XL)");
   F  (6,         "VIA C3 (unknown model)");
   DEFAULT       ("unknown");
   printf("\n");
}

static void
print_synth_simple(unsigned int  val_eax,
                   vendor_t      vendor)
{
   switch (vendor) {
   case VENDOR_INTEL:
      print_synth_intel("      (simple synth)  = ", val_eax, UN);
      break;
   case VENDOR_AMD:
      print_synth_amd("      (simple synth)  = ", val_eax, UN, NULL);
      break;
   case VENDOR_CYRIX:
      print_synth_cyrix("      (simple synth)  = ", val_eax);
      break;
   case VENDOR_VIA:
      print_synth_via("      (simple synth)  = ", val_eax);
      break;
   case VENDOR_TRANSMETA:
      print_synth_transmeta("      (simple synth)  = ", val_eax, UN);
      break;
   case VENDOR_UMC:
      print_synth_umc("      (simple synth)  = ", val_eax);
      break;
   case VENDOR_NEXGEN:
      print_synth_nexgen("      (simple synth)  = ", val_eax);
      break;
   case VENDOR_RISE:
      print_synth_rise("      (simple synth)  = ", val_eax);
      break;
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

static void
print_synth(const code_stash_t*  stash)
{
   code_t  cd = decode(stash);

   switch (stash->vendor) {
   case VENDOR_INTEL:
      print_synth_intel("   (synth) = ", stash->val_1_eax, cd);
      break;
   case VENDOR_AMD:
      print_synth_amd("   (synth) = ", stash->val_1_eax, cd, stash);
      break;
   case VENDOR_CYRIX:
      print_synth_cyrix("   (synth) = ", stash->val_1_eax);
      break;
   case VENDOR_VIA:
      print_synth_via("   (synth) = ", stash->val_1_eax);
      break;
   case VENDOR_TRANSMETA:
      print_synth_transmeta("      (simple synth)  = ", stash->val_1_eax, cd);
      break;
   case VENDOR_UMC:
      print_synth_umc("      (simple synth)  = ", stash->val_1_eax);
      break;
   case VENDOR_NEXGEN:
      print_synth_nexgen("      (simple synth)  = ", stash->val_1_eax);
      break;
   case VENDOR_RISE:
      print_synth_rise("      (simple synth)  = ", stash->val_1_eax);
      break;
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

#define GET_LogicalProcessorCount(val_1_ebx) \
   (BIT_EXTRACT_LE((val_1_ebx), 16, 24))
#define IS_HTT(val_1_edx) \
   (BIT_EXTRACT_LE((val_1_edx), 28, 29))
#define IS_CmpLegacy(val_80000001_ecx) \
   (BIT_EXTRACT_LE((val_80000001_ecx), 1, 2))
#define GET_NC_INTEL(val_4_eax) \
   (BIT_EXTRACT_LE((val_4_eax), 26, 32))
#define GET_NC_AMD(val_80000008_ecx) \
   (BIT_EXTRACT_LE((val_80000008_ecx), 0, 8))

static void decode_mp_synth(code_stash_t*  stash)
{
   /*
   ** This logic based on the AMD CPUID Specification (25481).
   ** NOTE: The AMD CPUID documentation says that c always will be 0
   **       on a single-core machine.  But Intel doesn't follow that
   **       convention and uses 1 for a single-core machine.  So, cope.
   */
   if (IS_HTT(stash->val_1_edx)) {
      unsigned int  tc = GET_LogicalProcessorCount(stash->val_1_ebx);
      unsigned int  c;
      switch (stash->vendor) {
      case VENDOR_INTEL:
         c = GET_NC_INTEL(stash->val_4_eax) + 1;
         stash->mp.ok = TRUE;
         break;
      case VENDOR_AMD:
         c = GET_NC_AMD(stash->val_80000008_ecx) + 1;
         stash->mp.ok = (tc == c) == IS_CmpLegacy(stash->val_80000001_ecx);
         break;
      default:
         c = 0;
         stash->mp.ok = FALSE;
         break;
      }
      if (!stash->mp.ok) {
         stash->mp.cores        = -1;
         stash->mp.hyperthreads = -1;
         // DO NOTHING
      } else if (c > 1) {
         if (tc == c) {
            stash->mp.cores        = c;
            stash->mp.hyperthreads = 1;
         } else {
            stash->mp.cores        = c;
            stash->mp.hyperthreads = tc / c;
         }
      } else {
         stash->mp.cores        = 1;
         stash->mp.hyperthreads = (tc >= 2 ? tc : 2);
      }
   } else {
      stash->mp.ok           = TRUE;
      stash->mp.cores        = 1;
      stash->mp.hyperthreads = 1;
   }
}

static void print_mp_synth(const struct mp*  mp)
{
   printf("   (multi-processing synth): ");
   if (!mp->ok) {
      printf("?");
   } else if (mp->cores > 1) {
      if (mp->hyperthreads > 1) {
         printf("multi-core (c=%u), hyper-threaded (t=%u)", 
                mp->cores, mp->hyperthreads);
      } else {
         printf("multi-core (c=%u)", mp->cores);
      }
   } else if (mp->hyperthreads > 1) {
      printf("hyper-threaded (t=%u)", mp->hyperthreads);
   } else {
      printf("none");
   }
   printf("\n");
}

static void
print_1_eax(unsigned int  value,
            vendor_t      vendor)
{
   static ccstring  processor[] = { "primary processor (0)",
                                    "Intel OverDrive (1)",
                                    "secondary processor (2)",
                                    NULL };
   static ccstring  family[]    = { NULL,
                                    NULL,
                                    NULL,
                                    "Intel 80386 (3)",
                                    "Intel 80486, AMD Am486, UMC 486 U5 (4)",
                                    "Intel Pentium, AMD K5/K6,"
                                       " Cyrix M1, NexGen Nx586,"
                                       " Centaur C6, Rise mP6,"
                                       " Transmeta Crusoe (5)", 
                                    "Intel Pentium Pro/II/III/Celeron,"
                                       " AMD Athlon/Duron, Cyrix M2,"
                                       " VIA C3 (6)",
                                    "Intel Itanium,"
                                       " AMD Athlon 64/Opteron/Sempron/Turion"
                                       " (7)",
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    "Intel Pentium 4/Pentium D/"
                                    "Pentium Extreme Edition/Celeron/Xeon/"
                                    "Xeon MP/Itanium2,"
                                    " AMD Athlon 64/Athlon XP-M/Opteron/"
                                    "Sempron/Turion (15)" };
   static named_item  names[]
      = { { "processor type"                          , 12, 13, processor },
          { "family"                                  ,  8, 11, family },
          { "model"                                   ,  4,  7, NIL_IMAGES },
          { "stepping id"                             ,  0,  3, NIL_IMAGES },
          { "extended family"                         , 20, 27, NIL_IMAGES },
          { "extended model"                          , 16, 19, NIL_IMAGES },
        };

   printf("   version information (1/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   print_synth_simple(value, vendor);
}

#define B(b,str)                                             \
   else if (   __B(val_ebx)    == _B(b))                     \
      printf(str)
#define FMB(f,m,b,str)                                       \
   else if (   __FM(val_eax)   ==         _F(f) +_M(m)       \
            && __B(val_ebx)    == _B(b))                     \
      printf(str)
#define FMSB(f,m,s,b,str)                                    \
   else if (   __FMS(val_eax)  ==         _F(f) +_M(m)+_S(s) \
            && __B(val_ebx)    == _B(b))                     \
      printf(str)
#define XFMB(xf,m,b,str)                                     \
   else if (   __XFM(val_eax)  == _XF(xf)+_F(15)+_M(m)       \
            && __B(val_ebx)    == _B(b))                     \
      printf(str)
#define XFMSB(xf,m,s,b,str)                                  \
   else if (   __XFMS(val_eax) == _XF(xf)+_F(15)+_M(m)+_S(s) \
            && __B(val_ebx)    == _B(b))                     \
      printf(str)

static void
print_brand(unsigned int  val_eax,
            unsigned int  val_ebx)
{
   printf("   brand id = 0x%02x (%u): ", __B(val_ebx), __B(val_ebx));
   START;
   B    (           1, "Intel Celeron, .18um");
   B    (           2, "Intel Pentium III, .18um");
   FMSB (6, 11, 1,  3, "Intel Celeron, .13um");
   B    (           3, "Intel Pentium III Xeon, .18um");
   B    (           4, "Intel Pentium III, .13um");
   B    (           6, "Mobile Intel Pentium III, .13um");
   B    (           7, "Mobile Intel Celeron, .13um");
   XFMB (0,  0,     8, "Intel Pentium 4, .18um");
   XFMSB(0,  1, 0,  8, "Intel Pentium 4, .18um");
   XFMSB(0,  1, 1,  8, "Intel Pentium 4, .18um");
   XFMSB(0,  1, 2,  8, "Intel Pentium 4, .18um");
   B    (           8, "Mobile Intel Celeron 4, .13um");
   B    (           9, "Intel Pentium 4, .13um");
   B    (          10, "Intel Celeron 4, .18um");
   XFMB (0,  0,    11, "Intel Xeon MP, .18um");
   XFMSB(0,  1, 0, 11, "Intel Xeon MP, .18um");
   XFMSB(0,  1, 1, 11, "Intel Xeon MP, .18um");
   XFMSB(0,  1, 2, 11, "Intel Xeon MP, .18um");
   B    (          11, "Intel Xeon, .13um");
   B    (          12, "Intel Xeon MP, .13um");
   XFMB (0,  0,    14, "Intel Xeon, .18um");
   XFMSB(0,  1, 0, 14, "Intel Xeon, .18um");
   XFMSB(0,  1, 1, 14, "Intel Xeon, .18um");
   XFMSB(0,  1, 2, 14, "Intel Xeon, .18um");
   XFMB (0,  2,    14, "Mobile Intel Pentium 4 Processor-M");
   B    (          14, "Mobile Intel Xeon, .13um");
   XFMB (0,  2,    15, "Mobile Intel Pentium 4 Processor-M");
   B    (          15, "Mobile Intel Celeron 4");
   B    (          17, "Mobile Genuine Intel");
   B    (          18, "Intel Celeron M");
   B    (          19, "Mobile Intel Celeron");
   B    (          20, "Intel Celeron");
   B    (          21, "Mobile Genuine Intel");
   B    (          22, "Intel Pentium M, .13um");
   B    (          23, "Mobile Intel Celeron");
   DEFAULT            ("unknown");
   printf("\n");
}

static void
print_1_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "process local APIC physical ID"          , 24, 31, NIL_IMAGES },
          { "cpu count"                               , 16, 23, NIL_IMAGES },
          { "CLFLUSH line size"                       ,  8, 15, NIL_IMAGES },
          { "brand index"                             ,  0,  7, NIL_IMAGES },
        };

   printf("   miscellaneous (1/ebx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_1_ecx(unsigned int  value)
{
   static named_item  names[]
      = { { "PNI/SSE3: Prescott New Instructions"     ,  0,  0, bools },
          { "MONITOR/MWAIT"                           ,  3,  3, bools },
          { "CPL-qualified debug store"               ,  4,  4, bools },
          { "VMX: virtual machine extensions"         ,  5,  5, bools },
          { "Enhanced Intel SpeedStep Technology"     ,  7,  7, bools },
          { "thermal monitor 2"                       ,  8,  8, bools },
          { "context ID: adaptive or shared L1 data"  , 10, 10, bools },
          { "cmpxchg16b available"                    , 13, 13, bools },
          { "xTPR disable"                            , 14, 14, bools },
        };

   printf("   feature information (1/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_1_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "x87 FPU on chip"                         ,  0,  0, bools },
          { "virtual-8086 mode enhancement"           ,  1,  1, bools },
          { "debugging extensions"                    ,  2,  2, bools },
          { "page size extensions"                    ,  3,  3, bools },
          { "time stamp counter"                      ,  4,  4, bools },
          { "RDMSR and WRMSR support"                 ,  5,  5, bools },
          { "physical address extensions"             ,  6,  6, bools },
          { "machine check exception"                 ,  7,  7, bools },
          { "CMPXCHG8B inst."                         ,  8,  8, bools },
          { "APIC on chip"                            ,  9,  9, bools },
          { "SYSENTER and SYSEXIT"                    , 11, 11, bools },
          { "memory type range registers"             , 12, 12, bools },
          { "PTE global bit"                          , 13, 13, bools },
          { "machine check architecture"              , 14, 14, bools },
          { "conditional move/compare instruction"    , 15, 15, bools },
          { "page attribute table"                    , 16, 16, bools },
          { "page size extension"                     , 17, 17, bools },
          { "processor serial number"                 , 18, 18, bools },
          { "CLFLUSH instruction"                     , 19, 19, bools },
          { "debug store"                             , 21, 21, bools },
          { "thermal monitor and clock ctrl"          , 22, 22, bools },
          { "MMX Technology"                          , 23, 23, bools },
          { "FXSAVE/FXRSTOR"                          , 24, 24, bools },
          { "SSE extensions"                          , 25, 25, bools },
          { "SSE2 extensions"                         , 26, 26, bools },
          { "self snoop"                              , 27, 27, bools },
          { "hyper-threading / multi-core supported"  , 28, 28, bools },
          { "therm. monitor"                          , 29, 29, bools },
          { "IA64"                                    , 30, 30, bools },
          { "pending break event"                     , 31, 31, bools },
        };

   printf("   feature information (1/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void print_2_meaning(unsigned char  value,
                            vendor_t       vendor,
                            unsigned int   val_1_eax)
{
   if (vendor == VENDOR_CYRIX || vendor == VENDOR_VIA) {
      switch (value) {
      case 0x70: printf("TLB: 4k pages, 4-way, 32 entries");    return;
      case 0x74: printf("Cyrix-specific: ?");                   return;
      case 0x77: printf("Cyrix-specific: ?");                   return;
      case 0x80: printf("L1 cache: 16K, 4-way, 16 byte lines"); return;
      case 0x82: printf("Cyrix-specific: ?");                   return;
      case 0x84: printf("L2 cache: 1M, 8-way, 32 byte lines");  return;
      }
   }

   switch (value) {
   case 0x01: printf("instruction TLB: 4K pages, 4-way, 32 entries");    break;
   case 0x02: printf("instruction TLB: 4M pages, 4-way, 2 entries");     break;
   case 0x03: printf("data TLB: 4K pages, 4-way, 64 entries");           break;
   case 0x04: printf("data TLB: 4M pages, 4-way, 8 entries");            break;
   case 0x06: printf("L1 instruction cache: 8K, 4-way, 32 byte lines");  break;
   case 0x08: printf("L1 instruction cache: 16K, 4-way, 32 byte lines"); break;
   case 0x0a: printf("L1 data cache: 8K, 2-way, 32 byte lines");         break;
   case 0x0c: printf("L1 data cache: 16K, 4-way, 32 byte lines");        break;
   case 0x10: printf("L1 data cache: 16K, 4-way, 32 byte lines");        break;
   case 0x15: printf("L1 instruction cache: 16K, 4-way, 32 byte lines"); break;
   case 0x1a: printf("L2 cache: 96K, 6-way, 64 byte lines");             break;
   case 0x22: printf("L3 cache: 512K, 4-way, 64 byte lines");            break;
   case 0x23: printf("L3 cache: 1M, 8-way, 64 byte lines");              break;
   case 0x25: printf("L3 cache: 2M, 8-way, 64 byte lines");              break;
   case 0x29: printf("L3 cache: 4M, 8-way, 64 byte lines");              break;
   case 0x2c: printf("L1 data cache: 32K, 8-way, 64 byte lines");        break;
   case 0x30: printf("L1 cache: 32K, 8-way, 64 byte lines");             break;
   case 0x39: printf("L2 cache: 128K, 4-way, sectored, 64 byte lines");  break;
   case 0x3a: printf("L2 cache: 192K, 6-way, sectored, 64 byte lines");  break;
   case 0x3b: printf("L2 cache: 128K, 2-way, sectored, 64 byte lines");  break;
   case 0x3c: printf("L2 cache: 256K, 4-way, sectored, 64 byte lines");  break;
   case 0x3d: printf("L2 cache: 384K, 6-way, sectored, 64 byte lines");  break;
   case 0x3e: printf("L2 cache: 512K, 4-way, sectored, 64 byte lines");  break;
   case 0x40: if (__F(val_1_eax) <= _F(6)) {
                 printf("No L2 cache");
              } else {
                 printf("No L3 cache");
              }
              break;
   case 0x41: printf("L2 cache: 128K, 4-way, 32 byte lines");            break;
   case 0x42: printf("L2 cache: 256K, 4-way, 32 byte lines");            break;
   case 0x43: printf("L2 cache: 512K, 4-way, 32 byte lines");            break;
   case 0x44: printf("L2 cache: 1M, 4-way, 32 byte lines");              break;
   case 0x45: printf("L2 cache: 2M, 4-way, 32 byte lines");              break;
   case 0x46: printf("L3 cache: 4M, 4-way, 64 byte lines");              break;
   case 0x47: printf("L3 cache: 8M, 8-way, 64 byte lines");              break;
   case 0x49: printf("L3 cache: 4M, 16-way, 64 byte lines");             break;
   case 0x4a: printf("L3 cache: 6M, 12-way, 64 byte lines");             break;
   case 0x4b: printf("L3 cache: 8M, 16-way, 64 byte lines");             break;
   case 0x4c: printf("L3 cache: 12M, 12-way, 64 byte lines");            break;
   case 0x4d: printf("L3 cache: 16M, 16-way, 64 byte lines");            break;
   case 0x50: printf("instruction TLB: 4K & 2M/4M pages, 64 entries");   break;
   case 0x51: printf("instruction TLB: 4K & 2M/4M pages, 128 entries");  break;
   case 0x52: printf("instruction TLB: 4K & 2M/4M pages, 256 entries");  break;
   case 0x5b: printf("data TLB: 4K & 4M pages, 64 entries");             break;
   case 0x5c: printf("data TLB: 4K & 4M pages, 128 entries");            break;
   case 0x5d: printf("data TLB: 4K & 4M pages, 256 entries");            break;
   case 0x60: printf("L1 data cache: 16K, 8-way, 64 byte lines");        break;
   case 0x66: printf("L1 data cache: 8K, 4-way, 64 byte lines");         break;
   case 0x67: printf("L1 data cache: 16K, 4-way, 64 byte lines");        break;
   case 0x68: printf("L1 data cache: 32K, 4-way, 64 byte lines");        break;
   case 0x70: printf("Trace cache: 12K-uop, 8-way");                     break;
   case 0x71: printf("Trace cache: 16K-uop, 8-way");                     break;
   case 0x72: printf("Trace cache: 32K-uop, 8-way");                     break;
   case 0x73: printf("Trace cache: 64K-uop, 8-way");                     break;
   case 0x77: printf("L1 instruction cache: 16K, 4-way, sectored,"
                     " 64 byte lines");                                  break;
   case 0x78: printf("L2 cache: 1M, 4-way, 64 byte lines");              break;
   case 0x79: printf("L2 cache: 128K, 8-way, sectored, 64 byte lines");  break;
   case 0x7a: printf("L2 cache: 256K, 8-way, sectored, 64 byte lines");  break;
   case 0x7b: printf("L2 cache: 512K, 8-way, sectored, 64 byte lines");  break;
   case 0x7c: printf("L2 cache: 1M, 8-way, sectored, 64 byte lines");    break;
   case 0x7d: printf("L2 cache: 2M, 8-way, sectored, 64 byte lines");    break;
   case 0x7e: printf("L2 cache: 256K, 8-way, sectored, 128 byte lines"); break;
   case 0x7f: printf("L2 cache: 512K, 2-way, 64 byte lines");            break;
   case 0x81: printf("L2 cache: 128K, 8-way, 32 byte lines");            break;
   case 0x82: printf("L2 cache: 256K, 8-way, 32 byte lines");            break;
   case 0x83: printf("L2 cache: 512K, 8-way, 32 byte lines");            break;
   case 0x84: printf("L2 cache: 1M, 8-way, 32 byte lines");              break;
   case 0x85: printf("L2 cache: 2M, 8-way, 32 byte lines");              break;
   case 0x86: printf("L2 cache: 512K, 4-way, 64 byte lines");            break;
   case 0x87: printf("L2 cache: 1M, 8-way, 64 byte lines");              break;
   case 0x88: printf("L3 cache: 2M, 4-way, 64 byte lines");              break;
   case 0x89: printf("L3 cache: 4M, 4-way, 64 byte lines");              break;
   case 0x8a: printf("L3 cache: 8M, 4-way, 64 byte lines");              break;
   case 0x8d: printf("L3 cache: 3M, 12-way, 128 byte lines");            break;
   case 0x90: printf("instruction TLB: 4K-256M, fully, 64 entries");     break;
   case 0x96: printf("instruction TLB: 4K-256M, fully, 32 entries");     break;
   case 0x9b: printf("instruction TLB: 4K-256M, fully, 96 entries");     break;
   case 0xb0: printf("instruction TLB: 4K, 4-way, 128 entries");         break;
   case 0xb3: printf("data TLB: 4K, 4-way, 128 entries");                break;
   case 0xf0: printf("64 byte prefetching");                             break;
   case 0xf1: printf("128 byte prefetching");                            break;
   default:   printf("unknown");                                         break;
   }
}

static void print_2_byte(unsigned char  value,
                         vendor_t       vendor,
                         unsigned int   val_1_eax)
{
   if (value == 0x00) return;

   printf("      0x%02x: ", value);
   print_2_meaning(value, vendor, val_1_eax);
   printf("\n");
}

static void
print_4_eax(unsigned int  value)
{
   static ccstring  cache_type[] = { "no more caches (0)",
                                     "data cache (1)",
                                     "instruction cache (2)",
                                     "unified cache (3)" };
   static named_item  names[]
      = { { "cache type"                              ,  0,  4, cache_type },
          { "cache level"                             ,  5,  7, NIL_IMAGES },
          { "self-initializing cache level"           ,  8,  8, bools },
          { "fully associative cache"                 ,  9,  9, bools },
          { "extra threads sharing this cache"        , 14, 25, NIL_IMAGES },
          { "extra processor cores on this die"       , 26, 31, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 33);
}

static void
print_4_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "system coherency line size"              ,  0, 11, NIL_IMAGES },
          { "physical line partitions"                , 12, 21, NIL_IMAGES },
          { "ways of associativity"                   , 22, 31, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 33);
}

static void
print_5_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "smallest monitor-line size (bytes)"      ,  0, 15, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_5_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "largest monitor-line size (bytes)"       ,  0, 15, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_5_ecx(unsigned int  value)
{
   static named_item  names[]
      = { { "enum of Monitor-MWAIT exts supported"    ,  0,  0, bools },
          { "supports intrs as break-event for MWAIT" ,  1,  1, bools },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_5_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "number of C0 sub C-states using MWAIT"   ,  0,  3, NIL_IMAGES },
          { "number of C1 sub C-states using MWAIT"   ,  4,  7, NIL_IMAGES },
          { "number of C2 sub C-states using MWAIT"   ,  8, 11, NIL_IMAGES },
          { "number of C3 sub C-states using MWAIT"   , 12, 15, NIL_IMAGES },
          { "number of C4 sub C-states using MWAIT"   , 16, 19, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_6_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "digital thermometer"                     ,  0,  0, bools },
          { "operating point protection"              ,  2,  2, bools },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_6_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "digital thermometer thresholds"          ,  0,  3, NIL_IMAGES },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_6_ecx(unsigned int  value)
{
   static named_item  names[]
      = { { "ACNT/MCNT supported performance measure" ,  0,  0, bools },
        };

   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 39);
}

static void
print_a_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "version ID"                              ,  0,  7, NIL_IMAGES },
          { "number of counters per logical processor",  8, 15, NIL_IMAGES },
          { "bit width of counter"                    , 16, 23, NIL_IMAGES },
          { "length of EBX bit vector"                , 24, 31, NIL_IMAGES },
        };

   printf("   Architecture Performance Monitoring Features (0xa/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_a_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "core cycle event not available"          ,  0,  0, bools },
          { "instruction retired event not available" ,  1,  1, bools },
          { "reference cycles event not available"    ,  2,  2, bools },
          { "last-level cache ref event not available",  3,  3, bools },
          { "last-level cache miss event not avail"   ,  4,  4, bools },
          { "branch inst retired event not available" ,  5,  5, bools },
          { "branch mispred retired event not avail"  ,  6,  6, bools },
        };

   printf("   Architecture Performance Monitoring Features (0xa/ebx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_eax_amd(unsigned int  value)
{
   static ccstring  family[]   = { NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   "AMD Am486 (4)",
                                   "AMD K5 (5)",
                                   "AMD K6 (6)",
                                   "AMD Athlon/Duron (7)",
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   "AMD Athlon 64/Opteron/Sempron/Turion (15)" };
   static named_item  names[]
      = { { "generation    "                          ,  8, 11, family },
          { "model         "                          ,  4,  7, NIL_IMAGES },
          { "stepping      "                          ,  0,  3, NIL_IMAGES },
        };

   printf("   extended processor signature (0x80000001/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   print_x_synth_amd(value);
}

static void
print_80000001_eax_via(unsigned int  value)
{
   static ccstring  family[]   = { NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   "VIA C3" };
   static named_item  names[]
      = { { "generation    "                          ,  8, 11, family },
          { "model         "                          ,  4,  7, NIL_IMAGES },
          { "stepping      "                          ,  0,  3, NIL_IMAGES },
        };

   printf("   extended processor signature (0x80000001/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   print_x_synth_via(value);
}

static void
print_80000001_eax_transmeta(unsigned int  value)
{
   static ccstring  family[]   = { NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   "Transmeta Crusoe" };
   static named_item  names[]
      = { { "generation    "                          ,  8, 11, family },
          { "model         "                          ,  4,  7, NIL_IMAGES },
          { "stepping      "                          ,  0,  3, NIL_IMAGES },
        };

   printf("   extended processor signature (0x80000001/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   print_synth_transmeta("      (simple synth)", value, UN);
}

static void
print_80000001_eax(unsigned int  value,
                   vendor_t      vendor)
{
   switch (vendor) {
   case VENDOR_AMD:
      print_80000001_eax_amd(value);
      break;
   case VENDOR_VIA:
      print_80000001_eax_via(value);
      break;
   case VENDOR_TRANSMETA:
      print_80000001_eax_transmeta(value);
      break;
   case VENDOR_INTEL:
   case VENDOR_CYRIX:
   case VENDOR_UMC:
   case VENDOR_NEXGEN:
   case VENDOR_RISE:
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

static void
print_80000001_edx_intel(unsigned int  value)
{
   static named_item  names[]
      = { { "SYSCALL and SYSRET instructions"         , 11, 11, bools },
          { "execution disable"                       , 20, 20, bools },
          { "64-bit extensions technology available"  , 29, 29, bools },
        };

   printf("   extended feature flags (0x80000001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_edx_amd(unsigned int  value)
{
   static named_item  names[]
      = { { "x87 FPU on chip"                         ,  0,  0, bools },
          { "virtual-8086 mode enhancement"           ,  1,  1, bools },
          { "debugging extensions"                    ,  2,  2, bools },
          { "page size extensions"                    ,  3,  3, bools },
          { "time stamp counter"                      ,  4,  4, bools },
          { "RDMSR and WRMSR support"                 ,  5,  5, bools },
          { "physical address extensions"             ,  6,  6, bools },
          { "machine check exception"                 ,  7,  7, bools },
          { "CMPXCHG8B inst."                         ,  8,  8, bools },
          { "APIC on chip"                            ,  9,  9, bools },
          { "SYSCALL and SYSRET instructions"         , 11, 11, bools },
          { "memory type range registers"             , 12, 12, bools },
          { "global paging extension"                 , 13, 13, bools },
          { "machine check architecture"              , 14, 14, bools },
          { "conditional move/compare instruction"    , 15, 15, bools },
          { "page attribute table"                    , 16, 16, bools },
          { "page size extension"                     , 17, 17, bools },
          { "multiprocessing capable"                 , 19, 19, bools },
          { "no-execute page protection"              , 20, 20, bools },
          { "AMD multimedia instruction extensions"   , 22, 22, bools },
          { "MMX Technology"                          , 23, 23, bools },
          { "FXSAVE/FXRSTOR"                          , 24, 24, bools },
          { "SSE extensions"                          , 25, 25, bools },
          { "RDTSCP"                                  , 27, 27, bools },
          { "long mode (AA-64)"                       , 29, 29, bools },
          { "3DNow! instruction extensions"           , 30, 30, bools },
          { "3DNow! instructions"                     , 31, 31, bools },
        };

   printf("   extended feature flags (0x80000001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_edx_cyrix_via(unsigned int  value)
{
   static named_item  names[]
      = { { "x87 FPU on chip"                         ,  0,  0, bools },
          { "virtual-8086 mode enhancement"           ,  1,  1, bools },
          { "debugging extensions"                    ,  2,  2, bools },
          { "page size extensions"                    ,  3,  3, bools },
          { "time stamp counter"                      ,  4,  4, bools },
          { "RDMSR and WRMSR support"                 ,  5,  5, bools },
          { "physical address extensions"             ,  6,  6, bools },
          { "machine check exception"                 ,  7,  7, bools },
          { "CMPXCHG8B inst."                         ,  8,  8, bools },
          { "APIC on chip"                            ,  9,  9, bools },
          { "SYSCALL and SYSRET instructions"         , 11, 11, bools },
          { "memory type range registers"             , 12, 12, bools },
          { "global paging extension"                 , 13, 13, bools },
          { "machine check architecture"              , 14, 14, bools },
          { "conditional move/compare instruction"    , 15, 15, bools },
          { "page attribute table"                    , 16, 16, bools },
          { "page size extension"                     , 17, 17, bools },
          { "multiprocessing capable"                 , 19, 19, bools },
          { "AMD multimedia instruction extensions"   , 22, 22, bools },
          { "MMX Technology"                          , 23, 23, bools },
          { "extended MMX"                            , 24, 24, bools },
          { "SSE extensions"                          , 25, 25, bools },
          { "AA-64"                                   , 29, 29, bools },
          { "3DNow! instruction extensions"           , 30, 30, bools },
          { "3DNow! instructions"                     , 31, 31, bools },
        };

   printf("   extended feature flags (0x80000001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_edx_transmeta(unsigned int  value)
{
   static named_item  names[]
      = { { "x87 FPU on chip"                         ,  0,  0, bools },
          { "virtual-8086 mode enhancement"           ,  1,  1, bools },
          { "debugging extensions"                    ,  2,  2, bools },
          { "page size extensions"                    ,  3,  3, bools },
          { "time stamp counter"                      ,  4,  4, bools },
          { "RDMSR and WRMSR support"                 ,  5,  5, bools },
          { "CMPXCHG8B inst."                         ,  8,  8, bools },
          { "conditional move/compare instruction"    , 15, 15, bools },
          { "FP conditional move instructions"        , 16, 16, bools },
          { "MMX Technology"                          , 23, 23, bools },
        };

   printf("   extended feature flags (0x80000001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_edx(unsigned int  value,
                   vendor_t      vendor)
{
   switch (vendor) {
   case VENDOR_INTEL:
      print_80000001_edx_intel(value);
      break;
   case VENDOR_AMD:
      print_80000001_edx_amd(value);
      break;
   case VENDOR_CYRIX:
   case VENDOR_VIA:
      print_80000001_edx_cyrix_via(value);
      break;
   case VENDOR_TRANSMETA:
      print_80000001_edx_transmeta(value);
      break;
   case VENDOR_UMC:
   case VENDOR_NEXGEN:
   case VENDOR_RISE:
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

static void
print_80000001_ecx_amd(unsigned int  value)
{
   static named_item  names[]
      = { { "LAHF/SAHF supported in 64-bit mode"      ,  0,  0, bools },
          { "CMP Legacy"                              ,  1,  1, bools },
          { "SVM: secure virtual machine"             ,  2,  2, bools },
          { "AltMovCr8"                               ,  4,  4, bools },
        };

   printf("   AMD feature flags (0x80000001/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_ecx_intel(unsigned int  value)
{
   static named_item  names[]
      = { { "LAHF/SAHF supported in 64-bit mode"      ,  0,  0, bools },
        };

   printf("   Intel feature flags (0x80000001/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_ecx(unsigned int  value,
                   vendor_t      vendor)
{
   switch (vendor) {
   case VENDOR_AMD:
      print_80000001_ecx_amd(value);
      break;
   case VENDOR_INTEL:
      print_80000001_ecx_intel(value);
      break;
   case VENDOR_CYRIX:
   case VENDOR_VIA:
   case VENDOR_TRANSMETA:
   case VENDOR_UMC:
   case VENDOR_NEXGEN:
   case VENDOR_RISE:
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

static void
print_80000001_ebx_amd(unsigned int  value)
{
   static ccstring  processor[] = { "reserved (0b000000)",
                                    "reserved (0b000001)",
                                    "reserved (0b000010)",
                                    "reserved (0b000011)",
                                    "UP client (0b000100)",
                                    "reserved (0b000101)",
                                    "reserved (0b000110)",
                                    "reserved (0b000111)",
                                    "mobile client (0b001000)",
                                    "mobile client low-power mobile VIDs"
                                       " (0b001001)",
                                    "reserved (0b001010)",
                                    "reserved (0b001011)",
                                    "UP server (0b001100)",
                                    "reserved (0b001101)",
                                    "UP server low-power 55W (0b001110)",
                                    "UP server low-power 30W (0b001111)",
                                    "2P server (0b010000)",
                                    "reserved (0b010001)",
                                    "2P server low-power 55W (0b010010)",
                                    "2P server low-power 30W (0b010011)",
                                    "MP server (0b010100)",
                                    "reserved (0b010101)",
                                    "MP server low-power 55W (0b010110)",
                                    "MP server low-power 30W (0b010111)",
                                    "reserved (0b011000)",
                                    "reserved (0b011001)",
                                    "reserved (0b011010)",
                                    "reserved (0b011011)",
                                    "reserved (0b011100)",
                                    "mobile client, 32-bit (0b011101)",
                                    "mobile client, 32-bit low-power mobile"
                                       " VIDs (0b011110)",
                                    "reserved (0b011111)",
                                    "desktop/DTR client, 32-bit (0b100000)",
                                    "mobile client, 32-bit (0b100001)",
                                    "desktop/DTR client, 32-bit (0b100010)",
                                    "mobile client, 32-bit, low-power mobile"
                                       " VIDs (0b100011)",
                                    "desktop client (0b100100)",
                                    "reserved (0b100101)",
                                    "reserved (0b100110)",
                                    "reserved (0b100111)",
                                    "reserved (0b101000)",
                                    "reserved (0b101001)",
                                    "reserved (0b101010)",
                                    "reserved (0b101011)",
                                    "reserved (0b101100)",
                                    "reserved (0b101101)",
                                    "reserved (0b101110)",
                                    "reserved (0b101111)",
                                    "reserved (0b110000)",
                                    "reserved (0b110001)",
                                    "reserved (0b110010)",
                                    "reserved (0b110011)",
                                    "reserved (0b110100)",
                                    "reserved (0b110101)",
                                    "reserved (0b110110)",
                                    "reserved (0b110111)",
                                    "reserved (0b111000)",
                                    "reserved (0b111001)",
                                    "reserved (0b111010)",
                                    "reserved (0b111011)",
                                    "reserved (0b111100)",
                                    "reserved (0b111101)",
                                    "reserved (0b111110)",
                                    "reserved (0b111111)" };
   static named_item  names[]
      = { { "MSB"                                     ,  6, 12, processor },
          { "NN"                                      ,  0,  6, NIL_IMAGES },
        };

   printf("   extended brand id = 0x%03x (%u):\n", __XB(value), __XB(value));
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000001_ebx(unsigned int  value,
                   vendor_t      vendor)
{
   switch (vendor) {
   case VENDOR_AMD:
      print_80000001_ebx_amd(value);
      break;
   case VENDOR_INTEL:
   case VENDOR_CYRIX:
   case VENDOR_VIA:
   case VENDOR_TRANSMETA:
   case VENDOR_UMC:
   case VENDOR_NEXGEN:
   case VENDOR_RISE:
   case VENDOR_UNKNOWN:
      /* DO NOTHING */
      break;
   }
}

static void
print_80000005_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "instruction # entries"                   ,  0,  7, NIL_IMAGES },
          { "instruction associativity"               ,  8, 15, NIL_IMAGES },
          { "data # entries"                          , 16, 23, NIL_IMAGES },
          { "data associativity"                      , 24, 31, NIL_IMAGES },
        };

   printf("   L1 TLB/cache information: 2M/4M pages & L1 TLB"
          " (0x80000005/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000005_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "instruction # entries"                   ,  0,  7, NIL_IMAGES },
          { "instruction associativity"               ,  8, 15, NIL_IMAGES },
          { "data # entries"                          , 16, 23, NIL_IMAGES },
          { "data associativity"                      , 24, 31, NIL_IMAGES },
        };

   printf("   L1 TLB/cache information: 4K pages & L1 TLB"
          " (0x80000005/ebx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000005_ecx(unsigned int  value)
{
   static named_item  names[]
      = { { "line size (bytes)"                       ,  0,  7, NIL_IMAGES },
          { "lines per tag"                           ,  8, 15, NIL_IMAGES },
          { "associativity"                           , 16, 23, NIL_IMAGES },
          { "size (Kb)"                               , 24, 31, NIL_IMAGES },
        };

   printf("   L1 data cache information (0x80000005/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000005_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "line size (bytes)"                       ,  0,  7, NIL_IMAGES },
          { "lines per tag"                           ,  8, 15, NIL_IMAGES },
          { "associativity"                           , 16, 23, NIL_IMAGES },
          { "size (Kb)"                               , 24, 31, NIL_IMAGES },
        };

   printf("   L1 instruction cache information (0x80000005/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static ccstring  l2_assoc[] = { "L2 off (0)",
                                "direct mapped (1)",
                                "2-way (2)",
                                NULL,
                                "4-way (4)",
                                NULL,
                                "8-way (6)",
                                NULL,
                                "16-way (8)",
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                "full (15)" };

static void
print_80000006_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "instruction # entries"                   ,  0, 11, NIL_IMAGES },
          { "instruction associativity"               , 12, 15, l2_assoc },
          { "data # entries"                          , 16, 27, NIL_IMAGES },
          { "data associativity"                      , 28, 31, l2_assoc },
        };

   printf("   L2 TLB/cache information: 2M/4M pages & L2 TLB"
          " (0x80000006/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000006_ebx(unsigned int  value)
{
   static named_item  names[]
      = { { "instruction # entries"                   ,  0, 11, NIL_IMAGES },
          { "instruction associativity"               , 12, 15, l2_assoc },
          { "data # entries"                          , 16, 27, NIL_IMAGES },
          { "data associativity"                      , 28, 31, l2_assoc },
        };

   printf("   L2 TLB/cache information: 4K pages & L2 TLB (0x80000006/ebx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000006_ecx(unsigned int   value,
                   code_stash_t*  stash)
{
   static named_item  names[]
      = { { "line size (bytes)"                       ,  0,  7, NIL_IMAGES },
          { "lines per tag"                           ,  8, 11, NIL_IMAGES },
          { "associativity"                           , 12, 15, l2_assoc },
          { "size (Kb)"                               , 16, 31, NIL_IMAGES },
        };

   printf("   L2 unified cache information (0x80000006/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   if (((value >> 12) & 0xf) == 4 && (value >> 16) == 256) {
      stash->L2_4w_256K = TRUE;
   } else if (((value >> 12) & 0xf) == 4 && (value >> 16) == 512) {
      stash->L2_4w_512K = TRUE;
   }
}

static void
print_80000007_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "temperature sensing diode"               ,  0,  0, NIL_IMAGES },
          { "frequency ID (FID) control"              ,  1,  1, NIL_IMAGES },
          { "voltage ID (VID) control"                ,  2,  2, NIL_IMAGES },
          { "thermal trip (TTP)"                      ,  3,  3, NIL_IMAGES },
          { "thermal monitor (TM)"                    ,  4,  4, NIL_IMAGES },
          { "software thermal control (STC)"          ,  5,  5, NIL_IMAGES },
          { "TscInvariant"                            ,  8,  8, NIL_IMAGES },
        };

   printf("   Advanced Power Management Features (0x80000007/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000008_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "maximum physical address"                ,  0,  7, NIL_IMAGES },
          { "maximum linear address"                  ,  8, 15, NIL_IMAGES },
        };

   printf("   Physical Address and Linear Address Size (0x80000008/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_80000008_ecx(unsigned int  value)
{
   static named_item  names[]
      = { { "number of logical CPU cores - 1"         ,  0,  7, NIL_IMAGES },
          { "ApicIdCoreIdSize"                        , 12, 15, NIL_IMAGES },
        };

   printf("   Logical CPU cores (0x80000008/ecx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_8000000a_eax(unsigned int  value)
{
   static named_item  names[]
      = { { "SvmRev: SVM revision"                    ,  0,  7, NIL_IMAGES },
        };

   printf("   SVM Secure Virtual Machine (0x8000000a/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_8000000a_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "LBR virtualization"                      ,  1,  1, bools },
        };

   printf("   SVM Secure Virtual Machine (0x8000000a/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_8000000a_ebx(unsigned int  value)
{
   printf("   NASID: number of address space identifiers = 0x%x (%u):\n",
          value, value);
}

static void
print_80860001_eax(unsigned int  value)
{
   static ccstring  family[]   = { NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   "Transmeta Crusoe" };
   static named_item  names[]
      = { { "generation    "                          ,  8, 11, family },
          { "model         "                          ,  4,  7, NIL_IMAGES },
          { "stepping      "                          ,  0,  3, NIL_IMAGES },
        };

   printf("   Transmeta processor signature (0x80860001/eax):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);

   print_synth_transmeta("      (simple synth)", value, UN);
}

static void
print_80860001_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "recovery CMS active"                     ,  0,  0, bools },
          { "LongRun"                                 ,  1,  1, bools },
          { "LongRun Table Interface LRTI (CMS 4.2)"  ,  3,  3, bools },
          { "persistent translation technology 1.x"   ,  7,  7, bools },
          { "persistent translation technology 2.0"   ,  8,  8, bools },
        };

   printf("   Transmeta feature flags (0x80860001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
print_transmeta_proc_rev_meaning(unsigned int  proc_rev)
{
   switch (proc_rev & 0xffff0000) {
   case 0x01010000:
      printf("(TM3200)");
      break;
   case 0x01020000:
      printf("(TM5400)");
      break;
   case 0x01030000:
      if ((proc_rev & 0xffffff00) == 0x00000000) {
         printf("(TM5400 / TM5600)");
      } else {
         printf("(unknown)");
      }
      break;
   case 0x01040000:
   case 0x01050000:
      printf("(TM5500 / TM5800)");
      break;
   default:
      printf("(unknown)");
      break;
   }
}

static void
print_80860001_ebx_ecx(unsigned int  val_ebx,
                       unsigned int  val_ecx)
{
   printf("   Transmeta processor revision (0x80000001/edx)"
          " = %u.%u-%u.%u-%u ", 
          (val_ebx >> 24) & 0xff,
          (val_ebx >> 16) & 0xff,
          (val_ebx >>  8) & 0xff,
          (val_ebx >>  0) & 0xff,
          val_ecx);
          
   if (val_ebx == 0x20000000) {
      printf("(see 80860002/eax)");
   } else {
      print_transmeta_proc_rev_meaning(val_ebx);
   }
}

static void
print_80860002_eax(unsigned int   value,
                   code_stash_t*  stash)
{
   if (stash->transmeta_proc_rev == 0x02000000) {
      printf("   Transmeta processor revision (0x80860002/eax)"
             " = %u.%u-%u.%u ", 
             (value >> 24) & 0xff,
             (value >> 16) & 0xff,
             (value >>  8) & 0xff,
             (value >>  0) & 0xff);

      print_transmeta_proc_rev_meaning(value);

      stash->transmeta_proc_rev = value;
   }
}

static void
print_c0000001_edx(unsigned int  value)
{
   static named_item  names[]
      = { { "alternate instruction set"                ,  0,  0, bools },
          { "alternate instruction set enabled"        ,  1,  1, bools },
          { "random number generator"                  ,  2,  2, bools },
          { "random number generator enabled"          ,  3,  3, bools },
          { "LongHaul MSR 0000_110Ah"                  ,  4,  4, bools },
          { "FEMMS"                                    ,  5,  5, bools },
          { "advanced cryptography engine (ACE)"       ,  6,  6, bools },
          { "advanced cryptography engine (ACE)enabled",  7,  7, bools },
        };

   printf("   extended feature flags (0xc0000001/edx):\n");
   print_names(value, names, LENGTH(names, named_item),
               /* max_len => */ 0);
}

static void
usage(void)
{
   printf("usage: %s [options...]\n", program);
   printf("\n");
   printf("Dump detailed information about the CPU(s) gathered from the CPUID"
          " instruction,\n");
   printf("and also determine the exact model of CPU(s).\n");
   printf("\n");
   printf("options:\n");
   printf("\n");
   printf("   -1,      --one-cpu    display information only for the first"
                                    " CPU.\n");
   printf("                         (Meaningful only with -k or --kernel.)\n");
   printf("   -f FILE, --file=FILE  read raw hex information (-r output) from"
                                    " FILE instead\n");
   printf("                         of from executions of the cpuid"
                                    " instruction\n");
   printf("   -h, -H,  --help       display this help information\n");
   printf("   -i,      --inst       use the CPUID instruction: The information"
                                    " it provides\n");
   printf("                         is reliable, but it works on only the"
                                    " current CPU.\n");
   printf("                         This should be OK on single-chip systems or"
                                    " homogeneous\n");
   printf("                         multi-chip systems.  It is not necessary to"
                                    " be root.\n");
   printf("                         (This option is the default.)\n");
   printf("   -k,      --kernel     use the CPUID kernel module: This displays"
                                    " information\n");
   printf("                         for all CPUs, but the information it"
                                    " provides is not\n");
   printf("                         reliable for all combinations of CPU type"
                                    " and kernel\n");
   printf("                         version.  Typically, it is necessary to be"
                                    " root.\n");
   printf("   -r,      --raw        display raw hex information with no"
                                    " decoding\n");
   printf("   -v,      --version    display cpuid version\n");
   printf("\n");
   exit(1);
}

static void
explain_errno(void)
{
   if (errno == ENODEV || errno == ENXIO) {
      fprintf(stderr,
              "%s: if running a modular kernel, execute"
              " \"modprobe cpuid\",\n",
              program);
      fprintf(stderr,
              "%s: wait a few seconds, and then try again\n",
              program);
      fprintf(stderr,
              "%s: or consider using the -i option\n", program);
   } else if (errno == ENOENT) {
      fprintf(stderr,
              "%s: if running a modular kernel, execute"
              " \"modprobe cpuid\",\n",
              program);
      fprintf(stderr,
              "%s: wait a few seconds, and then try again;\n",
              program);
      fprintf(stderr,
              "%s: if it still fails, try executing:\n",
              program);
      fprintf(stderr,
              "%s:    mknod /dev/cpu/0/cpuid c %u 0\n",
              program, CPUID_MAJOR);
      fprintf(stderr,
              "%s:    mknod /dev/cpu/1/cpuid c %u 1\n",
              program, CPUID_MAJOR);
      fprintf(stderr,
              "%s:    etc.\n",
              program);
      fprintf(stderr,
              "%s: and then try again\n",
              program);
      fprintf(stderr,
              "%s: or consider using the -i option\n", program);
   } else if ((errno == EPERM || errno == EACCES) && getuid() != 0) {
      fprintf(stderr,
              "%s: on most systems,"
              " it is necessary to execute this as root\n",
              program);
      fprintf(stderr,
              "%s: or consider using the -i option\n", program);
   }
   exit(1);
}

#define WORD_EAX  0
#define WORD_EBX  1
#define WORD_ECX  2
#define WORD_EDX  3

#define WORD_NUM  4

#define FOUR_CHARS_VALUE(s) \
   ((unsigned int)((s)[0] + ((s)[1] << 8) + ((s)[2] << 16) + ((s)[3] << 24)))
#define IS_VENDOR_ID(words, s)                        \
   (   (words)[WORD_EBX] == FOUR_CHARS_VALUE(&(s)[0]) \
    && (words)[WORD_EDX] == FOUR_CHARS_VALUE(&(s)[4]) \
    && (words)[WORD_ECX] == FOUR_CHARS_VALUE(&(s)[8]))

static void
print_reg_raw (unsigned int        reg,
               const unsigned int  words[WORD_NUM])
{
   printf("   0x%08x: eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
          (unsigned int)reg,
          words[WORD_EAX], words[WORD_EBX],
          words[WORD_ECX], words[WORD_EDX]);
}

static void 
print_reg (unsigned int        reg,
           const unsigned int  words[WORD_NUM],
           boolean             raw,
           unsigned int        try,
           code_stash_t*       stash)
{
   if (raw) {
      print_reg_raw(reg, words);
   } else if (reg == 0) {
      // basic_max already set to words[WORD_EAX]
      printf("   vendor_id = \"%4.4s%4.4s%4.4s\"\n",
             (const char*)&words[WORD_EBX], 
             (const char*)&words[WORD_EDX], 
             (const char*)&words[WORD_ECX]);
      if (IS_VENDOR_ID(words, "GenuineIntel")) {
         stash->vendor = VENDOR_INTEL;
      } else if (IS_VENDOR_ID(words, "AuthenticAMD")) {
         stash->vendor = VENDOR_AMD;
      } else if (IS_VENDOR_ID(words, "CyrixInstead")) {
         stash->vendor = VENDOR_CYRIX;
      } else if (IS_VENDOR_ID(words, "CentaurHauls")) {
         stash->vendor = VENDOR_VIA;
      } else if (IS_VENDOR_ID(words, "UMC UMC UMC ")) {
         stash->vendor = VENDOR_UMC;
      } else if (IS_VENDOR_ID(words, "NexGenDriven")) {
         stash->vendor = VENDOR_NEXGEN;
      } else if (IS_VENDOR_ID(words, "RiseRiseRise")) {
         stash->vendor = VENDOR_RISE;
      } else if (IS_VENDOR_ID(words, "GenuineTMx86")) {
         stash->vendor = VENDOR_TRANSMETA;
      }
   } else if (reg == 1) {
      print_1_eax(words[WORD_EAX], stash->vendor);
      print_1_ebx(words[WORD_EBX]);
      print_brand(words[WORD_EAX], words[WORD_EBX]);
      print_1_edx(words[WORD_EDX]);
      print_1_ecx(words[WORD_ECX]);
      stash->val_1_eax = words[WORD_EAX];
      stash->val_1_ebx = words[WORD_EBX];
      stash->val_1_ecx = words[WORD_ECX];
      stash->val_1_edx = words[WORD_EDX];
   } else if (reg == 2) {
      unsigned int  word = 0;
      for (; word < 4; word++) {
         if ((words[word] & 0x80000000) == 0) {
            const unsigned char*  bytes = (const unsigned char*)&words[word];
            unsigned int          byte  = (try == 0 && word == WORD_EAX ? 1
                                                                        : 0);
            for (; byte < 4; byte++) {
               print_2_byte(bytes[byte], stash->vendor, stash->val_1_eax);
               stash_intel_cache(stash, bytes[byte]);
            }
         }
      }
   } else if (reg == 3) {
      printf("   processor serial number:"
             " %04X-%04X-%04X-%04X-%04X-%04X\n",
             stash->val_1_eax >> 16, stash->val_1_eax & 0xffff, 
             words[WORD_EDX] >> 16, words[WORD_EDX] & 0xffff, 
             words[WORD_ECX] >> 16, words[WORD_ECX] & 0xffff);
   } else if (reg == 4) {
      printf("   deterministic cache parameters (4):\n");
      print_4_eax(words[WORD_EAX]);
      print_4_ebx(words[WORD_EAX]);
      printf("      number of sets - 1 (s)            = %u\n", words[WORD_ECX]);
      stash->val_4_eax = words[WORD_EAX];
   } else if (reg == 5) {
      printf("   MONITOR/MWAIT (5):\n");
      print_5_eax(words[WORD_EAX]);
      print_5_ebx(words[WORD_EAX]);
      print_5_ecx(words[WORD_ECX]);
      print_5_edx(words[WORD_EDX]);
   } else if (reg == 6) {
      printf("   Thermal and Power Management Features (6):\n");
      print_6_eax(words[WORD_EAX]);
      print_6_ebx(words[WORD_EAX]);
      print_6_ecx(words[WORD_ECX]);
   } else if (reg == 7) {
      /* Reserved: DO NOTHING */
   } else if (reg == 8) {
      /* Reserved: DO NOTHING */
   } else if (reg == 9) {
      /* Reserved: DO NOTHING */
   } else if (reg == 0xa) {
      print_a_eax(words[WORD_EAX]);
      print_a_ebx(words[WORD_EBX]);
   } else if (reg == 0x80000000) {
      // basic_max already set to words[WORD_EAX]
   } else if (reg == 0x80000001) {
      print_80000001_eax(words[WORD_EAX], stash->vendor);
      print_80000001_edx(words[WORD_EDX], stash->vendor);
      print_80000001_ebx(words[WORD_EBX], stash->vendor);
      print_80000001_ecx(words[WORD_ECX], stash->vendor);
      stash->val_80000001_ebx = words[WORD_EBX];
      stash->val_80000001_ecx = words[WORD_ECX];
   } else if (reg == 0x80000002) {
      memcpy(&stash->brand[0], words, sizeof(unsigned int)*WORD_NUM);
   } else if (reg == 0x80000003) {
      memcpy(&stash->brand[16], words, sizeof(unsigned int)*WORD_NUM);
   } else if (reg == 0x80000004) {
      memcpy(&stash->brand[32], words, sizeof(unsigned int)*WORD_NUM);
      printf("   brand = \"%s\"\n", stash->brand);
   } else if (reg == 0x80000005) {
      print_80000005_eax(words[WORD_EAX]);
      print_80000005_ebx(words[WORD_EBX]);
      print_80000005_ecx(words[WORD_ECX]);
      print_80000005_edx(words[WORD_EDX]);
   } else if (reg == 0x80000006) {
      print_80000006_eax(words[WORD_EAX]);
      print_80000006_ebx(words[WORD_EBX]);
      print_80000006_ecx(words[WORD_ECX], stash);
   } else if (reg == 0x80000007) {
      print_80000007_edx(words[WORD_EDX]);
   } else if (reg == 0x80000008) {
      print_80000008_eax(words[WORD_EAX]);
      print_80000008_ecx(words[WORD_ECX]);
      stash->val_80000008_ecx = words[WORD_ECX];
   } else if (reg == 0x80000009) {
      /* reserved for Intel feature flag expansion */
   } else if (reg == 0x8000000a) {
      print_8000000a_eax(words[WORD_EAX]);
      print_8000000a_edx(words[WORD_EAX]);
      print_8000000a_ebx(words[WORD_EBX]);
   } else if (0x8000000b <= reg && reg <= 0x80000018) {
      /* reserved for vendors to be determined feature flag expansion */
   } else if (reg == 0x80860000) {
      // basic_max already set to words[WORD_EAX]
   } else if (reg == 0x80860001) {
      print_80860001_eax(words[WORD_EAX]);
      print_80860001_edx(words[WORD_EDX]);
      print_80860001_ebx_ecx(words[WORD_EBX], words[WORD_ECX]);
   } else if (reg == 0x80860002) {
      print_80860002_eax(words[WORD_EAX], stash);
      printf("   Transmeta CMS revision (0x80000002/ecx)"
             " = %u.%u-%u.%u-%u ", 
             (words[WORD_EBX] >> 24) & 0xff,
             (words[WORD_EBX] >> 16) & 0xff,
             (words[WORD_EBX] >>  8) & 0xff,
             (words[WORD_EBX] >>  0) & 0xff,
             words[WORD_ECX]);
   } else if (reg == 0x80860003) {
      memcpy(&stash->transmeta_info[0], words, sizeof(unsigned int)*WORD_NUM);
   } else if (reg == 0x80860004) {
      memcpy(&stash->transmeta_info[16], words, sizeof(unsigned int)*WORD_NUM);
   } else if (reg == 0x80860005) {
      memcpy(&stash->transmeta_info[32], words, sizeof(unsigned int)*WORD_NUM);
   } else if (reg == 0x80860006) {
      memcpy(&stash->transmeta_info[48], words, sizeof(unsigned int)*WORD_NUM);
      printf("   Transmeta information = \"%s\"\n", stash->transmeta_info);
   } else if (reg == 0x80860007) {
      printf("   Transmeta core clock frequency = %u MHz\n",
             words[WORD_EAX]);
      printf("   Transmeta processor voltage    = %u mV\n",
             words[WORD_EBX]);
      printf("   Transmeta performance          = %u%%\n",
             words[WORD_ECX]);
      printf("   Transmeta gate delay           = %u fs\n",
             words[WORD_EDX]);
   } else if (reg == 0xc0000000) {
      // basic_max already set to words[WORD_EAX]
   } else if (reg == 0xc0000001) {
      if (stash->vendor == VENDOR_VIA) {
         printf("   0x%08x: eax=0x%08x\n", 
                (unsigned int)reg, words[WORD_EAX]);
         print_c0000001_edx(words[WORD_EDX]);
      } else {
         /* DO NOTHING */
      }
   } else {
      print_reg_raw(reg, words);
   }
}

#define USE_INSTRUCTION  (-2)

static int
real_setup(unsigned int  cpu,
           boolean       inst)
{
   if (inst) {
      return USE_INSTRUCTION;
   } else {
      int    cpuid_fd = -1;
      char   cpuid_name[20];

      if (cpuid_fd == -1 && cpu == 0) {
         cpuid_fd = open("/dev/cpuid", O_RDONLY);
         if (cpuid_fd == -1 && errno != ENOENT) {
            fprintf(stderr, 
                    "%s: cannot open /dev/cpuid; errno = %d (%s)\n", 
                    program, errno, strerror(errno));
            explain_errno();
         }
      }

      if (cpuid_fd == -1) {
         sprintf(cpuid_name, "/dev/cpu/%u/cpuid", cpu);
         cpuid_fd = open(cpuid_name, O_RDONLY);
         if (cpuid_fd == -1) {
            if (cpu > 0) {
               if (errno == ENXIO)  return -1;
               if (errno == ENODEV) return -1;
            }
            if (errno != ENOENT) {
               fprintf(stderr, 
                       "%s: cannot open /dev/cpuid or %s; errno = %d (%s)\n", 
                       program, cpuid_name, errno, strerror(errno));
               explain_errno();
            }
         }
      }

      if (cpuid_fd == -1) {
         /*
         ** Lots of Linux's omit the /dev/cpuid or /dev/cpu/%u/cpuid files.
         ** Try creating a temporary file with mknod.
         **
         ** mkstemp is of absolutely no security value here because I can't
         ** use the actual file it generates, and have to delete it and
         ** re-create it with mknod.  But I have to use it anyway to
         ** eliminate errors from smartypants gcc/glibc during the link if I
         ** attempt to use tempnam.
         */
         char  tmpname[20];
         int   dummy_fd;
         strcpy(tmpname, "/tmp/cpuidXXXXXX");
         dummy_fd = mkstemp(tmpname);
         if (dummy_fd != -1) {
            close(dummy_fd);
            remove(tmpname);
            {
               int  status = mknod(tmpname,
                                   (S_IFCHR | S_IRUSR),
                                   makedev(CPUID_MAJOR, cpu));
               if (status == 0) {
                  cpuid_fd = open(tmpname, O_RDONLY);
                  remove(tmpname);
               }
            }
         }
         if (cpuid_fd == -1) {
            if (cpu > 0) {
               if (errno == ENXIO)  return -1;
               if (errno == ENODEV) return -1;
            }
            fprintf(stderr, 
                    "%s: cannot open /dev/cpuid or %s; errno = %d (%s)\n", 
                    program, cpuid_name, errno, strerror(errno));
            explain_errno();
         }
      }

      return cpuid_fd;
   }
}

static int real_get (int           cpuid_fd,
                     unsigned int  reg,
                     unsigned int  words[],
                     boolean       quiet)
{
   if (cpuid_fd == USE_INSTRUCTION) {
     asm("cpuid"
         : "=a" (words[WORD_EAX]),
           "=b" (words[WORD_EBX]),
           "=c" (words[WORD_ECX]),
           "=d" (words[WORD_EDX])
         : "a" (reg));
   } else {
      int  status;

      status = lseek(cpuid_fd, reg, SEEK_SET);
      if (status == -1) {
         if (quiet) {
            return FALSE;
         } else {
            fprintf(stderr,
                    "%s: unable to seek cpuid file to offset 0x%08x;"
                    " errno = %d (%s)\n",
                    program, reg, errno, strerror(errno));
            exit(1);
         }
      }

      status = read(cpuid_fd, words, 16);
      if (status == -1) {
         if (quiet) {
            return FALSE;
         } else {
            fprintf(stderr,
                    "%s: unable to read cpuid file at offset 0x%08x;"
                    " errno = %d (%s)\n",
                    program, reg, errno, strerror(errno));
            exit(1);
         }
      }
   }

   return TRUE;
}

static void
do_real(boolean  one_cpu,
        boolean  inst,
        boolean  raw)
{
   unsigned int  cpu;

   for (cpu = 0;; cpu++) {
      int            cpuid_fd   = -1;
      code_stash_t   stash      = NIL_STASH;
      unsigned int   basic_max;
      unsigned int   reg;

      if (one_cpu && cpu > 0) break;
      if (inst && cpu > 0)    break;

      cpuid_fd = real_setup(cpu, inst);
      if (cpuid_fd == -1) break;

      if (inst) {
         printf("CPU:\n");
      } else {
         printf("CPU %u:\n", cpu);
      }

      basic_max = 0;
      for (reg = 0; reg <= basic_max; reg++) {
         unsigned int  words[WORD_NUM];

         real_get(cpuid_fd, reg, words, FALSE);

         if (reg == 0) {
            basic_max = words[WORD_EAX];
         }

         if (reg == 2) {
            unsigned int  max_tries = words[WORD_EAX] & 0xff;
            unsigned int  try       = 0;

            if (!raw) {
               printf("   cache and TLB information (2):\n");
            }

            for (;;) {
               print_reg(reg, words, raw, try, &stash);

               try++;
               if (try >= max_tries) break;

               real_get(cpuid_fd, reg, words, FALSE);
            } while (try < max_tries);
         } else {
            print_reg(reg, words, raw, 0, &stash);
         }
      }

      basic_max = 0x80000000;
      for (reg = 0x80000000; reg <= basic_max; reg++) {
         boolean       success;
         unsigned int  words[WORD_NUM];

         success = real_get(cpuid_fd, reg, words, TRUE);
         if (!success) break;

         if (reg == 0x80000000) {
            basic_max = words[WORD_EAX];
         }

         print_reg(reg, words, raw, 0, &stash);
      }

      basic_max = 0x80860000;
      for (reg = 0x80860000; reg <= basic_max; reg++) {
         boolean       success;
         unsigned int  words[WORD_NUM];

         success = real_get(cpuid_fd, reg, words, TRUE);
         if (!success) break;

         if (reg == 0x80860000) {
            basic_max = words[WORD_EAX];
         }

         print_reg(reg, words, raw, 0, &stash);
      }

      basic_max = 0xc0000000;
      for (reg = 0xc0000000; reg <= basic_max; reg++) {
         boolean       success;
         unsigned int  words[WORD_NUM];

         success = real_get(cpuid_fd, reg, words, TRUE);
         if (!success) break;

         if (reg == 0xc0000000) {
            basic_max = words[WORD_EAX];
         }

         print_reg(reg, words, raw, 0, &stash);
      }
      
      if (!raw) {
         decode_mp_synth(&stash);
         print_mp_synth(&stash.mp);
         print_synth(&stash);
      }

      close(cpuid_fd);
   }
}

static void
do_file(ccstring  filename,
        boolean   raw)
{
   boolean       seen_cpu = FALSE;
   unsigned int  cpu      = -1;
   unsigned int  try2     = -1;
   code_stash_t  stash;

   FILE*  file = fopen(filename, "r");
   if (file == NULL) {
      fprintf(stderr,
              "%s: unable to open %s; errno = %d (%s)\n",
              program, filename, errno, strerror(errno));
      exit(1);
   }

   while (!feof(file)) {
      char          buffer[80];
      char*         ptr;
      unsigned int  len;
      int           status;
      unsigned int  reg;
      unsigned int  words[WORD_NUM];

      ptr = fgets(buffer, LENGTH(buffer, char), file);
      if (ptr == NULL && errno == 0) break;
      if (ptr == NULL) {
         fprintf(stderr,
                 "%s: unable to read a line of text from %s;"
                 " errno = %d (%s)\n",
                 program, filename, errno, strerror(errno));
         exit(1);
      }

      len = strlen(buffer);

      status = sscanf(ptr, "CPU %u:\r", &cpu);
      if (status == 1 || strcmp(ptr, "CPU:\n") == SAME) {
         if (!raw && seen_cpu) {
            decode_mp_synth(&stash);
            print_mp_synth(&stash.mp);
            print_synth(&stash);
         }

         seen_cpu = TRUE;

         if (status == 1) {
            printf("CPU %u:\n", cpu);
         } else {
            printf("CPU:\n");
         }
         try2 = 0;
         {
            static code_stash_t  empty_stash = NIL_STASH;
            stash = empty_stash;
         }
         continue;
      }

      status = sscanf(ptr,
                      "   0x%x: eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\r",
                      &reg, 
                      &words[WORD_EAX], &words[WORD_EBX],
                      &words[WORD_ECX], &words[WORD_EDX]);
      if (status == 5) {
         if (reg == 2) {
            if (try2 == 0) {
               if (!raw) {
                  printf("   cache and TLB information (2):\n");
               }
            }
            print_reg(reg, words, raw, try2++, &stash);
         } else {
            print_reg(reg, words, raw, 0, &stash);
         }
         continue;
      }

      fprintf(stderr,
              "%s: unexpected input with -f option: %s\n",
              program, ptr);
      exit(1);
   }

   if (!raw && seen_cpu) {
      decode_mp_synth(&stash);
      print_mp_synth(&stash.mp);
      print_synth(&stash);
   }

   fclose(file);
}

int
main(int     argc,
     string  argv[])
{
   static ccstring             shortopts  = "+hH1ikrf:v";
   static const struct option  longopts[] = {
      { "help",    no_argument,       NULL, 'h'  },
      { "one-cpu", no_argument,       NULL, '1'  },
      { "inst",    no_argument,       NULL, 'i'  },
      { "kernel",  no_argument,       NULL, 'k'  },
      { "raw",     no_argument,       NULL, 'r'  },
      { "file",    required_argument, NULL, 'f'  },
      { "version", no_argument,       NULL, 'v'  },
      { NULL,      no_argument,       NULL, '\0' }
   };

   boolean  opt_one_cpu  = FALSE;
   boolean  opt_inst     = FALSE;
   boolean  opt_kernel   = FALSE;
   boolean  opt_raw      = FALSE;
   cstring  opt_filename = NULL;
   boolean  opt_version  = FALSE;

   program = strrchr(argv[0], '/');
   if (program == NULL) {
      program = argv[0];
   } else {
      program++;
   }

   opterr = 0;

   for (;;) {
      int  longindex;
      int  opt = getopt_long(argc, argv, shortopts, longopts, &longindex);

      if (opt == EOF) break;

      switch (opt) {
      case 'h':
      case 'H':
         usage();
         /*NOTREACHED*/
      case '1':
         opt_one_cpu = TRUE;
         break;
      case 'i':
         opt_inst = TRUE;
         break;
      case 'k':
         opt_kernel = TRUE;
         break;
      case 'r':
         opt_raw = TRUE;
         break;
      case 'f':
         opt_filename = optarg;
         break;
      case 'v':
         opt_version = TRUE;
         break;
      case '?':
      default:
         if (optopt == '\0') {
            fprintf(stderr,
                    "%s: unrecogized option: %s\n", program, argv[optind-1]);
         } else {
            fprintf(stderr, 
                    "%s: unrecognized option letter: %c\n", program, optopt);
         }
         usage();
         /*NOTREACHED*/
      }
   }

   if (optind < argc) {
      fprintf(stderr, "%s: unrecognized argument: %s\n", program, argv[optind]);
      usage();
      /*NOTREACHED*/
   }

   if (opt_inst && opt_kernel) {
      fprintf(stderr,
              "%s: -i/--inst and -k/--kernel are incompatible options\n", 
              program);
      exit(1);
   }

   // Default to -i.  So use inst unless -k is specified.
   boolean  inst = !opt_kernel;

   if (opt_version) {
      printf("cpuid version %s\n", XSTR(VERSION));
   } else {
      if (opt_filename != NULL) {
         do_file(opt_filename, opt_raw);
      } else {
         do_real(opt_one_cpu, inst, opt_raw);
      }
   }

   exit(0);
   /*NOTREACHED*/
}
#endif
