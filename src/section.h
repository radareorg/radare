#ifndef _INCLUDE_SECTION_H_
#define _INCLUDE_SECTION_H_

struct section_t{
	char comment[256];
	ut64 from;
	ut64 to;
	ut64 vaddr;
	ut64 paddr; // offset on disk
	int rwx;
	struct list_head list;
};

enum {
	SECTION_R = 4,
	SECTION_W = 2,
	SECTION_X = 1,
};

int section_rm(int idx);
void section_add(ut64 from, ut64 to, ut64 vaddr, ut64 physical, int rwx, const char *comment);
void section_set(ut64 from, ut64 to, ut64 vaddr, ut64 physical, int rwx, const char *comment);
void section_list(ut64 addr, int rad);
struct section_t *section_get(ut64 addr);
void section_list_visual(ut64 seek, ut64 len);
ut64 section_get_vaddr(ut64 addr);
struct section_t *section_get_i(int idx);
void section_init(int foo);
int section_overlaps(struct section_t *s);

#endif
