#ifndef _INCLUDE_RANGES_H_
#define _INCLUDE_RANGES_H_

struct range_t {
  u64 from;
  u64 to;
  u8 *data;
  int datalen;
  struct list_head list;
};

extern void (*ranges_new_callback)(struct range_t *r);

int ranges_init();
void ranges_free();
u64 ranges_size(struct list_head *rang);
struct range_t *ranges_get(struct list_head *rang, u64 addr);
struct range_t *ranges_add(struct list_head *rang, u64 from, u64 to, int rw);
int ranges_sub(struct list_head *rang, u64 from, u64 to);
int ranges_list();
int ranges_boolean(u64 from, u64 to, int flags);
int ranges_cmd(const char *arg);
int ranges_get_n(int n, u64 *from, u64 *to);

#define RANGE_TRACES 1
#define RANGE_GRAPHS 2
#define RANGE_FUNCTIONS 4

#endif
