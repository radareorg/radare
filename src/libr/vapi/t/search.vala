/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

using Radare;

public class SearchExample
{
	public static int cb(Search.Keyword kw, void *user, uint64 addr)
	{
		stdout.printf("Hit %d! at 0x%llx\n", (int)kw.count, addr);
		return 0;
	}

	public static void main(string[] args)
	{
		string buf = "liblubliuamlibfoo";
		Search.State s = new Search.State(Search.Mode.KEYWORD);
		s.kw_add("lib", "");
		s.set_callback(cb, null);
/*
		s.set_callback(
			(user, addr) = {
				stdout.printf("Hit!\n")
			}, null);
*/
		s.initialize();

		stdout.printf("string: \"%s\"\n", buf);
		stdout.printf("length: %d\n", (int)buf.len());
		s.update_i(0LL, (uint8*)buf, (uint32)buf.len());
		s = null;
	}
}
