#ifndef MEM_H
#define MEM_H

#include <sys/param.h>
//#include <sys/user.h>
#include "../list.h"

/* on BSD this does not exist */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define MAX_MAP_SIZE (PAGE_SIZE * 8)

typedef struct {

	char *tag;
	char *addr;
	unsigned long size;

	struct list_head list;

} MAP_MEM;

typedef struct {

	unsigned long ini;
	unsigned long end;
	unsigned long perms, perms_orig;
	int flags;
	char *bin;
	unsigned long size;

	struct list_head list;

} MAP_REG;

void *dealloc_page(void *addr);
void *mmap_tagged_page(char *file, off_t addr, off_t size);
void *alloc_tagged_page(char *tag, int size);
inline void *alloc_page(int size);
inline void add_regmap(MAP_REG *mr);
void *dealloc_all();
void print_status_alloc();
void free_regmaps();
void print_maps_regions();
void up_regions(int usrcode);
void rest_region(MAP_REG *mr);
void page_dumper(const char *dir);
void page_restore(const char *dir);

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
