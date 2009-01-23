#ifndef _INCLUDE_R_SEARCH_H_
#define _INCLUDE_R_SEARCH_H_

#include "r_types.h"
#include "r_io.h"

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
	int estat;
	/* int lastpos; XXX unused */
} tokenlist;

struct r_search_binparse_t {
	tokenlist** tls;
	struct r_search_binparse_tokenlist_t **tls;
	int nlists;
	int interrupted;
	int (*callback)(struct _tokenizer *t, int i, u64 where);
};

struct r_search_binparse_t *binparse_new(int kws);
int r_search_binparse_free(struct r_search_binparse_t *ptokenizer);
int r_search_binparse_add(struct r_search_binparse_t *t, const char *string, const char *mask);
int r_search_binparse_add(struct r_search_binparse_t *t, const char *name, const char *string, const char *mask);
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
	r_search_kw_t *kw;
	int len;
	struct list_head list;
};

struct r_search_t {
#if 0 // TORETHINK
	int fd;
	u64 seek;
	int use_from_to;
	u64 from;
	u64 to;
#endif
	int (*callback)(); // XXX wtf?
	struct r_search_binparse_t *bp;
	struct list_head kws; //r_search_hw_t kws;
	struct list_head hits; //r_search_hit_t hits;
};

struct r_search_t *r_search_new(int fd);
struct r_search_t *r_search_free(struct r_search_t *r);

/* keyword management */
int r_search_start(struct r_search_t *s);
int r_search_pause(struct r_search_t *s);
int r_search_stop(struct r_search_t *s);
int r_search_reset(struct r_search_t *s);
/* */
int r_search_kw_add(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_hex(struct r_search_t *s, const char *kw, const char *bm);
int r_search_kw_add_bin(struct r_search_t *s, const char *kw, int kw_len, const char *bm, int bm_len);
struct r_search_kw_t *r_search_kw_list(struct r_search_t *s);
int r_search_reset(struct r_search_t *s);

/* configuration */
int r_search_set_range(struct r_search_t *s, u64 from, u64 to);
int r_search_set_limit(struct r_search_t *s, int counter);
int r_search_set_blocksize(struct r_search_t *s, int bsize);

/* pattern search */
int r_search_pattern(struct r_search_t *s, int min, int max);


#endif
