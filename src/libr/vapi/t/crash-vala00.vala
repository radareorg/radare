using Radare;

public class SearchExample
{

	public static void main(string[] args)
	{
		string buf = "liblubliuamlibfoo";
		Search.State s = new Search.State(Search.Mode.KEYWORD);
		s.kw_add("lib", "");
		s.set_callback(
			(user, addr) = {
				stdout.printf("Hit!\n")
			}, null);
		s.initialize();
		stdout.printf("length: %d\n", (int)buf.len());
		s.update_i(0LL, (uint8*)buf, (uint32)buf.len());
		s = null;
	}
}
