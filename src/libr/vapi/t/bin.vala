/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

using Radare;

public class BinExample
{
	public static void main(string[] args)
	{
		uint64 baddr;

		Bin bin = new Bin("/bin/ls", 0);
		stdout.printf("Info\n");
		baddr = bin.get_baddr();
		stdout.printf("Base addr: 0x%08llx\n", baddr);

		Entry[] e = bin.get_entry();
		Entry entry = e[0];
		stdout.printf("Entry point: 0x%08llx\n", entry.offset);
		bin.close();
	}
}
