using GLib;
using Gtk;
using Vte;

public class RadareGUI.Logs : VBox
{

	construct
	{
		HBox hb = new HBox(false, 3);
		hb.add(new Entry());
		hb.pack_start(new Button.from_stock("gtk-execute"),false,false,0);

		pack_start(hb,false,false,3);
	}

	public VBox get()
	{
		return this;
	}

}
