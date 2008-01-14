public class Radare.Debugger
{
	public static void step()
	{
		Radare.Core.cmd("!step");
	}

	public static void step_over()
	{
		Radare.Core.cmd("!stepo");
	}

	public static void play()
	{
		Radare.Core.cmd("!cont");
	}
}
