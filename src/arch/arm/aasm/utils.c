/* AASM - ARM assembler  Alpha+ version  J. Garside  UofM 26/7/07             */
/*      - adapted into radare by pancake <youterm.com> */
/* LICENSE: GPL */

#include "aasm.h"

/*----------------------------------------------------------------------------*/

int skip_spc(char *line, int position)
{
	while ((line[position] == ' ') || (line[position] == '\t')) position++;
	return position;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Makes a copy of "filename" with the last element stripped back to '/'      */
/* Returns null string if no '/' present.                                     */
/* Allocates space for new string.                                            */

char *file_path(char *filename)
{
	char *pPath;
	int position;

	position = strlen(filename);              /* Find back-end of original string */
	while ((position > 0) && (filename[position - 1] != '/')) position--;

	pPath = (char*) malloc(position+1);         /* Allocate space for path + '\0' */

	pPath[position] = '\0';                       /* Insert terminator, step back */
	while (position > 0)                                   /* Copy rest of string */
	{
		position = position - 1;
		pPath[position] = filename[position];
	}

	return pPath;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Concatenates source strings and returns newly allocated string.            */

char *pathname(char *name1, char *name2)
{
	char *pResult;

	pResult = (char*) malloc(strlen(name1) + strlen(name2) + 1);     /* Make room */
	strcpy(pResult, name1);                               /* Copy in first string */
	strcat(pResult, name2);                               /* Append second string */

	return pResult;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Returns Boolean - true if `character' is found                             */
/* If character is found it is stripped                                       */

boolean cmp_next_non_space(char *line, int *pPos, int offset, char character)
{
	boolean result;

	*pPos = skip_spc(line, *pPos +  offset);    /* Strip, possibly skipping first */
	result = (line[*pPos] == character);
	if (result) (*pPos)++;                                /* Strip test character */
	return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Returns Boolean - true if `character' is valid end of statement            */

boolean test_eol(char character)
{
	return (character == '\0') || (character == ';') || (character == '\n');
}

boolean alpha_numeric(char c)                                       /* Crude! */
{
	return (((c >= '0') && (c <= '9')) || alphabetic(c));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

boolean alphabetic(char c)                                          /* Crude! */
{
	return ((c == '_') || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Read number of specified radix into variable indicated by *value           */
/* Return flag to say number read (value at pointer).                         */

int get_num(char *line, int *position, int *value, unsigned int radix)
{

	int num_char(char *line, int *pos, unsigned int radix)
	{                          /* Return value(c) if in radix  else return -1 */
		char c;

		while ((c = line[*pos]) == '_') (*pos)++;          /* Allow & ignore  '_' */
		if (c < '0') return -1;
		if (c >= 'a') c = c & 0xDF;                         /* Upper case convert */
		if (c <= '9') { if (c < '0'+ radix) return c - '0';     /* Number < radix */
			else                return -1; }        /* Number > radix */
		else          { if (c < 'A')                   return -1;   /* Not letter */
			else if (c < 'A' + radix - 10) return c - 'A' + 10;
			else                      return -1; }
	}

	int i, new_digit;
	boolean found;

	i = skip_spc(line, *position);
	*value = 0;
	found  = FALSE;

	while ((new_digit = num_char(line, &i, radix)) >= 0)
	{
		*value = (*value * radix) + new_digit;
		found = TRUE;
		i++;
	}

	if (found) *position = i;                                     /* Move pointer */
	return found;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void list_hex(unsigned int number, unsigned int length, char *destination)
{
	int i, digit;

	for (i = 0; i < length; i++)
	{
		digit = (number >> ( 4 * (length - i - 1)) ) & 0xF;
		if (digit < 10) destination[i] = '0' + digit;
		else            destination[i] = 'A' + digit - 10;
	}
	return;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void hexpairs_dump(unsigned int address, char value)
{
	printf("%02x ", (unsigned char) value);
#if 0
	int i;

	if (hex_address_defined && (address == hex_address))
	{                                                       /* Expected address */
		list_hex(value, 2, &hex_buffer[HEX_LINE_ADDRESS+3*(address%HEX_BYTE_COUNT)]);
		hex_address++;
	}
	else
	{                                              /* New or unexpected address */
		if (hex_address_defined) fprintf(fHex, "%s\n", hex_buffer);

		for (i = 0; i < HEX_LINE_LENGTH - 1; i++) hex_buffer[i] = ' ';
		hex_buffer[i] = '\0';
		list_hex(address, 8, &hex_buffer[0]);
		hex_buffer[8] = ':';

		list_hex(value, 2, &hex_buffer[HEX_LINE_ADDRESS+3*(address%HEX_BYTE_COUNT)]);

		hex_address = address + 1;
		hex_address_defined = TRUE;
	}

	if ((hex_address % HEX_BYTE_COUNT) == 0)              /* If end of line, dump */
	{
		fprintf(fHex, "%s\n", hex_buffer);
		hex_address_defined = FALSE;
	}

	return;
#endif
}
void hex_dump(unsigned int address, char value)
{
	int i;

	if (hex_address_defined && (address == hex_address))
	{                                                       /* Expected address */
		list_hex(value, 2, &hex_buffer[HEX_LINE_ADDRESS+3*(address%HEX_BYTE_COUNT)]);
		hex_address++;
	}
	else
	{                                              /* New or unexpected address */
		if (hex_address_defined) fprintf(fHex, "%s\n", hex_buffer);

		for (i = 0; i < HEX_LINE_LENGTH - 1; i++) hex_buffer[i] = ' ';
		hex_buffer[i] = '\0';
		list_hex(address, 8, &hex_buffer[0]);
		hex_buffer[8] = ':';

		list_hex(value, 2, &hex_buffer[HEX_LINE_ADDRESS+3*(address%HEX_BYTE_COUNT)]);

		hex_address = address + 1;
		hex_address_defined = TRUE;
	}

	if ((hex_address % HEX_BYTE_COUNT) == 0)              /* If end of line, dump */
	{
		fprintf(fHex, "%s\n", hex_buffer);
		hex_address_defined = FALSE;
	}

	return;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void hex_dump_flush(void)
{
	if (hex_address_defined) fprintf(fHex, "%s\n", hex_buffer);
	hex_address_defined = FALSE;
	return;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void byte_dump(unsigned int address, unsigned int value, char *line, int size)
{
	int i;

	if (dump_code)
	{
		if (fList != NULL) list_mid_line(value, line, size);

		for (i = 0; i < size; i++)
		{
			if (fHex != NULL) hex_dump(address + i, (value >> (8*i)) & 0xFF);
			if (fElf != NULL) elf_dump(address + i, (value >> (8*i)) & 0xFF);
			if (hexpairs_stdout) hexpairs_dump(address + i, (value >> (8*i)) & 0xFF);
		}
	}
	return;
}
