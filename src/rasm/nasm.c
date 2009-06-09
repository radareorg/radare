/* radare - Copyright 2009 nibble<.ds@gmail.com> */

#include "rasm.h"
#include "../radare.h"

int rasm_nasm_x86(u64 offset, const char *str, u8 *data)
{
	int len;
	char cmd[1024];
	u8 *out;
	sprintf(cmd, "nasm /dev/stdin -o /dev/stdout <<__\nBITS 64\nORG 0x%llx\n%s\n__", offset, str);
	out = (u8 *)r_sys_cmd_str(cmd, "", &len);
	if (out) {
		memcpy(data, out, len);
		free(out);
	} else return -1;
	return len;
}
