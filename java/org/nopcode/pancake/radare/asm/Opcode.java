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

package org.nopcode.pancake.radare.asm;

import java.util.Formatter;

public class Opcode
{
  String opcode = null;
  String args = null;
  String line = null;
  int offset = 0;
  int size = 0;
  
  public Opcode(String line)
  {
  	String[] words = line.split(":");
  
  	if (words.length == 2) {
  		this.line = line = words[1].trim();
  
  		try {
  			offset = Integer.decode("0x"+words[0].trim()).intValue();
  		} catch(Exception e) {
  			e.printStackTrace();
  		}
  
  		String[] wos = line.split("\t");
  		this.size = wos[0].trim().split(" ").length;
		if (wos.length>0)
			this.line = wos[1].trim();

		wos = this.line.split(" ");
		if (wos.length>0) {
			opcode = wos[0].trim();
			if (wos.length>2) {
				String[] str = this.line.split(" ");
				args = str[str.length-1];
			}
		}
  	} else {
  		this.line = line;
  	}
  
  }

  public String getArgument()
  {
	return args;
  }

  public String getOpcode()
  {
	return opcode;
  }
  
  public int getOffset()
  {
  	return offset;
  }
  
  public int getSize()
  {
  	return size;
  }

  public static String cleanup(String str)
  {
	str = str.replace("$","");
	str = str.replace("%","");
	return str;
  }
  
  public String toString()
  {
  	return line;
  }
}
