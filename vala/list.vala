/*
 * Copyright (C) 2007
 *       pancake <youterm.com>
 *
 * bluspam is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bluspam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bluspam; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

using GLib;
using Gtk;

/*
 *
 * Implements a Widget for GTK containing a list
 * of selectable items with scrollbar and treeview.
 *         - Yay
 *
 */

public class RadareGUI.List : GLib.Object
{
//  ListStore rows;
  TreeView tv;
  TreeViewColumn col;
  CellRendererText cr;
  ListStore ls;

  public ScrolledWindow widget {get;set;}
  public signal void action();

  construct {
	widget = new ScrolledWindow(null, null);
	widget.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);

	tv = new TreeView();
	tv.set_reorderable(true);
	tv.set_rules_hint(true);
	tv.model = ls = new ListStore(1, typeof(string));

	widget.add(tv);
  }

  public void update()
  {
	action();
  }

  public List with_title(string title)
  {
	tv.insert_column_with_attributes(0, title,
		new CellRendererText(), "text", 0, null);
	return this;
  }

  public void add(string item)
  {
	TreeIter iter;
	ls.append(ref iter);
	ls.set(ref iter, 0, item);
  }

  public string get()
  {
	TreeIter iter;
	TreeModel model;
	string str = "";

	TreeSelection sel = tv.get_selection();

	if (sel.count_selected_rows() == 1) {
		String foo = new String.sized(1024);
		sel.get_selected(ref model, ref iter);
		tv.model.get(ref iter, 0, foo);
		str = foo.str;
	}

	return str;
  }

  public void clear()
  {
	ls.clear();
  }
}
