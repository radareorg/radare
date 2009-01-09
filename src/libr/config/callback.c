#include "r_config.h"

static int r_config_callback_q(void *data)
{
	struct r_config_node_t *node = data;
	*(node->cb_ptr_q) = node->i_value;
	return 1;
}

static int r_config_callback_i(void *data)
{
	struct r_config_node_t *node = data;
	*(node->cb_ptr_i) = node->i_value;
	return 1;
}

static int r_config_callback_s(void *data)
{
	struct r_config_node_t *node = data;
	if (!node->value || node->value[0]=='\0') {
		free(*node->cb_ptr_s);
		*node->cb_ptr_s = NULL;
	} else *node->cb_ptr_s = estrdup(node->cb_ptr_s, node->value);
	return 1;
}

int r_config_set_callback_q(struct r_config_t *cfg, const char *name, u64 *ptr)
{
	struct r_config_node_t *node;
	node = r_config_node_get(cfg, name);
	if (node) {
		node->cb_ptr_q = ptr;
		node->callback = &r_config_callback_q;
	}
}

int r_config_set_callback_i(struct r_config_t *cfg, const char *name, u64 *ptr)
{
	struct r_config_node_t *node;
	node = r_config_node_get(cfg, name);
	if (node) {
		node->cb_ptr_i = ptr;
		node->callback = &r_config_callback_i;
	}
}

int r_config_set_callback_s(struct r_config_t *cfg, const char *name, u64 *ptr)
{
	struct r_config_node_t *node;
	node = r_config_node_get(cfg, name);
	if (node) {
		node->cb_ptr_s = ptr;
		node->callback = &r_config_callback_s;
	}
}
