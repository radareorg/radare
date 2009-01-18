/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include <r_cmd.h>

/* binary/byte tree storage for commands */
#if 0
  r_cmd_add(cmd, "s/", &seek_search);

#endif

int r_cmd_set_data(struct r_cmd_t *cmd, void *data)
{
	cmd->data = data;
	return 0;
}

int r_cmd_add(struct r_cmd_t *cmd, const char *command, r_cmd_callback(callback))
{
	struct r_cmd_item_t *item;
	int idx = (u8)command[0];

	item = cmd->cmds[idx];
	if (item == NULL) {
		item = MALLOC_STRUCT(struct r_cmd_item_t);
		cmd->cmds[idx] = item;
	}
	strncpy(item->cmd, command, 63);
	item->callback = callback;
	return 0;
}

int r_cmd_del(struct r_cmd_t *cmd, const char *command)
{
	int idx = (u8)command[0];
	free(cmd->cmds[idx]);
	cmd->cmds[idx] = NULL;
	return 0;
}

int r_cmd_call(struct r_cmd_t *cmd, const char *input)
{
	struct r_cmd_item_t *c;
	int ret = -1;
	c = cmd->cmds[(u8)input[0]];
	if (c != NULL && c->callback!=NULL)
		ret = c->callback(cmd->data, input+1);
	return ret;
}

int r_cmd_init(struct r_cmd_t *cmd)
{
	//INIT_LIST_HEAD(&cmd->cmds);
	memset(&cmd->cmds, '\0', sizeof(struct list_head)*255);
	cmd->data = NULL;
	return 0;
}

#if 0
struct r_cmd_t cmds;

r_cmd_init(&cmds);

r_cmd_add(&cmds, 
#endif
