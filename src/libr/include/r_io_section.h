#ifndef _INCLUDE_R_IO_SECTION_H_
#define _INCLUDE_R_IO_SECTION_H_

#include "list.h"

struct r_io_section_t {
	char comment[256];
	u64 from;
	u64 to;
	u64 vaddr;
	u64 paddr; // offset on disk
	struct list_head list;
};

int r_io_section_rm(int idx);
void r_io_section_add(u64 from, u64 to, u64 vaddr, u64 physical, const char *comment);
void r_io_section_set(u64 from, u64 to, u64 vaddr, u64 physical, const char *comment);
void r_io_section_list(u64 addr, int rad);
struct r_io_section_t * r_io_section_get(u64 addr);
void r_io_section_list_visual(u64 seek, u64 len);
u64 r_io_section_get_vaddr(u64 addr);
struct r_io_section_t * r_io_section_get_i(int idx);
void r_io_section_init(int foo);
int r_io_section_overlaps(struct r_io_section_t *s);

#endif
