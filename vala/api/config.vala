using GLib;

public class Radare.Config : Object
{
	construct {
		config_init();
	}

	// must return value
	public void set(string key, string val)
	{
		config_set(key, val);
	}

	public void set_i(string key, long i)
	{
		config_set_i(key, i);
	}

	public string get(string key)
	{
		return config_get(key);
	}

	public long get_i(string key)
	{
		return config_get_i(key);
	}

	public bool rm(string key)
	{
		return config_rm(key);
	}

	public void list(string mask)
	{
		config_list(mask);
	}
}

[Import()]
public static void config_init();

[Import()]
public static void *config_set(string key, string val);

[Import()]
public static void *config_set_i(string key, long i);

[Import()]
public static string config_get(string key);

[Import()]
public static long config_get_i(string key);

[Import()]
public static bool config_rm(string key);

[Import()]
public static void config_list(string mask);
