#ifndef _INCLUDE_TYPES_H_
#define _INCLUDE_TYPES_H_

/* basic data types */
#define u8 unsigned char
#define ut8 unsigned char
#define ut16 unsigned short
#ifndef ut32
#define ut32 unsigned int
#endif
#define ut64 unsigned long long
typedef ut64 addr_t;
#define uchar unsigned char

#ifndef SIZEOF_OFF_T
#define SIZEOF_OFF_T 8
#endif

#endif
