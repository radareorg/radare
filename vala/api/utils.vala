using GLib;

public class Radare.Utils
{
	public static ulong value(string str)
	{
		return get_offset(str);
	}

	public static ulong math(string str)
	{
		return get_math(str);
	}

	public static weak string get(string str)
	{
		return getenv(str);
	}

	public static int set(string foo, string bar)
	{
		return setenv(foo, bar, 1);
	}
}

[Import]
public static weak string getenv(string str);

[Import]
public static int setenv(string foo, string bar, int force);

[Import]
public static ulong get_offset(string str);

[Import]
public static ulong get_math(string str);
