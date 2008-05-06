/*
 * Copyright (C) 2008
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

void bindiff_help()
{
	printf("Usage: bindiff [-c] [-bge] [-p] [file-a] [file-b]\n");
	printf("  -c   colorize output\n");
	printf("  -b   bytediff (faster but doesnt support displacements)\n");
	printf("  -g   use gnu diff as backend (default)\n");
	printf("  -e   use erg0ts bdiff (c++) as backend\n");
	printf("  -p   binpatching (TODO)\n");
}

int main(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "hcbgep")) != -1)
	{
		switch( c ) {
		default:
			printf("Invalid option\n");
		case 'h':
			bindiff_help();
			return 0;
		}
	}
}
