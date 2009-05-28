#ifndef _INCLUDE_TYPES_H_
#define _INCLUDE_TYPES_H_

/* basic data types */
#define u8 unsigned char
#define u16 unsigned short
#ifndef u32
#define u32 unsigned int
#endif
#define u64 unsigned long long
typedef u64 addr_t;
#define uchar unsigned char

#ifndef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8
#endif

#endif
