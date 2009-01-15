#include "r_types.h"
#include "r_util.h"

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

int r_num_init(struct r_num_t *num)
{
	num->callback = NULL;
	num->userptr = NULL;
	return 0;
}

struct r_num_t *r_num_new(u64 (*cb)(void*,const char *,int*), void *ptr)
{
	struct r_num_t *num;
	num = (struct r_num_t*) malloc(sizeof(struct r_num_t));
	num->callback = cb;
	num->userptr = ptr;
	return num;
}

/* old get_offset */
u64 r_num_get(struct r_num_t *num, const char *str)
{
	int i, j;
	char lch;
	u64 ret = 0LL;

	/* resolve string with an external callback */
	if (num && num->callback) {
		int ok;
		ret = num->callback(num->userptr, str, &ok);
		if (ok) return ret;
	}

	for(i=0;str[i]!=' ';i++);
	if (i>1 && str[0]=='0' && str[1]=='x') {
		sscanf(str, "0x%llx", &ret);
	} else {
		lch = str[strlen(str)-1];
		switch(lch) {
		case 'o': // octal
			sscanf(str, "%llo", &ret);
			break;
		case 'b': // binary
			ret = 0;
			for(j=0,i=strlen(str)-2;i>=0;i--,j++) {
				if (str[i]=='1') ret|=1<<j;
				else if (str[i]!='0') break;
			}
			break;
		default:
			sscanf(str, "%lld", &ret);
			switch(lch) {
			case 'K': case 'k':
				ret *= 1024;
				break;
			case 'M': case 'm':
				ret *= 1024*1024;
				break;
			case 'G': case 'g':
				ret *= 1024*1024*1024;
				break;
			}
		}
	}

	return ret;
}

/* TODO: implement full support for nested ops with parenthesis and so on */
u64 r_num_math(struct r_num_t *num, const char *str)
{
	return r_num_get(num, str);
}
