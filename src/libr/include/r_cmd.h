#include <r_types.h>
#include "list.h"

#define r_cmd_callback(x) int (*x)(void *data, const char *input)

struct r_cmd_item_t {
	char cmd[64];
	r_cmd_callback(callback);
};

struct r_cmd_t {
	void *data;
	struct r_cmd_item_t *cmds[255];
};

int r_cmd_set_data(struct r_cmd_t *cmd, void *data);
int r_cmd_add(struct r_cmd_t *cmd, const char *command, r_cmd_callback(callback));
int r_cmd_del(struct r_cmd_t *cmd, const char *command);
