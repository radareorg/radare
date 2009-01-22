/* radare - LGPL - Copyright 2007-2009 pancake<nopcode.org> */

#include "r_cons.h"
#include "r_print.h"

static int flags =
	R_PRINT_FLAGS_COLOR |
	R_PRINT_FLAGS_ADDRMOD;

void r_print_set_flags(int _flags)
{
	flags = _flags;
}

void r_print_addr(u64 addr)
{
	//config_get_i("cfg.addrmod");
	int mod = flags & R_PRINT_FLAGS_ADDRMOD;
	char ch = (0==(addr%(mod?mod:1)))?',':' ';

	if (flags & R_PRINT_FLAGS_COLOR) {
		r_cons_printf("%s0x%08llx"C_RESET"%c ",
			r_cons_palette[PAL_ADDRESS], addr, ch);
	} else r_cons_printf("0x%08llx%c ", addr, ch);
}

static const char hex[16] = "0123456789ABCDEF";
void r_print_hexdump(u64 addr, u8 *buf, int len, int step, int columns, int header)
{
	int i,j,k,inc;

	inc = 2+(int)((columns-14)/4);
	if (inc%2) inc++;
	inc = 16;

	if (header) {
		// only for color..too many options .. brbr
		r_cons_printf(r_cons_palette[PAL_HEADER]);
		r_cons_strcat("   offset   ");
		k = 0; // TODO: ??? SURE??? config.seek & 0xF;
		for (i=0; i<inc; i++) {
			r_cons_printf(" %c", hex[(i+k)%16]);
			if (i&1) r_cons_strcat(" ");
		}
		for (i=0; i<inc; i++)
			r_cons_printf("%c", hex[(i+k)%16]);
		r_cons_newline();
	}

	for(i=0; i<len; i+=inc) {
		r_print_addr(addr+(i*step));

		for(j=i;j<i+inc;j++) {
			if (j>=len) {
				r_cons_printf("  ");
				if (j%2) r_cons_printf(" ");
				continue;
			}
			r_cons_printf("%02x", (u8)buf[j]);
			//print_color_byte_i(j, "%02x", (unsigned char)buf[j]);
			if (j%2) r_cons_strcat(" ");
		}

		for(j=i; j<i+inc; j++) {
			if (j >= len)
				r_cons_strcat(" ");
			else r_cons_printf("%c",
				(IS_PRINTABLE(buf[j]))?
					buf[j] : '.');
		}
		r_cons_newline();
		addr+=inc;
	}
}

void r_print_bytes(const u8* buf, int len, const char *fmt)
{
	int i;
	for(i=0;i<len;i++)
		r_cons_printf(fmt, buf[i]);
	r_cons_newline();
}

void r_print_raw(const u8* buf, int len)
{
	r_cons_memcat(buf, len);
}

