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

package org.nopcode.pancake.radare;

public enum Endian
{
  BIG("big", true),
  LITTLE("little", false);

  private final String name;
  private final boolean big_endian;

  Endian(String name, boolean big_endian)
  {
	this.name = name;
	this.big_endian = big_endian;
  }

  public String toString()
  {
	return name;
  }

  public boolean isBig()
  {
	return big_endian;
  }

  public boolean isLittle()
  {
	return !big_endian;
  }
}
