/* Author: esteve 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 *
 * TODO:
 *  * Refactor
 */


#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dietpe.h"

extern int rad;

int dietpe_list_sections( dietpe_pe_memfile* filein )
{
	int i;

    if (rad)
		printf("fs sections\n");
	else {
		printf("==> Sections:\n");
		for ( i = 0 ; i < filein->file_header->NumberOfSections ; i ++ )
		{
			printf ("[%02i] name: %.8s\n\tVirtual Address 0x%x \n", i, &filein->section_headers[i].Name,filein->section_headers[i].VirtualAddress ) ;
			printf ("\tVirtual Size: 0x%x\n",filein->section_headers[i].Misc.VirtualSize);
			printf ("\tsize: 0x%x\n",filein->section_headers[i].SizeOfRawData);
			printf ("\tfile offset: 0x%x\n",filein->section_headers[i].PointerToRawData );
			printf ("\tCharacteristics: 0x%x\n",filein->section_headers[i].Characteristics);
			if ( (filein->section_headers[i].Characteristics & 0x00000020 ) == 0x00000020 ) 
			{
				printf ("\tCODE\n");
			}
			if ( (filein->section_headers[i].Characteristics & PE_IMAGE_SCN_MEM_DISCARDABLE ) == PE_IMAGE_SCN_MEM_DISCARDABLE ) 
			{
				printf ("\tDiscarded\n");
			}
			printf ("\n");
		}
	}
	return i;
}

void dietpe_clean_discard_sections ( dietpe_pe_memfile* filein )
{
	int i,j;
	unsigned int nsect ;
	nsect = filein->file_header->NumberOfSections;
	for ( i = 0 ; i < nsect ; i ++ )
	{
		if ( ( filein->section_headers[i].Characteristics & PE_IMAGE_SCN_MEM_DISCARDABLE ) == PE_IMAGE_SCN_MEM_DISCARDABLE )
		{
			// this section can be discarded
			for ( j = i ; j < nsect ; j ++ )
			{
				memcpy ( &filein->section_headers[j], &filein->section_headers[j+1], sizeof (PE_IMAGE_SECTION_HEADER) );
			}	
			nsect --;
			i--;
			// res referent a i!!
		}
		// res referent a i!!
	}
	
	filein->file_header->NumberOfSections = nsect;
}

void dietpe_set_section_characteristics ( dietpe_pe_memfile* filein , unsigned char* secname , unsigned int chars) 
{
	int i;
	unsigned int nsect ;
	unsigned int addr;

	nsect = filein->file_header->NumberOfSections;
	
	for ( i = 0 ; i < nsect ; i ++ )
	{
		if ( !strncmp( filein->section_headers[i].Name, secname , 8) )
		{	
			filein->section_headers[i].Characteristics = chars;
			break;
		}
	}
	
}


PE_IMAGE_SECTION_HEADER dietpe_get_section_byname ( dietpe_pe_memfile* filein , unsigned char* secname ) 
{
	int i;
	unsigned int nsect ;
	unsigned int addr;

	nsect = filein->file_header->NumberOfSections;
	
	for ( i = 0 ; i < nsect ; i ++ )
	{
		if ( !strncmp( filein->section_headers[i].Name, secname , 8) )
		{
			return filein->section_headers[i];
		}
	}
	
}

int dietpe_clean_section_byname ( dietpe_pe_memfile* filein , unsigned char* secname ) 
{
	int i,j;
	unsigned int nsect ;

	nsect = filein->file_header->NumberOfSections;
	
	for ( i = 0 ; i < nsect ; i ++ )
	{
		if ( !strncmp( filein->section_headers[i].Name, secname , 8) )
		{
			for ( j = i ; j < nsect ; j ++ )
			{
				memcpy ( &filein->section_headers[j], &filein->section_headers[j+1], sizeof (PE_IMAGE_SECTION_HEADER) );
			}	
			filein->file_header->NumberOfSections --;
	
			return 0;
		}
	}
	return 1;
}

unsigned int dietpe_set_entrypoint_to_address ( dietpe_pe_memfile* filein , unsigned int address ) 
{
	filein->opt_header->AddressOfEntryPoint = ( address - filein->opt_header->ImageBase );

}

unsigned int dietpe_set_entrypoint_to_section ( dietpe_pe_memfile* filein , unsigned char* secname ) 
{
	int i;
	unsigned int nsect ;
	unsigned int addr;

	nsect = filein->file_header->NumberOfSections;
	
	for ( i = 0 ; i < nsect ; i ++ )
	{
		if ( !strncmp( filein->section_headers[i].Name, secname , 8) )
		{
			addr = filein->section_headers[i].VirtualAddress;
			break;
		}
	}
	filein->opt_header->AddressOfEntryPoint = addr;
	
}

