/*
 * lsbstego.c, v1.0
 * by Roman Medina-Heigl Hernandez a.k.a. RoMaNSoFt <roman@rs-labs.com>
 * [16.12.2007]
 *
 * Lsbstego is a simple tool useful for a fast and "visual" stegoanalysis phase.
 * It decrypts files by applying the commonly known LSB technique. The program
 * permits to run the process several times varying certain parameters so different
 * cases are covered. If you have an unknown file and you suspect it could have a
 * secret message embeded, run in this way:
 *
 * $ ./lsbstego -a <file>
 *
 * Output is written to stdout so you could easily redirect to a file or pipe to
 * another util like "strings" (useful when output contains non-printable chars).
 *
 * Other options can be used to launch a customized one pass only run. These are:
 *  -f (forward):   process file from the beginning to the eof
 *  -r (reverse):   process file from eof to the beginning
 *  -d (downward):  lsb's are joined downward (i.e dbyte = lsb1 ... lsb8)
 *  -u (upward):    lsb's are joined upward (i.e. dbyte = lsb8 ... lsb1)
 *  -o <offset>:    skip <offset> bytes before starting normal processing
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef enum { false, true } bool;

/* Prototypes */
void help (void);
void process (FILE *fd, int length, bool forward, bool downward, int offset);


/* Main program */
int main(int argc, char **argv)
{
	bool forward = false, reverse = false, downward = false, upward = false, all = false;
	int offset = 0;
	FILE *fd;
	int length;
	int c, i, j;

	if (argc == 1)
	{
		help();
		return EXIT_SUCCESS;
	}

	opterr = 0;

	while ((c = getopt (argc, argv, "frduao:")) != -1)
		switch (c)
		{
			case 'f':
				forward = true;
				break;
			case 'r':
				reverse = true;
				break;
			case 'd':
				downward = true;
				break;
			case 'u':
				upward = true;
				break;
			case 'a':
				all = true;
				break;
			case 'o':
				offset = atoi(optarg);
				if (offset < 0 || offset >7)
				{
					fprintf(stderr, "Offset must be in 0-7 range\n");
					return EXIT_FAILURE;
				}
				break;
			case '?':
				if (optopt == 'o')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return EXIT_FAILURE;
			default:
				abort();
		}

	/* Check for some errors */
	if (forward && reverse)
	{
		fprintf(stderr, "Cannot use -f and -r options together\n");
		return EXIT_FAILURE;
	}

	if (downward && upward)
	{
		fprintf(stderr, "Cannot use -d and -u options together\n");
		return EXIT_FAILURE;
	}

	if (optind > argc - 1)
	{
		fprintf(stderr, "You must specify a source file\n");
		return EXIT_FAILURE;
	} else if (optind < argc - 1)
	{
		fprintf(stderr, "You must specify one and only one source file\n");
		return EXIT_FAILURE;
	}

	/* Set default options */
	if (!forward && !reverse)
		forward = true;

	if (!downward && !upward)
		downward = true;

	/* Open file */
	if( (fd = fopen( argv[optind], "rb" )) == NULL )
	{
		fprintf(stderr,"Could not open source file! Does it exist and have read permission?\n");
		return EXIT_FAILURE;
	}

	/* Calculate length */
	fseek (fd, 0, SEEK_END);
	length = ftell(fd);
	fseek (fd, 0, SEEK_SET);

	/* File processing */
	if (all)
	{
		for (i = 0 ; i <= 1 ; i++)
		{
			for (j = 0 ; j <= 1 ; j++)
			{
				for (offset = 0 ; offset <= 7 ; offset++)
				{
					/* Brute-force run */
					process (fd, length, i, j, offset);
				}
			}
		}
	} else
		/* Customized one pass only run */
		process (fd, length, forward, downward, offset);
	
	fclose(fd);
	return EXIT_SUCCESS;
}


/* Print help */
void help (void)
{
	fprintf(stderr, "\n- lsbstego v1.0. (c) RoMaNSoFt, 2007 -\n");
	fprintf(stderr, "A simple tool useful for a fast and \"visual\" LSB-stegoanalysis phase.\n\n");
	fprintf(stderr, "Syntax: ./lsbstego {-f | -r} {-d | -u} {-o <offset>} <file>\n");
	fprintf(stderr, "        ./lsbstego -a <file>\n\n");
	fprintf(stderr, "Options\n");
	fprintf(stderr, "        -f (forward):   process file from the beginning to eof\n");
	fprintf(stderr, "        -r (reverse):   process file from eof to the beginning\n");
	fprintf(stderr, "        -d (downward):  lsb's are joined downward (i.e dbyte = lsb1 ... lsb8)\n");
	fprintf(stderr, "        -u (upward):    lsb's are joined upward (i.e. dbyte = lsb8 ... lsb1)\n");
	fprintf(stderr, "        -o <offset>:    skip <offset> bytes before starting normal processing\n");
	fprintf(stderr, "        -a:             run all possible combinations / cases\n\n");
}


/* Process source file with given parameters. Output to stdout */
void process (FILE *fd, int length, bool forward, bool downward, int offset)
{
	int byte;	/* Byte index */
	int bit;	/* Bit index */
	int lsb;	/* Least Significant Bit */
	char sbyte;	/* Source byte (i.e. from src file */
	char dbyte;	/* Destination byte (decrypted msg) */


	for ( byte = offset ; byte < length ; )
	{
		dbyte = 0;

		for (bit = 0; bit <= 7; bit++, byte++)
		{
			/* Set position at the beginning or eof */
			if (forward)
				fseek(fd, byte, SEEK_SET);
			else
				fseek(fd, -(byte+1), SEEK_END);

			/* Read one byte */
			fread(&sbyte, sizeof(sbyte), 1, fd);

			/* Obtain Least Significant Bit */
			lsb = sbyte & 1;

			/* Add lsb to decrypted message */
			if (downward)
				dbyte = dbyte | lsb << (7-bit) ;
			else
				dbyte = dbyte | lsb << bit ;
		}

		printf ("%c", dbyte);
	}
}
