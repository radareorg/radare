/* example plugin */

#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;

int my_hack(char *input)
{
	int (*r)(char *cmd, int log);

	printf("Hello hack! %s\n", radare_plugin.config->file);

	/* radare_cmd call example */
	r = radare_plugin.resolve("radare_cmd");
	if (r != NULL) {
		r("b 20", 0);
		r("x", 0);
	} else	printf("Cannot resolve 'radare_cmd' symbol\n");
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "hello",
	.desc = "Hello hack example",
	.callback = &my_hack
};
