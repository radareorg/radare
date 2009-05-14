/*
 * Copyright (C) 2008 Daniel Fernandez <soyfeliz@48bits.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <exception>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "bdiff.h"

class FMap
{
	int fd ;
public:
	char *buff ;
	size_t size ;

	FMap(const char *file)
	{
		struct stat buf ;

		if (lstat(file, &buf) == -1)
			throw std::runtime_error("lstat") ;

		size = buf.st_size ;

		if ((fd = open(file, O_RDONLY)) == -1)
			throw std::runtime_error("open") ;

		buff = (char *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0) ;

		if (buff == MAP_FAILED)
			throw std::runtime_error("mmap") ;
	}
	~FMap()
	{
		munmap(buff, size) ;
		close(fd) ;
	}
} ;

int main(int argc, char *argv[])
{
	static char hex[256][5] = {
		"00 ", "01 ", "02 ", "03 ", "04 ", "05 ", "06 ", "07 ",
		"08 ", "09 ", "0A ", "0B ", "0C ", "0D ", "0E ", "0F ",
		"10 ", "11 ", "12 ", "13 ", "14 ", "15 ", "16 ", "17 ",
		"18 ", "19 ", "1A ", "1B ", "1C ", "1D ", "1E ", "1F ",
		"20 ", "21 ", "22 ", "23 ", "24 ", "25 ", "26 ", "27 ",
		"28 ", "29 ", "2A ", "2B ", "2C ", "2D ", "2E ", "2F ",
		"30 ", "31 ", "32 ", "33 ", "34 ", "35 ", "36 ", "37 ",
		"38 ", "39 ", "3A ", "3B ", "3C ", "3D ", "3E ", "3F ",
		"40 ", "41 ", "42 ", "43 ", "44 ", "45 ", "46 ", "47 ",
		"48 ", "49 ", "4A ", "4B ", "4C ", "4D ", "4E ", "4F ",
		"50 ", "51 ", "52 ", "53 ", "54 ", "55 ", "56 ", "57 ",
		"58 ", "59 ", "5A ", "5B ", "5C ", "5D ", "5E ", "5F ",
		"60 ", "61 ", "62 ", "63 ", "64 ", "65 ", "66 ", "67 ",
		"68 ", "69 ", "6A ", "6B ", "6C ", "6D ", "6E ", "6F ",
		"70 ", "71 ", "72 ", "73 ", "74 ", "75 ", "76 ", "77 ",
		"78 ", "79 ", "7A ", "7B ", "7C ", "7D ", "7E ", "7F ",
		"80 ", "81 ", "82 ", "83 ", "84 ", "85 ", "86 ", "87 ",
		"88 ", "89 ", "8A ", "8B ", "8C ", "8D ", "8E ", "8F ",
		"90 ", "91 ", "92 ", "93 ", "94 ", "95 ", "96 ", "97 ",
		"98 ", "99 ", "9A ", "9B ", "9C ", "9D ", "9E ", "9F ",
		"A0 ", "A1 ", "A2 ", "A3 ", "A4 ", "A5 ", "A6 ", "A7 ",
		"A8 ", "A9 ", "AA ", "AB ", "AC ", "AD ", "AE ", "AF ",
		"B0 ", "B1 ", "B2 ", "B3 ", "B4 ", "B5 ", "B6 ", "B7 ",
		"B8 ", "B9 ", "BA ", "BB ", "BC ", "BD ", "BE ", "BF ",
		"C0 ", "C1 ", "C2 ", "C3 ", "C4 ", "C5 ", "C6 ", "C7 ",
		"C8 ", "C9 ", "CA ", "CB ", "CC ", "CD ", "CE ", "CF ",
		"D0 ", "D1 ", "D2 ", "D3 ", "D4 ", "D5 ", "D6 ", "D7 ",
		"D8 ", "D9 ", "DA ", "DB ", "DC ", "DD ", "DE ", "DF ",
		"E0 ", "E1 ", "E2 ", "E3 ", "E4 ", "E5 ", "E6 ", "E7 ",
		"E8 ", "E9 ", "EA ", "EB ", "EC ", "ED ", "EE ", "EF ",
		"F0 ", "F1 ", "F2 ", "F3 ", "F4 ", "F5 ", "F6 ", "F7 ",
		"F8 ", "F9 ", "FA ", "FB ", "FC ", "FD ", "FE ", "FF "
	} ;

	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " file1 file2" << std::endl ;
		return -1 ;
	}

	FMap f1(argv[1]) ;
	FMap f2(argv[2]) ;
	BDiff test(f1.buff, f1.size, f2.buff, f2.size) ;
bool result = true;
int r_mod=0,r_new=0,r_lost=0;

	for (std::list<BDiff::Edit>::iterator b = test.diff.begin(); b != test.diff.end(); ++b)
	{
		//std::cout << "@offset: " << (void *)(*b).offset ;
		printf("%08p ",(void *)(*b).offset);

		switch ((*b).act)
		{
			case BDiff::INS:
				std::cout << "+ " << (*b).sz << " ";
				r_new++;
				break ;

			case BDiff::REM:
				std::cout << "- " << (*b).sz << " ";
				r_lost++;
				break ;

			case BDiff::SUB:
				std::cout << "= " << (*b).sz << " ";
				r_mod++;
				break;
		}

		//std::cout << ((*b).sz == 1 ? " byte\n" : " bytes\n") ;

		if (!(*b).buff.empty())
		{
			std::vector<char>::iterator c((*b).buff.begin()) ;
			size_t x(1) ;

			while (c != (*b).buff.end())
			{
//				if (!(x%16))
//					std::cout << "\n" ;

				std::cout << hex[(unsigned char)*c] ;
				++c ;
				++x ;
			}
		}

		std::cout << "\n" ;
	}

	std::cerr
		<< "New bytes in B: " << r_new << std::endl
		<< "Lost bytes in A: " << r_lost << std::endl
		<< "Modified bytes: " << r_mod << std::endl;

	return 0 ;
}
