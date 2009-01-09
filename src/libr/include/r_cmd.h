#include "r_types.h"
#include "list.h"

struct r_cmd_item_t {
	char cmd[64];
	struct list_head list;
};

struct r_cmd_t {
	struct list_head cmds;
};

typedef int (*r_cmd_callback)(void *data, char *input);
