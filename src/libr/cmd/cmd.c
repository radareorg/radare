/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_cmd.h"

/* binary/byte tree storage for commands */
#if 0
  r_cmd_add(cmd, "s/", &seek_search);

#endif

int r_cmd_add(struct r_cmd_t *cmd, const char *prefix, r_cmd_callback callback)
{
	struct r_cmd_item_t *item;
	strncpy(item->cmd, input, 63);
	list_add(item, data->list);
	return 0;
}

int r_cmd_call(struct r_cmd_t *cmd, const char *cmd)
{
}

int r_cmd_init(struct r_cmd_t *cmd)
{
	INIT_LIST_HEAD(&cmd->cmds);
	return 0;
}

#if 0
struct r_cmd_t cmds;

r_cmd_init(&cmds);

r_cmd_add(&cmds, 
#endif
