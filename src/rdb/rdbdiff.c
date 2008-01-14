#include "rdb.h"

// WARNING!!! The RDB diffing engine lives here

int rdb_diff_block(struct block_t *a, struct block_t *b)
{
	int i,j;
	printf("Function '%s' differs (address = 0x%08x)\n",
		b->name, b->addr);
	if (a->framesize != b->framesize)
		printf("   stackframe differs: %d vs %d\n", a->framesize, b->framesize);
	if (a->n_bytes!= b->n_bytes) {
		printf("   n_bytes differs: %d vs %d\n", a->n_bytes, b->n_bytes);
	} else {
		// byte diff
		for(i=0;i<a->n_bytes;i++) {
			if (a->bytes[i] != b->bytes[i]) {
				printf("  %d %02x vs %02x\t", i, a->bytes[i], b->bytes[i]);
			}
		}
		printf("\n");
	}
	return 0;
}

int rdb_diff(struct program_t *a, struct program_t *b, int mode)
{
	struct list_head *i, *j;
	struct block_t *b0, *b1;
	struct xref_t *x0, *x1;
	int ignored = 0;
	int count = 0;

	/* ignore dupped code blocks by checksum */
	// TODO: analyze xrefs too!
	list_for_each_prev(i, &(a->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		if (! b0->ignored)
		list_for_each_prev(j, &(b->blocks)) {
			b1 = list_entry(j, struct block_t, list);
			if (! b1->ignored )
			if (b0->checksum == b1->checksum) {
				b0->ignored = \
				b1->ignored = 1;
				ignored++;
				printf("ignored by checksum: %s\n", b0->name);
				// TODO: check xrefs
			}
		}
	}

	printf("Ignored blocks by checksum: %d / (%d, %d)\n", ignored, a->n_blocks, b->n_blocks);

	/* detect new code blocks */
	if (a->n_blocks != b->n_blocks) {
		printf("There are different number of code blocks!!! %d vs %d\n", a->n_blocks, b->n_blocks);
	}

	count = 0;
	list_for_each_prev(i, &(a->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		if (!b0->ignored)
		list_for_each_prev(j, &(b->blocks)) {
			b1 = list_entry(j, struct block_t, list);
			if (! b1->ignored ) {
				/* check if they have the same name */
				if (b0->name && b1->name) {
					if (!strcmp(b0->name, b1->name)) {
						/* hey hey! we got two different blocks with the same name! */
						rdb_diff_block(b0, b1);
						b0->ignored = \
						b1->ignored = 1;
						count++;
					}
				}
			}
		}
	}
	if (count) {
		printf("Found %d/(%d,%d) blocks with same name and different checksum\n", count, a->n_blocks, b->n_blocks);
	}
	printf("Missing %d blocks to analyze\n", a->n_blocks-(count+ignored));
	list_for_each_prev(i, &(a->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		if (!b0->ignored) {
			printf("%s block 0x%08x not processed\n", a->name, b0->addr);
//			list_for_each_prev(i, &(b0->xrefs)) {
//				x0 = list_entry(i, struct xref_t, list);
				//printf("  xref 0x%08x\n", x0->addr);
//			}
		}
	}
	list_for_each_prev(i, &(b->blocks)) {
		b0 = list_entry(i, struct block_t, list);
		if (!b0->ignored) {
			printf("%s block 0x%08x not processed\n", a->name, b0->addr);
//			list_for_each_prev(i, &(b0->xrefs)) {
//				x0 = list_entry(i, struct xref_t, list);
				//printf("  xref 0x%08x\n", x0->addr);
//			}
		}
	}

	/* diff similar code blocks */
	// similarity means:
	//  - check framesize differences
	//  - same name
	//  - same xrefs
	//  - bytediff < N% of the code
	// TODO
}
