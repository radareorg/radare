using GLib;
using Gtk;
using Vte;

public class RadareGUI.Console : VBox
{
	Entry entry;
	Label label;
	
	construct
	{
		HBox hb = new HBox(false, 3);

		label = new Label("");
		add(label);
		entry = new Entry();
		entry.activate += b => {
			label.set_text( Radare.Core.command(entry.get_text()) );
			entry.set_text("");
		};
		hb.add(entry);

		Button run = new Button.from_stock("gtk-execute");
		run.clicked += b => {
			label.set_text( Radare.Core.command(entry.get_text()) );
			entry.set_text("");
		};
		hb.pack_start(run,false,false,0);

		Button open = new Button.from_stock("gtk-open");
		open.clicked += b => {
		};
		hb.pack_start(open,false,false,0);

		pack_start(hb,false,false,3);
	}

	public VBox get()
	{
		return this;
	}

}
