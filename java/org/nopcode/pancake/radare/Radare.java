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

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.EOFException;
import java.io.FileNotFoundException;

public class Radare extends State implements StateListener
{
  public final static int BLOCK_SIZE = 512;

  private RandomAccessFile raf = null;
  private String filename      = "";
  private byte[] buffer        = null;

  public void stateEvent(StateEvent ev) throws Exception
  {
	switch(ev) {
	case BLOCKSIZE:
		this.buffer = new byte[super.getBlockSize()];
		break;
	case OFFSET:
		if (super.getOffset() > raf.length())
			throw new EOFException();
		raf.seek(super.getOffset());
		break;
	}

	this.readBlock();
  }

  public Radare(Radare r)
  {
	this();
	try {
		this.openFile(new File(r.getFileName()));
	} catch(Exception e) {
		e.printStackTrace();
	}
	this.setFlag( (Flag) r.getFlag().clone() );
  }

  public Radare()
  {
	super();
	buffer = new byte[super.getBlockSize()];
	setListener((StateListener)this);
  }

  public Radare(String file) throws Exception
  {
	this();
	openFile(new File(file));
  }

  public String getFileName()
  {
	return filename;
  }

  public void openFile(File file) throws Exception
  {
	this.openFile(file, "r");
  }

  public void openFile(File file, String mode) throws Exception
  {
	this.closeFile();
	this.raf = new RandomAccessFile(file.toString(), mode);
	this.filename = file.toString();
	this.readBlock();
  }

  public void closeFile()
  {
	this.filename = "";
	try {
		if (this.raf != null)
			this.raf.close();
	} catch(IOException e) {
		e.printStackTrace();
	}
  }

  public boolean seekTo(long offset)
  {
	try {
		super.setOffset(offset);
	} catch(Exception e) {
		e.printStackTrace();
		return false;
	}

	return true;
  }

  private boolean readBlock()
  {
	try {
		raf.seek(super.getOffset());
	} catch(Exception e) {
		e.printStackTrace();
		return false;
	}

	try {
		raf.read(this.buffer);
	} catch(IOException e) {
		return false;
	}

	return true;
  }

  public byte[] getBytes()
  {
	return this.buffer;
  }
}
