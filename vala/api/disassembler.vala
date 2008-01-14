using GLib;

public class Radare.Disassembler
{
	public static string# INTEL { get { return "intel"; } }
	public static string# ARM   { get { return "arm"; } }
	public static string# JAVA  { get { return "java"; } }

	public static weak string arch()
	{
		return Radare.Utils.get("ARCH");
	}

	
	//public static string# arch { get { return Radare.Utils.env("ARCH"); } }
//	public static string# syntax { get { return getenv("SYNTAX"); } }
}
