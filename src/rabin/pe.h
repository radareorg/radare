#ifndef PE_H
#define PE_H

#define PE_Word unsigned short
#define PE_DWord unsigned int
#define PE_Byte unsigned char

typedef struct _PE_IMAGE_DOS_HEADER
{
    PE_Word  e_magic;      /* 00: MZ Header signature */
    PE_Word  e_cblp;       /* 02: Bytes on last page of file */
    PE_Word  e_cp;         /* 04: Pages in file */
    PE_Word  e_crlc;       /* 06: Relocations */
    PE_Word  e_cparhdr;    /* 08: Size of header in paragraphs */
    PE_Word  e_minalloc;   /* 0a: Minimum extra paragraphs needed */
    PE_Word  e_maxalloc;   /* 0c: Maximum extra paragraphs needed */
    PE_Word  e_ss;         /* 0e: Initial (relative) SS value */
    PE_Word  e_sp;         /* 10: Initial SP value */
    PE_Word  e_csum;       /* 12: Checksum */
    PE_Word  e_ip;         /* 14: Initial IP value */
    PE_Word  e_cs;         /* 16: Initial (relative) CS value */
    PE_Word  e_lfarlc;     /* 18: File address of relocation table */
    PE_Word  e_ovno;       /* 1a: Overlay number */
    PE_Word  e_res[4];     /* 1c: Reserved words */
    PE_Word  e_oemid;      /* 24: OEM identifier (for e_oeminfo) */
    PE_Word  e_oeminfo;    /* 26: OEM information; e_oemid specific */
    PE_Word  e_res2[10];   /* 28: Reserved words */
    PE_DWord e_lfanew;     /* 3c: Offset to extended header */
} PE_IMAGE_DOS_HEADER;

#define PE_IMAGE_NT_SIGNATURE 0x00004550

typedef struct _PE_IMAGE_FILE_HEADER {
  PE_Word  Machine;
  PE_Word  NumberOfSections;
  PE_DWord TimeDateStamp;
  PE_DWord PointerToSymbolTable;
  PE_DWord NumberOfSymbols;
  PE_Word  SizeOfOptionalHeader;
  PE_Word  Characteristics;
} PE_IMAGE_FILE_HEADER;

#define	PE_IMAGE_FILE_MACHINE_ARM		0x1c0
/* Directory Entries, indices into the DataDirectory array */

#define	PE_IMAGE_DIRECTORY_ENTRY_EXPORT		0
#define	PE_IMAGE_DIRECTORY_ENTRY_IMPORT		1
#define	PE_IMAGE_DIRECTORY_ENTRY_RESOURCE		2
#define	PE_IMAGE_DIRECTORY_ENTRY_EXCEPTION		3
#define	PE_IMAGE_DIRECTORY_ENTRY_SECURITY		4
#define	PE_IMAGE_DIRECTORY_ENTRY_BASERELOC		5
#define	PE_IMAGE_DIRECTORY_ENTRY_DEBUG		6
#define	PE_IMAGE_DIRECTORY_ENTRY_COPYRIGHT		7
#define	PE_IMAGE_DIRECTORY_ENTRY_GLOBALPTR		8   /* (MIPS GP) */
#define	PE_IMAGE_DIRECTORY_ENTRY_TLS		9
#define	PE_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10
#define	PE_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11
#define	PE_IMAGE_DIRECTORY_ENTRY_IAT		12  /* Import Address Table */
#define	PE_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT	13
#define	PE_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

typedef struct _PE_IMAGE_DATA_DIRECTORY {
  PE_DWord VirtualAddress;
  PE_DWord Size;
} PE_IMAGE_DATA_DIRECTORY ;

#define PE_IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _PE_IMAGE_OPTIONAL_HEADER {

  /* Standard fields */

  PE_Word  Magic;
  PE_Byte  MajorLinkerVersion;
  PE_Byte  MinorLinkerVersion;
  PE_DWord SizeOfCode;
  PE_DWord SizeOfInitializedData;
  PE_DWord SizeOfUninitializedData;
  PE_DWord AddressOfEntryPoint;
  PE_DWord BaseOfCode;
  PE_DWord BaseOfData;

  /* NT additional fields */

  PE_DWord ImageBase;
  PE_DWord SectionAlignment;
  PE_DWord FileAlignment;
  PE_Word  MajorOperatingSystemVersion;
  PE_Word  MinorOperatingSystemVersion;
  PE_Word  MajorImageVersion;
  PE_Word  MinorImageVersion;
  PE_Word  MajorSubsystemVersion;
  PE_Word  MinorSubsystemVersion;
  PE_DWord Win32VersionValue;
  PE_DWord SizeOfImage;
  PE_DWord SizeOfHeaders;
  PE_DWord CheckSum;
  PE_Word  Subsystem;
  PE_Word  DllCharacteristics;
  PE_DWord SizeOfStackReserve;
  PE_DWord SizeOfStackCommit;
  PE_DWord SizeOfHeapReserve;
  PE_DWord SizeOfHeapCommit;
  PE_DWord LoaderFlags;
  PE_DWord NumberOfRvaAndSizes;
  PE_IMAGE_DATA_DIRECTORY DataDirectory[PE_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} PE_IMAGE_OPTIONAL_HEADER ;

