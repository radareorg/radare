#ifndef _INCLUDE_R_SEARCH_H_
#define _INCLUDE_R_SEARCH_H_

#include "r_types.h"

struct r_search_kw_t {
	char keyword[128];
	char binmask[128];
	int keyword_length;
	int binmask_length;
	struct list_head list;
};

struct r_search_hit_t {
	u64 addr;
	r_search_kw_t *kw;
	int len;
	struct list_head list;
};

struct r_search_t {
	int fd;
	int (*callback)(); // XXX
	struct r_search_hw_t kws;
	struct r_search_hit_t hits;
};

struct r_search_t *r_search_new(int fd);
struct r_search_t *r_search_free(struct r_search_t *r);

/* keyword management */
int r_search_kw(struct r_search_t *s);
int r_search_kw_add(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_hex(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_bin(struct r_search_t *s, const char *kw, int kw_len, const char *bm, int bm_len);
struct r_search_kw_t *r_search_kw_list(struct r_search_t *s);

/* configuration */
int r_search_set_range(struct r_search_t *s, u64 from, u64 to);
int r_search_set_limit(struct r_search_t *s, int counter);
int r_search_set_blocksize(struct r_search_t *s, int bsize);

/* pattern search */
int r_search_pattern(struct r_search_t *s, int min, int max);

#endif
