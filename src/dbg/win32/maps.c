// debug_init_maps.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <winbase.h>
#include <psapi.h>


// struct list_head list;
typedef struct {
	unsigned long ini;
	unsigned long end;
	unsigned long perms, perms_orig;
	int flags;
	char *bin;
	unsigned long size;
} MAP_REG;

struct food_t 
{
	int pid;
};
#define bool int

struct food_t *ProcessStruct;

int debug_init_maps(int rest);
bool CheckValidPE(unsigned char * PeHeader);


void (*gmbn)(HANDLE, HMODULE, LPTSTR, int) = NULL;
void (*gmi)(HANDLE, HMODULE, LPMODULEINFO, int) = NULL;

int main(int argc, char *argv[])
{

	gmbn = GetProcAddress(GetModuleHandle("psapi"), "GetModuleBaseName");
	gmi = GetProcAddress(GetModuleHandle("psapi"), "GetModuleInformation");

	if (gmbn == NULL || gmi == NULL) {
		printf("Cannot open required symbols \n");
		return 0;
	}

	ProcessStruct = (struct food_t *)malloc(sizeof(struct food_t));
	memset(ProcessStruct, 0, sizeof(struct food_t));
	ProcessStruct->pid = atoi(argv[1]);
	debug_init_maps(1);

	return 1;
}


int debug_init_maps(int rest)
{
	MAP_REG *mr;
	HANDLE hProcess; 
	SYSTEM_INFO SysInfo;
	MEMORY_BASIC_INFORMATION mbi;
	LPBYTE CurrentPage;
	char *ModuleName = NULL;
	unsigned char *PeHeader = NULL;
	unsigned char * p;
	MODULEINFO ModInfo;
	int i;


	ModuleName = (char *) malloc(MAX_PATH);        


	mr = (MAP_REG *)malloc(sizeof(MAP_REG));
	if(!mr) 
	{
		printf(":map_reg alloc");
		return -1; 
	}

	hProcess = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessStruct->pid);  
	if(!hProcess)
	{
		printf("\n No permissions...\n");       
		return -1;
	}

	//obetenemos lpMinimumApplicationAddress y lpMaximumApplicationAddress
	GetSystemInfo(&SysInfo);

	for (CurrentPage = (LPBYTE) SysInfo.lpMinimumApplicationAddress ; CurrentPage < (LPBYTE) SysInfo.lpMaximumApplicationAddress;)
	{
		if (!VirtualQueryEx (hProcess, CurrentPage, &mbi, sizeof (mbi))) 
		{
			printf("\nVirtualQueryEx ERROR, address = 0x%08X", CurrentPage );
			break;
		}

		if(mbi.Type == MEM_IMAGE)
		{
			memset(ModuleName, 0, MAX_PATH);    	        	    
			//GetModuleBaseName(hProcess, (HMODULE) CurrentPage, (LPTSTR) ModuleName, MAX_PATH);
			gmbn(hProcess, (HMODULE) CurrentPage, (LPTSTR) ModuleName, MAX_PATH);

			PeHeader = (unsigned char *) malloc(0x400);				      
			ReadProcessMemory(hProcess, (const void *)CurrentPage, PeHeader, 0x400, NULL);

			IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)PeHeader;
			IMAGE_NT_HEADERS *nt_headers;
			IMAGE_SECTION_HEADER *SectionHeader;

			if(CheckValidPE(PeHeader))
			{
				nt_headers = (IMAGE_NT_HEADERS *)((char *)dos_header
						+ dos_header->e_lfanew);    	    		    	
			}

			int NumSections = nt_headers->FileHeader.NumberOfSections;    	        	        	    

			SectionHeader = (IMAGE_SECTION_HEADER *) ((char *)nt_headers
					+ sizeof(IMAGE_NT_HEADERS));

			for(i = 0; i < NumSections;i++)
			{
				mr->ini = SectionHeader->VirtualAddress;
				mr->ini += (unsigned long)CurrentPage;
				mr->size = SectionHeader->Misc.VirtualSize;
				mr->end = mr->ini + mr->size;
				mr->perms = SectionHeader->Characteristics;
				mr->bin = ModuleName;

				SectionHeader++;
			}    	        	        	        	    

			//GetModuleInformation(hProcess, (HMODULE) CurrentPage, (LPMODULEINFO) &ModInfo, sizeof(MODULEINFO));
			gmi(hProcess, (HMODULE) CurrentPage, (LPMODULEINFO) &ModInfo, sizeof(MODULEINFO));
			CurrentPage += ModInfo.SizeOfImage;

		}
		else
		{
			mr->ini = (unsigned long)CurrentPage;
			mr->end = mr->ini + mbi.RegionSize;
			mr->size = mbi.RegionSize;
			mr->perms = mbi.Protect;
			CurrentPage +=  mbi.RegionSize; 
		}


	}

}

bool CheckValidPE(unsigned char * PeHeader)
{    	    
	IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)PeHeader;
	IMAGE_NT_HEADERS *nt_headers;

	if (dos_header->e_magic==IMAGE_DOS_SIGNATURE)
	{
		nt_headers = (IMAGE_NT_HEADERS *)((char *)dos_header
				+ dos_header->e_lfanew);
		if (nt_headers->Signature==IMAGE_NT_SIGNATURE)
		{
			return TRUE;
		}	        			
	}
	return FALSE;
}
