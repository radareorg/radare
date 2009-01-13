/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

int rasign_show_help()
{
	printf("Usage: rasign [options] [file]\n"
	" -r  : show output in radare commands\n"
	"Examples:\n"
	"  rasign libc.so.6 > libc.sig\n"
	"  rasign -s libc.sig ls.static\n");
	return 0;
}

int main(int argc, char **argv)
{
	int c;
	int action = 0;
	int rad = 0;

	while((c=getopt(argc, argv, "hrs:i")) !=-1) {
		switch(c) {
		case 's':
			action = c;
			rasign_load_sig_file(optarg);
			break;
		case 'r':
			rad = 1;
			break;
		default:
			return rasign_show_help();
		}
	}
	switch(action) {
	case 's':
		/* check sigfiles in optarg file */
		break;
	default:
		/* generate signature file */
	}
	return 0;
}
