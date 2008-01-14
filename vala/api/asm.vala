using GLib;

public class Radare.Asm
{
	public static bool setArchitecture(string arch)
	{
		if (arch == "arm")
			Radare.Utils.set("ARCH", "arm");
		else
		if (arch == "intel")
			Radare.Utils.set("ARCH", "intel");
		else
			return false;
		return true;
	}

	public static string getArchitecture()
	{
		return Radare.Utils.get("ARCH");
	}

	public static string asm(string code)
	{
		return Radare.Core.command("!rsc asm '"+code+"'");
	}

	public static string dasm(string hexpairs)
	{
		return Radare.Core.command("!rsc dasm '"+hexpairs+"'");
	}
}
