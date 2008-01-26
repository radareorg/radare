public class Radare.Project
{
	// must return value
	public static bool open(weak string file)
	{
		return project_open(file);
	}

	public static bool save(weak string file)
	{
		return project_save(file);
	}

	public static void info(weak string file)
	{
		project_save(file);
	}
}

[Import()]
public static bool project_open(weak string file);

[Import()]
public static bool project_save(weak string file);

[Import()]
public static void project_info(weak string file);
