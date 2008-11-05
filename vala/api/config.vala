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
public extern static void config_init();

[Import()]
public extern static void *config_set(string key, string val);

[Import()]
public extern static void *config_set_i(string key, long i);

[Import()]
public extern static string config_get(string key);

[Import()]
public extern static long config_get_i(string key);

[Import()]
public extern static bool config_rm(string key);

[Import()]
public extern static void config_list(string mask);
