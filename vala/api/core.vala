using GLib;

public class Radare.Core : Object
{
	public static string VERSION = "0.8.8";
	private static bool isinit = false;

/*
	public static block_size {
		get {
		}
		set {
		}
	}
*/

	private static void _init_() {
		radare_init();
		plugin_init();
		env_prepare("");
		isinit = true;
	}

	construct {
		_init_();
	}

	public static string command(string command)
	{
		return pipe_command_to_string(command);
	}

	public static int cmd(string command)
	{
		return radare_cmd(command, 0);
	}

	public static bool open(string file, bool write_mode)
	{
		if (!isinit) _init_();
		int ret = radare_open(file, write_mode?1:0);
		return (ret == 0);
	}

	public static bool seek(ulong offset)
	{
		bool ret = radare_seek(offset, 0);
		radare_read(0);
		return ret;
	}

	/**
	 * Returns the delta byte of the current block
	 */
	public static uchar get(int delta)
	{
		return radare_get(delta);
	}

	public static void close()
	{
		radare_close();
	}
}

[Import()]
public extern static string pipe_command_to_string(string command);

[Import()]
public extern static int radare_cmd(string command, int arg);

[Import()]
public extern static int system(string str);

[Import()]
public extern static int radare_init();

[Import()]
public extern static int plugin_init();

[Import()]
public extern static void radare_read(int next);

[Import()]
public extern static int radare_close();

[Import()]
public extern static int radare_open(string file, int allow_write);

[Import()]
public extern static void env_prepare(string str);

[Import()]
public extern static bool radare_seek(ulong offset, int which);

[Import()]
public extern static uchar radare_get(int delta);
