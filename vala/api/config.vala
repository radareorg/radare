using GLib;

public class Radare.Config : Object
{
	construct {
		config_init();
	}

	// must return value
	public void set(weak string key, weak string val)
	{
		config_set(key, val);
	}

	public void set_i(weak string key, long i)
	{
		config_set_i(key, i);
	}

	public weak string get(weak string key)
	{
		return config_get(key);
	}

	public long get_i(weak string key)
	{
		return config_get_i(key);
	}

	public bool rm(weak string key)
	{
		return config_rm(key);
	}

	public void list(weak string mask)
	{
		config_list(mask);
	}
}

[Import()]
public static void config_init();

[Import()]
public static void *config_set(weak string key, weak string val);

[Import()]
public static void *config_set_i(weak string key, long i);

[Import()]
public static weak string config_get(weak string key);

[Import()]
public static long config_get_i(weak string key);

[Import()]
public static bool config_rm(weak string key);

[Import()]
public static void config_list(weak string mask);
