using GLib;
using Gtk;

public class RadareGUI.RadgetDebugger : Radget, GLib.Object
{
	Line file;
	Line size;
	public VBox vb;
	public Box# box { get { return vb; } }

	public string# name { get { return "Debugger"; } }

	construct {
		vb= new VBox(false, 2);
		vb.add(new Label("(TODO)"));
		refresh();
	}

	public void refresh()
	{
	}
}
