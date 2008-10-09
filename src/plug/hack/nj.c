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
	case 0x0f:
		switch(buf[1]) {
		case 0x84: // jz
			buf[1] = 0x85;
			break;
		case 0x85: // jnz
			buf[1] = 0x84;
			break;
		case 0x8e: // jle
		case 0x86: // jbe
			buf[1] = 0x8f; // jg
			break;
		case 0x87: // ja
		case 0x8f: // jg
			buf[1] = 0x86; // jbe
			break;
		case 0x88: // js
			buf[1] = 0x89; // jns
			break;
		case 0x89: // jns
			buf[1] = 0x88; // js
			break;
		case 0x8a: // jp
			buf[1] = 0x8b; // jnp
			break;
		case 0x8b: // jnp
			buf[1] = 0x8a; // jp
			break;
		case 0x8c: // jl
			buf[1] = 0x8d;
			break;
		case 0x8d: // jge
			buf[1] = 0x8c;
			break;
		}
		break;
	case 0x70: buf[0] = 0x71; break; // jo->jno
	case 0x71: buf[0] = 0x70; break; // jno->jo
	case 0x72: buf[0] = 0x73; break; // jb->jae
	case 0x73: buf[0] = 0x72; break; // jae->jb
	case 0x75: buf[0] = 0x74; break; // jne->je
	case 0x74: buf[0] = 0x75; break; // je->jne
	case 0x76: buf[0] = 0x77; break; // jbe->ja
	case 0x77: buf[0] = 0x76; break; // jbe->ja
	case 0x78: buf[0] = 0x79; break; // js->jns
	case 0x79: buf[0] = 0x78; break; // jns->js
	case 0x7a: buf[0] = 0x7b; break; // jp->jnp
	case 0x7b: buf[0] = 0x7a; break; // jnp->jp
	case 0x7c: buf[0] = 0x7d; break; // jl->jge
	case 0x7d: buf[0] = 0x7c; break; // jge->jl
	case 0x7e: buf[0] = 0x7f; break; // jg->jle
	case 0x7f: buf[0] = 0x7e; break; // jle->jg
		break;
	}
	
	r_cmdf("wx %02x", buf[0]);

	return 0;
}

int radare_plugin_type = PLUGIN_TYPE_HACK;
struct plugin_hack_t radare_plugin = {
	.name = "nj",
	.desc = "Negate jump",
	.callback = &my_hack
};