int dietpe_add_section_pefile ( dietpe_pe_memfile* filein , unsigned char* secname , int size , unsigned int characteristics , char* code) 
{
	
	unsigned int off_last;
	unsigned int voff_last;
	int i;
	int headersize;
	PE_IMAGE_SECTION_HEADER newsec;

	int nsect = filein->file_header->NumberOfSections - 1;
		
	//tinc prou espai per afegir seccions ?
	headersize = filein->opt_header->SizeOfHeaders ;
		
	if ( ( (int)&filein->section_headers[nsect] - (int)filein->file ) >= ( (int)headersize - sizeof (PE_IMAGE_SECTION_HEADER) ) )
	{
		printf ("Not enough space to add section\n");
		return 1;
	}

	size = size + ( filein->opt_header->FileAlignment -  ( size % filein->opt_header->FileAlignment ) );

	// final de l'ultim section header
	off_last = filein->section_headers[nsect].PointerToRawData +  filein->section_headers[nsect].SizeOfRawData  ;
	off_last = off_last + ( filein->opt_header->FileAlignment - ( off_last % filein->opt_header->FileAlignment ) );
	
	voff_last = filein->section_headers[nsect].VirtualAddress +  filein->section_headers[nsect].SizeOfRawData  ;
	voff_last = voff_last + ( filein->opt_header->SectionAlignment - ( voff_last % filein->opt_header->SectionAlignment ) );

	//printf (" at : 0x%x\n", off_last);
	//printf (" at : 0x%x\n", voff_last);

	strncpy ( newsec.Name , secname , 8 );
	newsec.Misc.VirtualSize = ( size + ( filein->opt_header->SectionAlignment - ( size % filein->opt_header->SectionAlignment ) ) );
	newsec.VirtualAddress = voff_last ;
	newsec.SizeOfRawData = size;
	newsec.PointerToRawData = off_last ;
	newsec.PointerToRelocations = 0;
	newsec.PointerToLinenumbers = 0;
	newsec.NumberOfRelocations = 0;
	newsec.NumberOfLinenumbers = 0;
	newsec.Characteristics = characteristics ;
	//filein->opt_header->SizeOfHeaders
	

	// Afegim la seccio
	filein->opt_header->SizeOfCode += size;
	filein->opt_header->SizeOfImage += size;
	filein->opt_header->CheckSum = 0;

	//filein->opt_header->AddressOfEntryPoint = voff_last;

	nsect++;
	filein->file_header->NumberOfSections++;
	memcpy ( &filein->section_headers[nsect], &newsec, sizeof (PE_IMAGE_SECTION_HEADER ) );
	

	// copiem les dades al fitxer
	if ( code != NULL )
	{
		memcpy ( filein->file + off_last, code, size );
	}

	off_last = filein->section_headers[nsect].PointerToRawData +  filein->section_headers[nsect].SizeOfRawData  ;
	off_last = off_last + ( filein->opt_header->FileAlignment - ( off_last % filein->opt_header->FileAlignment ) );
	
	filein->fsize = off_last ;

	return 0;
}

int dietpe_init_pefile ( dietpe_pe_memfile* filein )
{

	filein->dos_header =(PE_IMAGE_DOS_HEADER*) filein->file;
	if ( memcmp ( &filein->dos_header->e_magic, "MZ", 2 ) )
	{
		printf ("NOT a PE file \n" );
		return 1;
	}
	
	if ( *(unsigned int*)( filein->file + filein->dos_header->e_lfanew) != PE_IMAGE_NT_SIGNATURE )
	{
		printf ("IMAGE_NT_SIGNATURE not found \n");
		return 1;
	}
	
	filein->file_header = (PE_IMAGE_FILE_HEADER*) ( filein->file + filein->dos_header->e_lfanew + 4);
	filein->opt_header = (PE_IMAGE_OPTIONAL_HEADER*)((int)filein->file_header + sizeof (PE_IMAGE_FILE_HEADER ));
	filein->section_headers = (PE_IMAGE_SECTION_HEADER*) ( (int)filein->opt_header + sizeof (PE_IMAGE_OPTIONAL_HEADER ) ) ;

	return 0;
}	

