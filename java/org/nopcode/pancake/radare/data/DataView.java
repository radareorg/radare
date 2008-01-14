/*
 * Copyright (C) 2006
 *       pancake <pancake@youterm.com>
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

package org.nopcode.pancake.radare.data;

import org.nopcode.pancake.radare.Endian;

/**
 * TODO:
 *  - Float
 *  - Date (unix, dos)
 *  - int64 (long)
 */
public class DataView
{
  /*
   * Get Int32 representation of the initial 4 bytes of byte[] buffer.
   */
  public static int getInteger(byte[] buffer)
  {
	return getInteger(buffer, Endian.BIG);
  }

  public static int getInteger(byte[] buffer, Endian endian)
  {
	int i = 0;
	if (endian == Endian.BIG) {
		i|=(int)((buffer[0]<<24&0xff000000));
		i|=(int)((buffer[1]<<16&0x00ff0000));
		i|=(int)((buffer[2]<<8&0x0000ff00));
		i|=(int)((buffer[3]&0x000000ff));
	} else {
		i|=(int)((buffer[3]<<24&0xff000000));
		i|=(int)((buffer[2]<<16&0x00ff0000));
		i|=(int)((buffer[1]<<8&0x0000ff00));
		i|=(int)((buffer[0]&0x000000ff));
	}
	return i;
  }

  /*
   * Get Int16 representation of the initial 2 bytes of byte[] buffer.
   */
  public static int getShort(byte[] buffer)
  {
	return getShort(buffer, Endian.BIG);
  }

  public static short getShort(byte[] buffer, Endian endian)
  {
	short i = 0;
	if (endian == Endian.BIG) {
		i|=(int)((buffer[0]<<8&0xff000000));
		i|=(int)((buffer[1]&0x00ff0000));
	} else {
		i|=(int)((buffer[1]<<8&0x0000ff00));
		i|=(int)((buffer[0]&0x000000ff));
	}
	return i;
  }
}
