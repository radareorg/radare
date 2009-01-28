#ifndef _INCLUDE_DIFF_H_
#define _INCLUDE_DIFF_H_

#include <r_types.h>
#include <r_util.h>

struct r_diff_t {
	u64 off_a;
	u64 off_b;
	int delta;
	void *user;
	int (*callback)(struct r_diff_t *d, void *user,
		u64 from, const u8 *oldbuf, int oldlen,
		u64 to, const u8 *newbuf, int newlen);
};

struct r_diff_t *r_diff_new(u64 off_a, u64 off_b);
int r_diff_init(struct r_diff_t *d, u64 off_a, u64 off_b);
struct r_diff_t *r_diff_free(struct r_diff_t *d);
int r_diff_buffers(struct r_diff_t *d, const u8 *a, u32 la, const u8 *b, u32 lb);
int r_diff_set_callback(struct r_diff_t *d,
	int (*callback)(struct r_diff_t *d, void *user,
		u64 from, const u8 *oldbuf, int oldlen,
		u64 to, const u8 *newbuf, int newlen),
	void *user);

#endif
