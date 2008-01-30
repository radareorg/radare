/* example plugin */

#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;

void my_hack(char *input)
{
	printf("Hello hack! %s\n", radare_plugin.config->file);
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "hello",
	.desc = "Hello hack example",
	.callback = &my_hack
};

main() { printf("i'm a plugin!\n"); }
