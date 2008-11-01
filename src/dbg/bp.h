// this is used by the arch/foo-bp files
// to store a list of all the valid software
// breakpoints for each architecture
#ifndef _INCLUDE_BP_H_
#define _INCLUDE_BP_H_

struct arch_bp_t {
	u8 *bytes;
	int length;
};

struct arch_bp_t **arch_bps;

int arch_bp_rm_soft(struct bp_t *bp);
int arch_bp_rm_hw(struct bp_t *bp);

#endif
