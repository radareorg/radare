/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

enum {
	ENCODING_ASCII = 0,
	ENCODING_CP850 = 1
};

char* aux_filter_rad_output(const char*);
int   aux_is_encoded(int, unsigned char);
int   aux_is_printable(int);
int aux_atoi32(const char *str);
