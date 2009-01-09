#include "r_types.h"

#define __htonq(x) (\
        (((x) & 0xff00000000000000LL) >> 56)  | \
        (((x) & 0x00ff000000000000LL) >> 40)  | \
        (((x) & 0x0000ff0000000000LL) >> 24)  | \
        (((x) & 0x000000ff00000000LL) >> 8)   | \
        (((x) & 0x00000000ff000000LL) << 8)   | \
        (((x) & 0x0000000000ff0000LL) << 24)  | \
        (((x) & 0x000000000000ff00LL) << 40)  | \
        (((x) & 0x00000000000000ffLL) << 56))

u64 htonq(u64 value) {
        u64 ret  = value;
#if LIL_ENDIAN
        endian_memcpy_e((u8*)&ret, (u8*)&value, 8, 0);
#endif
        return ret;
}


#warning get_offset MUST be renamed
u64 get_offset(const char *str)
{
	return atoi(str);
}

#warning get_math MUST be renamed
u64 get_math(const char *str)
{
	return get_offset(str);
}
