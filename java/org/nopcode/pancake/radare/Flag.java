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

/**
 *
 * This class stores the internal state of radare
 *
 */
package org.nopcode.pancake.radare;

public class Flag implements Cloneable
{
  public int bsize   = Radare.BLOCK_SIZE;
  public long offset =  0;
  public long limit  = -1;

  public Flag()
  {
  }

  public Flag(int bsize, long offset)
  {
	this.bsize  = bsize;
	this.offset = offset;
  }

  public Flag(int bsize, long offset, long limit)
  {
	this(bsize, offset);
	this.limit = limit;
  }

  public Object clone()
  {
	Object obj = null;
	try {
		obj = super.clone();
	} catch(CloneNotSupportedException ex) {
		System.out.println("Cannot clone this object");
	}
	return obj;
  }

  public String toString()
  {
	return "bsize="+bsize+";offset="+offset+";limit="+limit;
  }
}
