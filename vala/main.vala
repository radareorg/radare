using GLib;
using Gtk;

public class RadareGUI.Main
{
  public static void main(string[] args)
  {
	Radare.Core r = new Radare.Core();

//	Radare.Asm.setArchitecture("arm");
	//stdout.printf("DISASM : "+ Radare.Asm.asm("mov eax, 0x33") +"\n");

//r.open("/bin/ls", false);

//Radare.Plugins.list();
//stdout.printf("%s\n", r.command("x"));
//stdout.printf("Architecture: %s\n", Radare.Disassembler.arch());
//	stdout.printf("%d\n".printf(Radare.Utils.value("0x33")));
//r.close();

	Gtk.init(out args);
	MainWindow mw = new MainWindow();
	mw.show_all();
	Gtk.main();
  }
}
