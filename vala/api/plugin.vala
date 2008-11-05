using GLib;

/*
public interface Radare.Plugin
{
	public abstract static void run(string arg);
}
*/

public class Radare.Plugins : Object
{
	public static void list()
	{
		plugin_list();
	}

}

[Import()]
public extern static void plugin_list();
