using GLib;
using Gtk;

public class RadareGUI.RadgetInformation : Radget, GLib.Object
{
	Line file;
	Line size;
	VBox vb;

	public Box# box { get { return vb; } }
	public string# name { get { return "Information"; } }

	construct {
		vb = new VBox(false, 2);
		file = new Line().name("File","/bin/ls");
		vb.add(file);
		size = new Line().numeric("Size", 38432);
		vb.add(size);
		refresh();
	}

	public void refresh()
	{
	}
}
