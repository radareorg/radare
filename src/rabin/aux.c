/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include "aux.h"

char* aux_filter_rad_output(const char *string)
{
	static char buff[255];
	char *p = buff;

	for (; *string != '\0' && p-buff < 255; string++) {
		switch(*string) {
			case ' ':
			case '=':
			case '@':
			case '%':
			case '#':
			case '!':
			case '|':
			case ':':
			case '"':
			case '[':
			case ']':
			case '&':
			case '>':
			case '<':
			case ';':
			case '`':
			case '.':
			case '*':
			case '/':
			case '+':
			case '-':
			case '\'':
			case '\n':
			case '\t':
				*p++ = '_';
				break;
			default:
				*p++ = *string;
				break;
		}
	}

	*p='\0';

	return buff;
}

int aux_atoi32(const char *str)
{
	int ret;
	if (str[0]=='0'&&str[1]=='x')
		sscanf(str,"0x%x",&ret);
	else ret = atoi(str);
	return ret;
}

int aux_is_encoded(int encoding, unsigned char c)
{
	switch(encoding) {
		case ENCODING_ASCII:
			break;
		case ENCODING_CP850:
			switch(c) {
				// CP850
				case 128: // cedilla
				case 133: // a grave
				case 135: // minicedilla
				case 160: // a acute
				case 161: // i acute
				case 129: // u dieresi
				case 130: // e acute
				case 139: // i dieresi
				case 162: // o acute
				case 163: // u acute
				case 164: // enye
				case 165: // enyemay
				case 181: // A acute
				case 144: // E acute
				case 214: // I acute
				case 224: // O acute
				case 233: // U acute
					return 1;
			}
			break;
	}
	return 0;
}

int aux_is_printable(int c)
{
	if (c<' '||c>'~') return 0;
	return 1;
}
