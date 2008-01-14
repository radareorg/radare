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

/*
  Usage example:

  Radare r = new Radare("/bin/ls");
  Search s = new Search(r, "lib");
  while(s.hasMoreResults()) {
	System.out.println("Match: "+r.getFlag());
  }
  r.setFlag(s.getOriginalFlag());
*/

package org.nopcode.pancake.radare;

public class Search
{
  // TODO: regexp
  public final static int BINARY   =  0x0001;
  public final static int STRING   =  0x0002;
  public final static int BACKWARD =  0x0010;
  public final static int FORWARD  =  0x0020;
  public final static int PERBLOCK =  0x0100;
  public final static int PERMATCH =  0x0200;
  public final static int DEFAULT  =  STRING|FORWARD|PERBLOCK;
  Flag oflag;
  Radare r;
  String ss; // search string
  boolean eof = true;
  boolean delta = false;
  int options;

  public Search(Radare r, String string)
  {
	this(r, string, Search.DEFAULT);
  }

  public Search(Radare r, String string, int options)
  {
	oflag = r.getFlag();
	this.r = r;
	this.ss = string;
	this.setOptions(options);
  }

  public Flag getOriginalFlag()
  {
	return oflag;
  }

  public void setOptions(int options)
  {
	this.options = options;

	if (((options & FORWARD)==0) && ((options & BACKWARD)==0))
		this.options|=FORWARD;

	if (((options & PERBLOCK)==0) && ((options & PERMATCH)==0))
		this.options|=PERBLOCK;

	if (((options & BINARY)==0) && ((options & STRING)==0))
		this.options|=STRING;
  }

  public void reset()
  {
	eof = true;
	r.setFlag(oflag);
  }

  private boolean searchNotFinished(long limit)
  {
	if (eof)
		return true;

	if ((options & Search.BACKWARD) != 0) {
		if (limit==-1) {
			if (r.getOffset() <= 0)
				return false;
			else
				return true;
		} else {
			if (r.getOffset() > limit)
				return true;
		}
	} else {
	//if ((options & Search.FORWARD) != 0) {
		if (limit!=-1) {
			if ( r.getOffset() >= limit )
				return false;
			return true;
		} // else done by readBlock()
		return true;
	}

	return false;
  }

  private long getNextBlock()
  {
	if ((options & Search.BACKWARD) != 0)
		return r.prevBlock();

	return r.nextBlock();
  }

  public boolean hasMoreResults()
  {
	long limit = r.getLimit();

	if (eof)
		eof = false;

	/* perform the search */
	try {
		// XXX  : backward+permatch is not ok because of delta
		// TODO : if ( string > block ) throw something
		while( this.searchNotFinished(limit) ) {

			if ((delta) && ((options & Search.FORWARD) != 0))
				r.setOffset( r.getOffset() + 1);
			else	r.setOffset( this.getNextBlock() );

			String str = new String( r.getBytes() );

			int off = str.indexOf(ss);
			if ( off != -1 ) {
				if ((options & Search.PERMATCH) != 0) {
					r.seekTo(r.getOffset() + off);
					delta = true;
				}
				return true;
			} else {
				delta = false;
			}
		}
	} catch(Exception e) {
		// eof
	}

	return false;
  }
}
