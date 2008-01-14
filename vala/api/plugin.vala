using GLib;

public class Radare.Plugins : Object
{
	public static void list()
	{
		plugin_list();
	}

}

[Import()]
public static void plugin_list();
