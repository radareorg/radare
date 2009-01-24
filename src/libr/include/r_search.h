#ifndef _INCLUDE_R_SEARCH_H_
#define _INCLUDE_R_SEARCH_H_

#include "r_types.h"
#include "r_util.h"
#include "list.h"

/* binparse api */
// TODO: Remove typedef!!
typedef struct r_search_binparse_token {
	u8 mintok; // token
	u8 range;  // 0 only mintok, ( maxtoken - mintoken )
	u8 mask;   // binmask
} token;

typedef struct r_search_binparse_tokenlist_t {
	token* tl;
	int numtok;
	char name [300];
	char actp[300]; //aux pel parseig actual
	int stat;
	/* int lastpos; XXX unused */
} tokenlist;

struct r_search_binparse_t {
	//tokenlist** tls;
	struct r_search_binparse_tokenlist_t **tls;
	int nlists;
	int interrupted;
	int (*callback)(struct r_search_binparse_tokenlist_t *t, int i, u64 where);
};

struct r_search_binparse_t *binparse_new(int kws);
int r_search_binparse_free(struct r_search_binparse_t *ptokenizer);
int r_search_binparse_add(struct r_search_binparse_t *t, const char *string, const char *mask);
int r_search_binparse_add_named(struct r_search_binparse_t *t, const char *name, const char *string, const char *mask);
int r_search_binparse_update(struct r_search_binparse_t *t, u8 inchar, u64 where);

/* search api */

struct r_search_kw_t {
	char keyword[128];
	char binmask[128];
	u8 bin_keyword[128];
	u8 bin_binmask[128];
	int keyword_length;
	int binmask_length;
	struct list_head list;
};

struct r_search_hit_t {
	u64 addr;
	struct r_search_kw_t *kw;
	int len;
	struct list_head list;
};

struct r_search_range_t {
	u64 from;
	u64 to;
	struct list_head list;
};

struct r_search_t {
	int n_kws;
	int (*callback)(); // XXX wtf?
	struct r_search_binparse_t *bp;
	struct list_head kws; //r_search_hw_t kws;
	struct list_head hits; //r_search_hit_t hits;
};

struct r_search_t *r_search_new();
struct r_search_t *r_search_free(struct r_search_t *s);

/* keyword management */
int r_search_start(struct r_search_t *s);
int r_search_update(struct r_search_t *s, u64 from, const u8 *buf, int len);

/* */
int r_search_kw_add(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_hex(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_bin(struct r_search_t *s, const u8 *kw, int kw_len, const u8 *bm, int bm_len);
struct r_search_kw_t *r_search_kw_list(struct r_search_t *s);
int r_search_reset(struct r_search_t *s);

int r_search_range_add(struct r_search_t *s, u64 from, u64 to);
int r_search_range_set(struct r_search_t *s, u64 from, u64 to);
int r_search_range_reset(struct r_search_t *s);
int r_search_set_blocksize(struct r_search_t *s, u32 bsize);

/* pattern search */
int r_search_pattern(struct r_search_t *s, u32 size);
int r_search_strings(struct r_search_t *s, u32 min, u32 max);

#endif
