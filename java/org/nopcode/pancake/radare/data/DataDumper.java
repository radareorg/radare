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

public class DataDumper
{
  public static void println(byte[] buffer)
  {
	System.out.println( DataDumper.toString(buffer) );
  }

  public static String toString(byte b)
  {
	char[] hexa = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	return new String(
		""+hexa[(0xff&b)>>4]+
		""+hexa[(0xff&b)&0xf]);
  }

// TODO: use toOctalString(), toHexString() from Integer class
  public static String toString(byte[] buffer)
  {
	StringBuffer sb = new StringBuffer();

	for(int i=0;i<buffer.length;i++)
		sb = sb.append( DataDumper.toString(buffer[i])+" ");

	return sb.toString().trim();
  }

  public static String toString0(byte[] buffer)
  {
	StringBuffer sb = new StringBuffer();
	for(int i=0; (buffer[i]!=0) && (i<buffer.length); i++)
		sb = sb.append(isAscii(buffer[i])?""+(char)buffer[i]:".");
	return sb.toString();
  }

  public static boolean isAscii(byte b)
  {
	if ((0xff&b)<' '||(0xff&b)>'~')
		return false;
	return true;
  }

  public static String toAscii(byte[] buffer)
  {
	StringBuffer sb = new StringBuffer();
	for(int i=0; i<buffer.length; i++)
		sb = sb.append(isAscii(buffer[i])?""+(char)buffer[i]:".");
	return sb.toString();
  }

  public static String[] toStrings(byte[] buffer)
  {
	return DataDumper.toString(buffer).split(" ");
  }
}
