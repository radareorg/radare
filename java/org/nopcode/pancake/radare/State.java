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

public class State
{
  private Flag flag = new Flag();
  private StateListener sl;

  public Flag getFlag()
  {
	return flag;
  }

  public void setFlag(Flag flag)
  {
	this.flag = flag;
  }

  public long getLimit()
  {
	return this.flag.limit;
  }

  public void setLimit(long limit)
  {
	this.flag.limit = limit;
  }

  /*
   * Instantiate a new State class with an optional StateListener callback
   */
  public State()
  {
  }

  public State(StateListener sl)
  {
	setListener(sl);
  }

  public void setListener(StateListener sl)
  {
	this.sl = sl;
  }

  /*
   * Get and Set for the BlockSize
   */
  public int getBlockSize()
  {
	return flag.bsize;
  }

  public void setBlockSize(int bsize)
  {
	if (bsize<1)
		bsize = 1;
	this.flag.bsize = bsize;
	try {
	if (sl != null)
		sl.stateEvent(StateEvent.BLOCKSIZE);
	} catch(Exception e) {
		// no exception here. just skip't
	}
  }

  /*
   * Get and Set for the Offset
   */
  public long getOffset()
  {
	return flag.offset;
  }

  public void setOffset(long offset) throws Exception
  {
	this.flag.offset = offset;
	if (sl != null)
		sl.stateEvent(StateEvent.OFFSET);
  }

  public long nextBlock()
  {
	return this.flag.offset + this.flag.bsize;
  }

  public long prevBlock()
  {
	long off = this.flag.offset - this.flag.bsize;
	if (off<0) off=0;
	return off;
  }
}
