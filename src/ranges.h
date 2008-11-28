#ifndef _INCLUDE_RANGES_H_
#define _INCLUDE_RANGES_H_

struct range_t {
  u64 from;
  u64 to;
  struct list_head list;
};

int ranges_init();
void ranges_free();
int ranges_add(struct list_head *rang, u64 from, u64 to);
int ranges_sub(struct list_head *rang, u64 from, u64 to);
int ranges_list();
int ranges_boolean(u64 from, u64 to, int flags);
int ranges_cmd(const char *arg);

#endif
