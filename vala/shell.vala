using GLib;
using Gtk;
using Vte;

public class RadareGUI.Shell : VBox
{
	Terminal term;
	Entry entry;

	construct
	{
		term = new Terminal();
		add(term);
		HBox hb = new HBox(false, 3);
		entry = new Entry();
		entry.activate += b => {
			term.feed_child(entry.get_text(),(uint)entry.get_text().size());
			term.feed_child("\n",1);
			entry.set_text("");
		};
		hb.add(entry);

		Button run = new Button.from_stock("gtk-execute");
		run.clicked += b => {
			term.feed_child(entry.get_text(),(uint)entry.get_text().size());
			term.feed_child("\n",1);
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
		string[] argv = new string[]{"/bin/bash", null};
		//term.fork_command("/bin/bash", argv,  null, "/", false, false, false);
		return this;
	}

}
