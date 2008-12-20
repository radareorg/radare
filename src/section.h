#ifndef _INCLUDE_SECTION_H_
#define _INCLUDE_SECTION_H_

struct section_t{
	char comment[256];
	u64 from;
	u64 to;
	u64 base;
	u64 ondisk; // offset on disk
	struct list_head list;
} ;

int section_rm(int idx);
void section_add(u64 from, u64 to, u64 base, u64 ondisk, const char *comment);
void section_set(u64 from, u64 to, u64 base, u64 ondisk, const char *comment);
void section_list(u64 addr, int rad);
struct section_t *section_get(u64 addr);
void section_list_visual(u64 seek, u64 len);
u64 section_get_base(u64 addr);
struct section_t *section_get_i(int idx);
void section_init(int foo);
int section_overlaps(struct section_t *s);

#endif