#define PE_IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _PE_IMAGE_SECTION_HEADER {
  PE_Byte  Name[PE_IMAGE_SIZEOF_SHORT_NAME];
  union {
    PE_DWord PhysicalAddress;
    PE_DWord VirtualSize;
  } Misc;
  PE_DWord VirtualAddress;
  PE_DWord SizeOfRawData;
  PE_DWord PointerToRawData;
  PE_DWord PointerToRelocations;
  PE_DWord PointerToLinenumbers;
  PE_Word  NumberOfRelocations;
  PE_Word  NumberOfLinenumbers;
  PE_DWord Characteristics;
} PE_IMAGE_SECTION_HEADER;

#define PE_IMAGE_SCN_CNT_CODE			0x00000020
#define PE_IMAGE_SCN_CNT_INITIALIZED_DATA		0x00000040
#define PE_IMAGE_SCN_CNT_UNINITIALIZED_DATA	0x00000080

#define	PE_IMAGE_SCN_LNK_OTHER			0x00000100
#define	PE_IMAGE_SCN_LNK_INFO			0x00000200
/* #define	PE_IMAGE_SCN_TYPE_OVER		0x00000400 - Reserved */
#define	PE_IMAGE_SCN_LNK_REMOVE			0x00000800
#define	PE_IMAGE_SCN_LNK_COMDAT			0x00001000

/* 						0x00002000 - Reserved */
/* #define PE_IMAGE_SCN_MEM_PROTECTED 		0x00004000 - Obsolete */
#define	PE_IMAGE_SCN_MEM_FARDATA			0x00008000

/* #define PE_IMAGE_SCN_MEM_SYSHEAP		0x00010000 - Obsolete */
#define	PE_IMAGE_SCN_MEM_PURGEABLE			0x00020000
#define	PE_IMAGE_SCN_MEM_16BIT			0x00020000
#define	PE_IMAGE_SCN_MEM_LOCKED			0x00040000
#define	PE_IMAGE_SCN_MEM_PRELOAD			0x00080000

#define	PE_IMAGE_SCN_ALIGN_1PE_ByteS			0x00100000
#define	PE_IMAGE_SCN_ALIGN_2PE_ByteS			0x00200000
#define	PE_IMAGE_SCN_ALIGN_4PE_ByteS			0x00300000
#define	PE_IMAGE_SCN_ALIGN_8PE_ByteS			0x00400000
#define	PE_IMAGE_SCN_ALIGN_16PE_ByteS			0x00500000  /* Default */
#define PE_IMAGE_SCN_ALIGN_32PE_ByteS			0x00600000
#define PE_IMAGE_SCN_ALIGN_64PE_ByteS			0x00700000
/* 						0x00800000 - Unused */

#define PE_IMAGE_SCN_LNK_NRELOC_OVFL		0x01000000

#define PE_IMAGE_SCN_MEM_DISCARDABLE		0x02000000
#define PE_IMAGE_SCN_MEM_NOT_CACHED		0x04000000
#define PE_IMAGE_SCN_MEM_NOT_PAGED			0x08000000
#define PE_IMAGE_SCN_MEM_SHARED			0x10000000
#define PE_IMAGE_SCN_MEM_EXECUTE			0x20000000
#define PE_IMAGE_SCN_MEM_READ			0x40000000
#define PE_IMAGE_SCN_MEM_WRITE			0x80000000

typedef struct _dietpe_pe_memfile 
{
	int fd;				// file fd
	unsigned char* file;		// file pointer
	int fsize;			// file size
	PE_IMAGE_DOS_HEADER* dos_header;
        PE_IMAGE_FILE_HEADER* file_header;
        PE_IMAGE_OPTIONAL_HEADER * opt_header;
        PE_IMAGE_SECTION_HEADER* section_headers;
} dietpe_pe_memfile;

#define PE_PACK_SECNAME ".caca"

#endif
