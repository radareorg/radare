// this is used by the arch/foo-bp files
// to store a list of all the valid software
// breakpoints for each architecture
struct arch_bp_t {
	u8 *bytes;
	int length;
};

struct arch_bp_t **arch_bps;

void debug_bp_list();
