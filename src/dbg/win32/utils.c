/*
 * Copyright (C) 2008
 *       th0rpe <nopcode.org>
 *
 * radare is part of the radare project
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
#define __addr_t_defined

#include <windows.h>
#include <stdio.h>
#include "../../utils.h"



/* XXX code repeated */
/*
void eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
*/

void print_lasterr(char *str)
{
    /* code from MSDN, :? */

    LPWSTR pMessage = L"%1!*.*s! %4 %5!*s!";
    DWORD_PTR pArgs[] = { (DWORD_PTR)4, (DWORD_PTR)2, (DWORD_PTR)L"Bill",  // %1!*.*s!
         (DWORD_PTR)L"Bob",                                                // %4
         (DWORD_PTR)6, (DWORD_PTR)L"Bill" };                               // %5!*s!
    const DWORD size = 100+1;
    WCHAR buffer[size];


    if (!FormatMessage( FORMAT_MESSAGE_FROM_STRING |
		   	FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        pMessage, 
                       0,  // ignored
                       0,  // ignored
                       (LPTSTR)&buffer, 
                       size, 
                       (va_list*)pArgs)) {
   
        eprintf("Format message failed with 0x%x\n", GetLastError());
        return;
    }

    eprintf("%s ::: %s\n", str, buffer);
}

