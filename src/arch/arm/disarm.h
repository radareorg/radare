/* DISARM (header)
 * Simple-minded ARM disassembler
 * (C) Copyright 1999 Kevin F. Quinn, all rights reserved
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.*
 *
 * The GNU General Public License is also available on the World Wide Web
 * at http://www.gnu.org/copyleft/gpl.html
 *
 * To contact the author, email kevq@banana.demon.co.uk
 */

#ifndef _disarm_h
#define _disarm_h

#define DISARM_MAXSTRINGSIZE 256

typedef unsigned long int RawInstruction;

//char *disarm (RawInstruction rawinstruction, int offset);
char *disarm (unsigned long int instruction, int offset);

#endif