void dietpe_list_info ( dietpe_pe_memfile* filein )
{
	int i;

	printf ("DOS header:\n");
	printf (" e_magic:                 %.2s\n", &filein->dos_header->e_magic );	
	printf (" e_sp:                    %x\n", &filein->dos_header->e_sp );	
	printf ("File header:\n");
	if ( filein->file_header->Machine == PE_IMAGE_FILE_MACHINE_ARM )
	{
		printf ( " Machine:                 ARM\n");
	}
	else
	{
		printf ( " Machine:                 0x%x\n", filein->file_header->Machine );
	//	return 1;
	}

	printf ( " Number of sections:      %d\n", filein->file_header->NumberOfSections);
	printf ( " Size of optional header: %d\n", filein->file_header->SizeOfOptionalHeader);
	printf ( " Pointer to symbol table: %x\n", filein->file_header->PointerToSymbolTable);
	
	
	if ( filein->opt_header->Magic == 0x010b )
	{
		printf ("Opt header:\n" );	

		
		printf (" Entry point RVA:         0x%x\n",filein->opt_header->AddressOfEntryPoint );
		printf (" Prefered Image Base:     0x%x\n",filein->opt_header->ImageBase );
		printf (" Section alignment:       %d\n",filein->opt_header->SectionAlignment );
		printf (" File alignment:          %d\n",filein->opt_header->FileAlignment );
		printf (" Size of Image:           %d\n",filein->opt_header->SizeOfImage );
		printf (" Check sum:               0x%x\n",filein->opt_header->CheckSum );
		printf (" First section at:        0x%x\n",filein->opt_header->SizeOfHeaders );	

		printf (" Data directory:\n" );	
		for ( i = 0 ; i < PE_IMAGE_NUMBEROF_DIRECTORY_ENTRIES ; i ++ ) 
		{
			if ( filein->opt_header->DataDirectory[i].Size != 0 )
			{
				printf ("  Directory %d, RVA 0x%x, size 0x%x\n",i,filein->opt_header->DataDirectory[i].VirtualAddress,filein->opt_header->DataDirectory[i].Size);
			}
		}

	}
	else
	{
		printf ("NO optional header\n");
	}
	

}

int dietpe_get_pe_section_byname ( dietpe_pe_memfile* filein ,  char* secname )
{
	unsigned int nsect,i;
	 nsect = filein->file_header->NumberOfSections;
        
        for ( i = 0 ; i < nsect ; i ++ )
        {
                if ( !strncmp( filein->section_headers[i].Name, secname , 8) )
                {       
                        break;
                }
        }
	
	
	return ( ( i >= 0) ? i : -1 ) ;
}

unsigned int dietpe_get_entrypoint ( dietpe_pe_memfile* filein ) 
{
	return filein->opt_header->AddressOfEntryPoint ;
}

int dietpe_new(dietpe_pe_memfile *filein, const char *file)
{
	filein->fd = open ( file, O_RDONLY );
	lseek ( filein->fd, 0, SEEK_END);
	filein->fsize = lseek ( filein->fd, 0, SEEK_CUR);
	lseek ( filein->fd, 0, SEEK_SET);

	filein->file = (unsigned char*) mmap ( 0, filein->fsize, PROT_READ, MAP_PRIVATE, filein->fd, 0 );
	
	close(filein->fd);

	if ( filein->file == NULL )
	{
		printf ("ERROR mapping \n");
		return 1;
	}
	
	if ( dietpe_init_pefile ( filein ) != 0 )
	{
		return 1;
	}

	return 0;
}

#if 0
unsigned char auxbuff[1024*1024*5];
unsigned char code[5*1024*1024];
int main ( int argc , char** argv)
{
	int i;
	int fdo;

	pe_memfile filein;
	pe_memfile fileout;	

	arm_code_desc codedesc ;
	if ( argc !=2 )
	{
		printf ("USAGE fname\n");
		return 1;
	}

	filein.fd = open ( argv[1], O_RDONLY );
	lseek ( filein.fd, 0, SEEK_END);
	filein.fsize = lseek ( filein.fd, 0, SEEK_CUR);
	lseek ( filein.fd, 0, SEEK_SET);

	filein.file = (unsigned char*) mmap ( 0, filein.fsize, PROT_READ, MAP_PRIVATE, filein.fd, 0 );
	
	if ( filein.file == NULL )
	{
		printf ("ERROR mapping \n");
		return 1;
	}

	
	if ( init_pefile ( &filein ) != 0 )
	{
		return 1;
	}

	print_info ( &filein ) ;

	/*	
	printf ("\n-----------------------------------\n");
	memcpy ( auxbuff , filein.file , filein.fsize );
		
	fileout.file = auxbuff ;
	fileout.fsize =  filein.fsize ;

	if ( init_pefile ( &fileout ) != 0 )
	{
		return 1;
	}


	//clean_discard_sections ( &fileout ) ;
	add_section_pefile ( &fileout , PACK_SECNAME , 	4096 , IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_CODE, NULL) ;

	IMAGE_SECTION_HEADER packsec =  get_section_byname ( &fileout , PACK_SECNAME );
	IMAGE_SECTION_HEADER codesection =  get_section_byname ( &fileout , ".text" );
	
	
	
	clean_section_byname ( &fileout , PACK_SECNAME );
	set_section_characteristics ( &fileout , ".text" , IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | 
							   IMAGE_SCN_CNT_INITIALIZED_DATA  ); 


	add_section_pefile ( &fileout , PACK_SECNAME , 40*1024 , IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_WRITE , code) ;
	

	set_entrypoint_to_address ( &fileout  , arm_get_label_addr (&armseq, "main") ) ;

//	print_info ( &fileout ) ;
	

	fdo = open ( "out.exe", O_CREAT|O_WRONLY|O_TRUNC, 00666 ) ;
	write ( fdo , fileout.file, fileout.fsize );
	*/
	return 0;
}
#endif
