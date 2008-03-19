using GLib;
using Gtk;

public class RadareGUI.MainWindow : Window
{
        public static Statusbar statusbar;
        public List left_list;
        public List right_list;
	public Term shell;
	public Visor visor;
	public Term con;
	private Panel panel;

        construct
        {
                create_widgets ();
                title = "gradare vala frontend";
                border_width = 3;
                set_position(WindowPosition.CENTER);
                resize(700, 500);
        }

	/* reload information */
	private void load()
	{
	}

        private void create_widgets()
        {
		destroy += w => {
			Gtk.main_quit();
		};

		shell = new Term();
		visor = new Visor();
		con   = new Term();

		VBox root = new VBox(false, 0);
		root.pack_start(menu(), false, false, 0);
		//root.pack_start(toolbar(),false,false, 2);

		HPaned hp = new HPaned();
		
		VBox vb = new VBox(false, 0);

		Notebook nb = new Notebook();
		nb.append_page(visor.get(), new Label("Visor"));
		nb.append_page(con.get(), new Label("Terminal"));
		nb.append_page(shell.get(), new Label("Terminal"));
		nb.append_page(comments(), new Label("Database"));
		vb.add(nb);

		panel = new Panel();
		hp.add(panel);
		hp.add(vb);
		root.add(hp);
		hp.set_position(170);

                statusbar = new Statusbar();
                statusbar.push(0, "Interface loaded.");
                root.pack_end(statusbar, false, false, 0);

		add(root);
	}

	private Widget comments()
	{
		return new Label("(TODO)");
	}

	private MenuBar menu()
	{
		MenuBar bar = new MenuBar();
		Menu m = new Menu();
			MenuItem mo = new MenuItem.with_label("Open");
			mo.activate += m => {
				FileChooserDialog fcd = new
					FileChooserDialog(
						"Open file",
						this, 
						FileChooserAction.OPEN,
						"gtk-cancel", 0,
						"gtk-ok", 1);
				if (fcd.run()==1) {
					Radare.Core.open(fcd.get_filename(), false);
					this.load();
				}
				fcd.destroy();
				fcd = null;
			};
			m.append(mo);
			mo = new MenuItem.with_label("Exit");
			mo.activate += m => {
				Gtk.main_quit();
			};
			m.append(mo);

		MenuItem root_menu = new MenuItem.with_label("File");
		root_menu.set_submenu(m);
		bar.append(root_menu);

		m = new Menu();
			mo = new MenuItem.with_label("Copy");
			m.append(mo);
		root_menu = new MenuItem.with_label("Edit");
		root_menu.set_submenu(m);

		/* Help menu */
		m = new Menu();
			mo = new MenuItem.with_label("About");
			m.append(mo);
		root_menu = new MenuItem.with_label("Help");
		root_menu.set_submenu(m);

		bar.append(root_menu);

		return bar;
	}

	private Toolbar toolbar()
	{
		Toolbar t = new Toolbar();
		t.style = ToolbarStyle.ICONS;
		ToolItem ti = new ToolItem();
	
		t.add(ti);
/*
		t.append_item("new", "new", "", new Gtk.Image.from_stock(Gtk.STOCK_SAVE),
			Gtk.IconSize.MENU, null, 0);
*/
		return t;
	}
}
