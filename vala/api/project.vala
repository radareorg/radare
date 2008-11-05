public class Radare.Project
{
	// must return value
	public static bool open(string file)
	{
		return project_open(file);
	}

	public static bool save(string file)
	{
		return project_save(file);
	}

	public static void info(string file)
	{
		project_save(file);
	}
}

[Import()]
public extern static bool project_open(string file);

[Import()]
public extern static bool project_save(string file);

[Import()]
public extern static void project_info(string file);
