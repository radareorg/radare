package org.nopcode.pancake.radare.demos;

import org.nopcode.pancake.radare.*;
import org.nopcode.pancake.radare.data.DataDumper;
import org.nopcode.pancake.radare.data.DataView;

public class ElfHeader
{
  public static void main(String[] args)
  {
	Radare r = null;

	try {
		r = new Radare(args[0]);
	} catch(Exception e) {
		System.out.println("Usage: java ElfHeader [file]");
		System.exit(1);
	}

	r.setBlockSize(4); /* 32 bits */
	r.seekTo(0x18); /* elf_header.eh_entry */

	Flag f = r.getFlag();
	System.out.println("Flag: "+f.toString());

	System.out.print("entry_point = ");
	String[] bytes = DataDumper.toStrings(r.getBytes());
	System.out.print("0x");
	for(int i = bytes.length-1; i>-1; i--) {
		System.out.print(bytes[i]);
	}
	System.out.println("");
	System.out.printf("ENTRYPOINT = 0x%x\n",
		DataView.getInteger(r.getBytes(),Endian.LITTLE));

	System.out.println(DataView.getInteger(r.getBytes(),Endian.LITTLE));

	System.out.println("Search: 'lib'");
	r.setBlockSize(100);
//	r.seekTo(0x4000);
	Search s = new Search(r, "lib");
	s.setOptions(Search.PERMATCH);
	while(s.hasMoreResults()) {
		System.out.println("Match: " + r.getFlag());
		System.out.println("  > " + DataDumper.toString0(r.getBytes()));
	}     
	r.setFlag(s.getOriginalFlag());
  }
}
