/* example plugin */

#include "plugin.h"

extern int radare_plugin_type;
extern struct plugin_hack_t radare_plugin;

static int my_hack(const char *input)
{
	u8 *buf;
	int (*r_cmdf)(const char *cmd, ...);
	int (*r_block)();


	/* radare_cmd call example */
	r_cmdf  = radare_plugin.resolve("radare_cmdf");
	r_block = radare_plugin.resolve("radare_block");

	if (r_cmdf == NULL || r_block == NULL) {
		fprintf(stderr, "Cannot resolve core symbols!\n");
	}
	buf = r_block();
	fprintf(stderr, "warning: x86 only atm.\n");
	
	switch(buf[0]) {
	case 0x75:
		buf[0]='\xeb';
		break;
	case 0x0f:
		if (buf[1]>=0x80 && buf[1]< 0x8f) {
			buf[0] = 0x90; // nop
			buf[1] = 0xe9; // jmp
		}
		break;
	}
	r_cmdf("wx %02x %02x", buf[0], buf[1]);

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "fj",
	.desc = "Force jump",
	.callback = &my_hack
};

