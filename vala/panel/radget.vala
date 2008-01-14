using GLib;
using Gtk;

public interface RadareGUI.Radget : GLib.Object
{
	public abstract string# name { get; }
	public abstract Box# box { get; }
	public abstract void refresh();
}
