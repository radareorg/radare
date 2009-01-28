#include <r_diff.h>

static int count = 0;

static int cb(struct r_diff_t *d, void *user,
	u64 from, const u8 *oldbuf, int oldlen,
	u64 to, const u8 *newbuf, int newlen)
{
	int i;

	if (count) {
		count++;
		return 1;
	}
	printf(" 0x%08llx  ", from);
	for(i = 0;i<oldlen;i++)
		printf("%02x", oldbuf[i]);
	printf(" => ");
	for(i = 0;i<newlen;i++)
		printf("%02x", newbuf[i]);
	printf("  0x%08llx\n", to);
	return 1;
}

static int show_help(int line)
{
	printf("Usage: radiff2 [-nd] [file] [file]\n");
	if (!line) printf(
		"  -c   :  count of changes\n"
		"  -d   :  use delta diffing\n");
	return 1;
}

int main(int argc, char **argv)
{
	struct r_diff_t d;
	int c, delta = 0;
	char *file, *file2;
	u8 *bufa, *bufb;
	u32 sza, szb;

	if (argc<3)
		return show_help(0);

	while ((c = getopt(argc, argv, "cd")) != -1) {
		switch(c) {
		case 'c':
			count = 1;
			break;
		case 'd':
			delta = 1;
			break;
		default:
			return show_help(1);
		}
	}
	
	if (optind+2<argc)
		return show_help(0);

	file = argv[optind];
	file2 = argv[optind+1];

	bufa = r_file_slurp(file, &sza);
	bufb = r_file_slurp(file2, &szb);
	if (bufa == NULL || bufb == NULL) {
		fprintf(stderr, "Error slurping source files\n");
		return 1;
	}

	r_diff_init(&d, 0LL, 0LL);
	r_diff_set_delta(&d, delta);
	r_diff_set_callback(&d, &cb, NULL);
	r_diff_buffers(&d, bufa, sza, bufb, szb);

	if (count)
		printf("%d\n", count-1);

	return 0;
}
