/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "../main.h"

enum {
	ENCODING_ASCII = 0,
	ENCODING_CP850 = 1
};

char* aux_filter_rad_output(const char*);
int   aux_is_encoded(int, unsigned char);
int   aux_is_printable(int);
int aux_atoi32(const char *str);
int aux_bin2str(const u8 *in, int len, char *out);
