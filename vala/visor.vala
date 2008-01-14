using GLib;
using Gtk;

public class RadareGUI.Visor : VBox
{
	Label label;
	//List3 list;

	construct
	{
	/*	list = new List3().with_title("Address","Hexdump","Ascii");
		list.add("0x00000000", "00 00 7f 31 42 03 99 42 53 23", ".....B.0B97");
		list.add("0x00000010", "82 1a 00 81 42 93 72 42 1c 10", "82| rsa93..");
		add(list.widget);
*/

		HBox hb = new HBox(false, 3);
		label = new Label("miau");
		hb.pack_start(label,false,false,3);
		pack_start(hb,false,false,3);
	}

	public VBox get()
	{
		return this;
	}

}
