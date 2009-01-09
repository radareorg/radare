#ifndef _INCLUDE_CONFIG_H_
#define _INCLUDE_CONFIG_H_

#include "r_config.h"

#define CN_BOOL  0x000001
#define CN_INT   0x000002
#define CN_OFFT  0x000004
#define CN_STR   0x000008
#define CN_RO    0x000010
#define CN_RW    0x000020

struct r_config_node_t {
	char *name;
	int hash; /* hash of the name - optimized search */
	int flags;
	char *value;
	u64 i_value;
	u64 *cb_ptr_q;
	int *cb_ptr_i;
	char **cb_ptr_s;
	int (*callback)(void *data);
	struct list_head list;
};

struct r_config_new_t {
	int lock;
	int n_nodes;
	struct list_head nodes;
};

#define O struct r_config_new_t *obj

void r_config_init(O);
void r_config_lock(O, int l);
void r_config_eval(O, char *str);
struct r_config_node_t *r_config_set_i(O, const char *name, const u64 i);
int r_config_rm(O, const char *name);
struct r_config_node_t *r_config_set(O, const char *name, const char *value);
u64 r_config_get_i(O, const char *name);
const char *r_config_get(O, const char *name);
void r_config_list(O, const char *str);
struct r_config_node_t *r_config_node_get(O, const char *name);
struct r_config_node_t *r_config_node_new(O, const char *name, const char *value);

#endif
