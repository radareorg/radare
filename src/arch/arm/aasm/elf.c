/* AASM - ARM assembler  Alpha+ version  J. Garside  UofM 26/7/07             */
/*      - adapted into radare by pancake <youterm.com> */
/* LICENSE: GPL */

#include "aasm.h"

void elf_dump(unsigned int address, char value)
{
	elf_temp *pTemp;

	elf_section_valid = TRUE;    /* Note that we've dumped -something- in section */

	if (elf_new_block || (elf_section != elf_section_old))
	{                                              /* New or unexpected address */
		pTemp = (elf_temp*) malloc(ELF_TEMP_SIZE);                /* Allocate block */
		pTemp->pNext        = NULL;                        /* Initialise new record */
		pTemp->continuation = (elf_section == elf_section_old);
		pTemp->section      = elf_section;
		pTemp->address      = address;
		pTemp->count        = 0;

		if (current_elf_record == NULL)
		{                                                         /* First record */
			elf_record_list = pTemp;
			pTemp->continuation = FALSE;
		}
		else current_elf_record->pNext = pTemp;

		current_elf_record = pTemp;                                      /* Move on */
	}

	current_elf_record->data[(current_elf_record->count)++] = value;

	elf_section_old = elf_section;

	elf_new_block = (current_elf_record->count >= ELF_TEMP_LENGTH);

	return;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Start a new ELF section if one is already in use                           */

void elf_new_section_maybe(void)
{
	if (elf_section_valid) { elf_section++; elf_section_valid = FALSE; }
	return;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	static void elf_dump_word(FILE *fElf, unsigned int word)
	{
		fprintf(fElf, "%c%c%c%c", word & 0xFF, (word>>8) & 0xFF,
				(word>>16) & 0xFF, (word>>24) & 0xFF);
		return;
	}
	static void elf_dump_SH(FILE *fElf, unsigned int name, unsigned int type,
			unsigned int flags, unsigned int addr, unsigned int pos, unsigned int size,
			unsigned int link, unsigned int info, unsigned int align, unsigned int size2)
	{
		elf_dump_word(fElf, name);   elf_dump_word(fElf, type);
		elf_dump_word(fElf, flags);  elf_dump_word(fElf, addr);
		elf_dump_word(fElf, pos);    elf_dump_word(fElf, size);
		elf_dump_word(fElf, link);   elf_dump_word(fElf, info);
		elf_dump_word(fElf, align);  elf_dump_word(fElf, size2);
		return;
	}
void elf_dump_out(FILE *fElf, sym_table *table)
{




	elf_temp *pTemp, *pTemp_old;
	elf_info *pInfo_head, *pInfo, *pInfo_new;
	unsigned int fragments, total, pad_to_align, i, j, temp;
	unsigned int symtab_count, symtab_length, strtab_length, shstrtab_length;
	unsigned int symtab_local_count;
	unsigned int prog_start;
	unsigned int code_SHstr_offset,   sym_SHstr_offset;
	unsigned int  str_SHstr_offset, SHstr_SHstr_offset;
	char *strings, *SHstrings;
	sym_record *head, *ptr;

	char *sym_sectionname = "symtab";                 /* Predefined section names */
	char *str_sectionname = "strtab";
	char *shs_sectionname = "shstrtab";


	pInfo      = NULL;
	pInfo_head = NULL;    /* Needed in case there's nothing to write (e.g. error) */
	pTemp      = elf_record_list;
	fragments  = 0;                                             /* Number of ORGs */
	total      = 0;                                   /* Length of all code bytes */

	while (pTemp != NULL)                     /* Make an code output section list */
	{
		if (!pTemp->continuation)
		{
			pInfo_new = (elf_info*) malloc(ELF_INFO_SIZE);          /* Allocate block */
			pInfo_new->pNext    = NULL;
			pInfo_new->address  = pTemp->address;
			pInfo_new->position = total;
			if (pInfo == NULL) pInfo_head   = pInfo_new;        /* Start ...          */
			else               pInfo->pNext = pInfo_new;        /* ... or add to list */

			pInfo = pInfo_new;
			pInfo->size = 0;
			fragments++;
		}
		pInfo->size  = pInfo->size + pTemp->count;
		total = total + pTemp->count;
		pTemp = pTemp->pNext;
	}

	head = sym_sort_symbols(table, ALL, FOR_ELF);       /* Make temp. symbol list */

	prog_start         = ELF_EHSIZE;
	symtab_count       = sym_count_symbols(table, ALL) + 1;  /* Number of entries */
	/* " + 1" is for dummy first entry */
	symtab_local_count = symtab_count - sym_count_symbols(table, EXPORTED);
	strings            = sym_strtab(head, symtab_count, &strtab_length);

	shstrtab_length = 1;                              /* Build shstrtab in memory */
	i = 0; do { shstrtab_length++; } while (elf_file_name[i++]   != '\0');
	i = 0; do { shstrtab_length++; } while (sym_sectionname[i++] != '\0');
	i = 0; do { shstrtab_length++; } while (str_sectionname[i++] != '\0');
	i = 0; do { shstrtab_length++; } while (shs_sectionname[i++] != '\0');
	shstrtab_length = (shstrtab_length + 3) & 0xFFFFFFFC;

	SHstrings = (char*) malloc(shstrtab_length);                /* Allocate block */
	j = 0;                                                          /* Fill block */
	SHstrings[j++]  = '\0';
	code_SHstr_offset = j;   i = 0; 
	do {SHstrings[j++] = elf_file_name[i];}   while (elf_file_name[i++]   != '\0');
	sym_SHstr_offset = j;    i = 0;
	do {SHstrings[j++] = sym_sectionname[i];} while (sym_sectionname[i++] != '\0');
	str_SHstr_offset = j;    i = 0;
	do {SHstrings[j++] = str_sectionname[i];} while (str_sectionname[i++] != '\0');
	SHstr_SHstr_offset = j;  i = 0;
	do {SHstrings[j++] = shs_sectionname[i];} while (shs_sectionname[i++] != '\0');
	while ((j & 3)!=0) SHstrings[j++]='\0';                  /* Pad to word align */

	pad_to_align    = -(total % 4) & 3;
	total           = total + pad_to_align;                         /* Word align */
	strtab_length   = (strtab_length + 3) & 0xFFFFFFFC;             /* Word align */
	symtab_length   = 16 * symtab_count;                           /* True length */

	elf_dump_word(fElf, 0x7F | ('E'<<8) | ('L'<<16) | ('F'<<24));  /* File header */
	elf_dump_word(fElf, 0x00010101);
	elf_dump_word(fElf, 0);
	elf_dump_word(fElf, 0);
	elf_dump_word(fElf, 2 + (ELF_MACHINE << 16));
	elf_dump_word(fElf, 1);
	elf_dump_word(fElf, entry_address);
	elf_dump_word(fElf, prog_start + total + symtab_length + strtab_length
			+ shstrtab_length);
	elf_dump_word(fElf, prog_start + total + symtab_length + strtab_length
			+ shstrtab_length + (ELF_PHENTSIZE * fragments));
	elf_dump_word(fElf, 0);			// Flags @@@
	elf_dump_word(fElf, ELF_EHSIZE      +   (ELF_PHENTSIZE << 16));
	elf_dump_word(fElf, fragments       +   (ELF_SHENTSIZE << 16));
	elf_dump_word(fElf, (fragments + 4) + ((fragments + 3) << 16));	// @@@

	pTemp = elf_record_list;

	while (pTemp != NULL)                               /* Dump the code sections */
	{
		for (i = 0; i < pTemp->count; i++) { fprintf(fElf, "%c", pTemp->data[i]); }
		pTemp = pTemp->pNext;
	}
	for (i = 0; i < pad_to_align; i++) fprintf(fElf, "%c", 0);           /* Align */

	/* Symbol table - values et alia */
	for (i = 0; i < 4; i++) elf_dump_word(fElf, 0);         /* Dummy first symbol */

	ptr = head;
	j   = 0;
	for (i = 1; i < symtab_count; i++)
	{
		while (strings[j++] != '\0');                     /* Point beyond next '\0' */
		elf_dump_word(fElf, j);
		elf_dump_word(fElf, ptr->value);
		elf_dump_word(fElf, 0x00000000);

		if ((ptr->flags & SYM_REC_EXPORT_FLAG) == 0) temp = 0x00; else temp = 0x10;
		/* Binding */
		if ((ptr->flags & SYM_REC_EQU_FLAG) == 0)
			elf_dump_word(fElf, (ptr->elf_section << 16) | temp);
		else                                          /* EQU, so regard as absolute */
			elf_dump_word(fElf, (ELF_SHN_ABS      << 16) | temp);
		ptr = ptr->pNext;
	}

	sym_delete_record_list(&head, FALSE);               /* Destroy temporary list */

	//printf("SHStrtab start:  %08X\n", ELF_EHSIZE+total+symtab_length+strtab_length);

	for (i = 0; i < strtab_length; i++)   fprintf(fElf, "%c", strings[i]);
	for (i = 0; i < shstrtab_length; i++) fprintf(fElf, "%c", SHstrings[i]);

	pInfo = pInfo_head;

	for (j = 0; j < fragments; j++)					// PHeader	@@@@@
	{                      // Should one -segment- magically cover all these -sections- ?@@@
		elf_dump_word(fElf, 1);
		elf_dump_word(fElf, ELF_EHSIZE + pInfo->position);
		elf_dump_word(fElf, pInfo->address);
		elf_dump_word(fElf, pInfo->address);
		elf_dump_word(fElf, pInfo->size);
		elf_dump_word(fElf, pInfo->size);
		elf_dump_word(fElf, 0x00000000);
		elf_dump_word(fElf, 1);
		pInfo = pInfo->pNext;
	}

	elf_dump_SH(fElf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);        /* Dump section table */

	pInfo = pInfo_head;
	for (i = 0; i < fragments; i++)
	{
		elf_dump_SH(fElf, code_SHstr_offset, 1, 0x07, pInfo->address, 
				ELF_EHSIZE + pInfo->position, pInfo->size, 0, 0, 0, 0);
		pInfo = pInfo->pNext;
	}

	elf_dump_SH(fElf, sym_SHstr_offset, 2, 0, 0,ELF_EHSIZE + total,
			symtab_length, fragments+2, symtab_local_count, 0, 16);	// @@@

	elf_dump_SH(fElf, str_SHstr_offset, 3, 0, 0,ELF_EHSIZE + total + symtab_length,
			strtab_length, 0, 0, 0, 0);

	elf_dump_SH(fElf, SHstr_SHstr_offset, 3, 0, 0,
			ELF_EHSIZE + total + symtab_length + strtab_length,
			shstrtab_length, 0, 0, 0, 0);

	close_output_file(fElf, elf_file_name, pass_errors != 0);

	/* Trash temporary data structures */
	pInfo = pInfo_head;

	while (pInfo != NULL) { pInfo_new=pInfo->pNext; free(pInfo); pInfo=pInfo_new; }

	pTemp = elf_record_list;
	while (pTemp != NULL) { pTemp_old=pTemp; pTemp=pTemp->pNext; free(pTemp_old); }

	free(strings);
	free(SHstrings);

	return;
}
