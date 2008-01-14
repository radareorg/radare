using GLib;
using Gtk;

public class RadareGUI.RadgetSearch : Radget, GLib.Object
{
	Line file;
	public VBox vb;

	public Box# box { get { return vb; } }
	public string# name { get { return "Search"; } }

	construct {
		vb= new VBox(false, 2);
		file = new Line().name("String","");
		vb.add(file);
		file = new Line().name("HexPairs","");
		vb.add(file);

		HButtonBox hbb = new HButtonBox();
		hbb.set_layout(ButtonBoxStyle.END);
		hbb.add(new Button.from_stock("gtk-find"));
		vb.add(hbb);

		refresh();
	}

	public void refresh()
	{
	}
}
