using GLib;
using Gtk;

public class RadareGUI.Panel : ScrolledWindow
{
	SList<Radget> radgets;

	construct {
		radgets = new SList<Radget>();
		set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
		add(load());
	}

	public void refresh()
	{
		foreach (Radget r in radgets) {
			r.refresh();
		}
	}

	private Widget load()
	{
		radgets.append(new RadgetInformation());
		radgets.append(new RadgetSearch());
		radgets.append(new RadgetDebugger());

		VBox vb = new VBox(false, 5);

		foreach (Radget r in radgets) {
			Expander ex = new Expander(r.name);
			ex.add(r.box);
			vb.pack_start(ex, false, false, 0);
		}

		return vb;
	}

	public Widget widget()
	{
		return this;
	}
}
