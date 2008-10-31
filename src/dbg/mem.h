#ifndef MEM_H
#define MEM_H

#include <sys/param.h>
//#include <sys/user.h>
#include "../list.h"
#include "arch/arch.h"

/* on BSD this does not exist */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define MAX_MAP_SIZE (PAGE_SIZE * 8)

typedef struct {
	u8 *tag;
	addr_t addr;
	u64 size;
	struct list_head list;
} MAP_MEM;

typedef struct {
	u64 ini;
	u64 end;
	u32 perms, perms_orig;
	u8 *bin;
	u64 size;
	int flags;

	struct list_head list;
} MAP_REG;

addr_t mmap_tagged_page(const char *file, addr_t addr, addr_t size);
addr_t alloc_tagged_page(const char *tag, unsigned long size);
addr_t arch_dealloc_page(addr_t addr, unsigned long size);
inline addr_t alloc_page(int size);
inline void add_regmap(MAP_REG *mr);
void dealloc_all();
void print_status_alloc();
void free_regmaps();
void up_regions(int usrcode);
void rest_region(MAP_REG *mr);
void page_dumper(const char *dir);
void page_restore(const char *dir);
addr_t dealloc_page(addr_t addr);
addr_t arch_alloc_page(unsigned long size, unsigned long *rsize);
inline addr_t alloc_page(int size);
inline void add_regmap(MAP_REG *mr);
void print_status_alloc();
void free_regmaps(int rest);
void print_maps_regions(int rad, int two);
void page_restore(const char *dir);
void page_dumper(const char *dir);
void up_regions(int usrcode);
void rest_region(MAP_REG *mr);
void rest_all_regions();
int arch_mprotect(addr_t addr, unsigned int size, int perms);


#define REGION_NONE	0
#define REGION_EXEC	1
#define REGION_READ	1 << 1
#define REGION_WRITE	1 << 2

#define FLAG_USERCODE	1
#define FLAG_SYSCODE    1 << 1
#define FLAG_NOPERM	1 << 2

#define SYSCODE		1
#define USERCODE	1 << 1

#endif
