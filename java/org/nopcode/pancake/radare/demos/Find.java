package org.nopcode.pancake.radare.demos;

import org.nopcode.pancake.radare.*;
import org.nopcode.pancake.radare.data.DataDumper;
import org.nopcode.pancake.radare.data.DataView;

public class Find
{
  public static void main(String[] args)
  {
	Radare r = null;

	try {
		r = new Radare(args[0]);
	} catch(Exception e) {
		System.out.println("Usage: java Find [file] [string]");
		System.exit(1);
	}

	r.setBlockSize(100);
	Search s = new Search(r, args[1]);
	s.setOptions(Search.PERMATCH);
	while(s.hasMoreResults()) {
		System.out.println(""+r.getOffset()+"  "+DataDumper.toString0(r.getBytes()));
	}     
	r.setFlag(s.getOriginalFlag());
  }
}
