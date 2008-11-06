using GLib;

public class Radare.Disassembler
{
	public string INTEL = "intel";
	public string ARM = "arm";
	public string JAVA = "java";

	public static weak string arch()
	{
		return Radare.Utils.get("ARCH");
	}

	
	//public static string# arch { get { return Radare.Utils.env("ARCH"); } }
//	public static string# syntax { get { return getenv("SYNTAX"); } }
}
