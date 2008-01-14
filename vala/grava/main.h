/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007  pancake <youterm.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *

using GLib;
using Cairo;
using Gtk;
using Gdk;
using Grava;

public class Grava.Main : Gtk.Window {
	Grava.Widget grava;

	construct {
		title = "Grava Demo";
		destroy += w => {
			Gtk.main_quit();
		};
		set_default_size (450, 550);

		// initialize graph
		grava = new Grava.Widget();
		VBox vbox = new VBox(false, 0);
		HBox hbox = new HBox(false, 0);
			hbox.add(new Entry());
			Button b = new Button().from_stock("gtk-refresh");
			b.clicked += btn => {
				grava.graph.update();
				grava.graph.draw(Gdk.cairo_create(grava.da.window));
			};
			hbox.pack_start(b, false, false, 2);
		vbox.pack_start(hbox, false, false, 2);
		vbox.add(grava.get_widget());
		add(vbox);
	}

	// capture mouse motion
	private bool button_press (DrawingArea da, Gdk.Event event)
	{
		weak EventButton eb = ref event.button;
		if (eb.button == 3) {
			Menu menu = new Menu();
			ImageMenuItem imi;

			imi = new ImageMenuItem.with_label("Cum on this node");
			menu.append(imi);

			imi = new ImageMenuItem.with_label("Focus this node");
			menu.append(imi);

			imi = new ImageMenuItem.with_label("Remove");
			menu.append(imi);

			imi = new ImageMenuItem.with_label("Hide");
			menu.append(imi);

			menu.show_all();
			menu.popup(null, null, null, null, eb.button, 0);
		}
		return true;
	}

	static int main (string[] args)
	{
		Gtk.init (out args);

		var win = new Main();
		win.show_all ();

		Gtk.main ();
		return 0;
	}
}
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS




G_END_DECLS

#endif
