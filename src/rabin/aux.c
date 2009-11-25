/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include <stdio.h>
#include <string.h>
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

int aux_bin2str(const u8 *in, int len, char *out)
{
	int i;
	char tmp[5];
	out[0]='\0';
	for(i=0;i<len;i++)  {
		sprintf(tmp, "%02x", in[i]);
		strcat(out, tmp);
	}
	return len;
}

int str_cpy(char *dst, const char *org)
{
	int i = 0;
	if (org&&*org)
	do { *dst++=*org++;
	} while (*org && ++i);
	*dst=0;
	return i;
}

ut64 get_offset(const char *orig)
{
	char *arg;
	ut64 ret = 0;
	int i, j;
	if (orig==NULL||orig[0]=='\0')
		return 0;

	while(*orig==' '&&*orig) orig++;
	arg = alloca(strlen(orig)+32);
	str_cpy(arg, orig);

	/* single char 'A' */
	if (arg[0]=='\'' && arg[0+2]=='\'')
		return arg[0+1];

	for(;*arg==' ';arg=arg+1);
	for(i=0;arg[i]==' ';i++);
	for(;arg[i]=='\\';i++); i++;

	if (arg[i] == 'x' && i>0 && arg[i-1]=='0') {
		sscanf(arg, "0x"OFF_FMTx, &ret);
	} else {
		sscanf(arg, OFF_FMTd, &ret);

		switch(arg[strlen(arg)-1]) {
		case 'f': // float
			{
			float f;
			sscanf(arg, "%f", &f);
			memcpy(&ret, &f, sizeof(f));
			}
			break;
		case 'o': // octal
			sscanf(arg, "%llo", &ret);
			break;
		case 'b': // binary
			ret = 0;
			for(j=0,i=strlen(arg)-2;i>=0;i--,j++) {
				if (arg[i]=='1') ret|=1<<j; else
				if (arg[i]!='0') break;
			}
			break;
		case 'K': case 'k':
			ret*=1024;
			break;
		case 'M': case 'm':
			ret*=1024*1024;
			break;
		case 'G': case 'g':
			ret*=1024*1024*1024;
			break;
		}
	}

	return ret;
}
