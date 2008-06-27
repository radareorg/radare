/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008  pancake <youterm.com>
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
 */

using GLib;
using Cairo;
using Gtk;
using Gdk;

public class Grava.Widget : GLib.Object {
	const int SIZE = 30;
	const double ZOOM_FACTOR = 0.1;
	[Widget] public DrawingArea da;
	public Grava.Graph graph;
	public const double S = 96;
	ScrolledWindow sw;
	Menu menu;

	public signal void load_graph_at(string addr);
	public signal void breakpoint_at(string addr);

	public Gtk.Widget get_widget()
	{
		return sw;
	}

	construct {
		graph = new Graph();
		graph.update();
		create_widgets ();
	}

	public void create_widgets ()
	{
		da = new DrawingArea ();

		/* add event listeners */
		da.add_events(  Gdk.EventMask.BUTTON1_MOTION_MASK |
				Gdk.EventMask.SCROLL_MASK |
				Gdk.EventMask.BUTTON_PRESS_MASK |
				Gdk.EventMask.BUTTON_RELEASE_MASK 
				);
		//da.set_events(  Gdk.EventMask.BUTTON1_MOTION_MASK );
				// Gdk.EventMask.POINTER_MOTION_MASK );
		da.expose_event += expose;
		da.motion_notify_event += motion;
		da.button_release_event += button_release;
		da.button_press_event += button_press;
		//da.key_press_event += key_press;
		//da.key_release_event += key_press;
		da.scroll_event += scroll_press;

		sw = new ScrolledWindow(
				new Adjustment(0, 10, 1000, 2, 100, 1000),
				new Adjustment(0, 10, 1000, 2, 100, 1000));
		//sw.set_policy(PolicyType.ALWAYS,PolicyType.ALWAYS);
		sw.set_policy(PolicyType.NEVER, PolicyType.NEVER);

		Viewport vp = new Viewport(
				new Adjustment(0, 10, 1000, 2, 100, 1000),
				new Adjustment(0, 10, 1000, 2, 100, 1000));
		vp.add(da);
		sw.add_events(  Gdk.EventMask.KEY_PRESS_MASK | Gdk.EventMask.KEY_RELEASE_MASK);
		sw.key_press_event += key_press;

		sw.add_with_viewport(vp);

		load_graph_at += (obj, addr) => {
//			stdout.printf("HOWHOWHOW "+addr);
		};
	}

	/* capture mouse motion */
	private bool scroll_press (Gtk.DrawingArea da, Gdk.EventScroll es)
	{
		switch(es.direction) {
			case ScrollDirection.UP:
				graph.zoom+=ZOOM_FACTOR;
				//graph.angle-=0.02;
				break;
			case ScrollDirection.DOWN:
				graph.zoom-=ZOOM_FACTOR;
				//graph.angle+=0.02;
				break;
		}

		da.queue_draw_area(0,0,5000,2000);
		return false;
	}

	private bool key_press (Gtk.Widget w, Gdk.EventKey ek)
	{
		bool handled = true;
		DrawingArea da = (DrawingArea)w;

		/* */
		//stdout.printf("Key pressed %d (%c)\n", ek.keyval,ek.keyval);

		switch (ek.keyval) {
			case 46: // dot. center view on selected node
				if (Graph.selected != null) {
					//sw.get_size(ref w, ref, h);
					graph.panx = -Graph.selected.x + 350;
					graph.pany = -Graph.selected.y + 350;
				}
				break;
			case 65056: // shift+tab : select previous
				//graph.select_prev();
				break;
			case 65289: // tab : select next node
				graph.select_next();
				if (graph.selected == null)
					graph.select_next();
				if (Graph.selected != null) {
					//sw.get_size(ref w, ref, h);
					// XXXX get window size
					graph.panx = -Graph.selected.x + 350;
					graph.pany = -Graph.selected.y + 350;
				}
				break;
			case 65361: // arrow left
			case 'h':
				graph.panx+=S*graph.zoom;
				break;
			case 65364: // arrow down
			case 'j':
				graph.pany-=S*graph.zoom;
				break;
			case 65362: // arrow up
			case 'k':
				graph.pany+=S*graph.zoom;
				break;
			case 65363: // arrow right
			case 'l':
				graph.panx-=S*graph.zoom;
				break;
			case 'u':
				graph.pany+=S*2;
				break;
			case ' ':
				graph.pany-=S*2;
				break;
			case 'H':
				graph.panx+=S*graph.zoom;
				if (Graph.selected != null)
					Graph.selected.x-=S*graph.zoom;
				break;
			case 'J':
				graph.pany-=S*graph.zoom;
				if (Graph.selected != null)
					Graph.selected.y+=S*graph.zoom;
				break;
			case 'K':
				graph.pany+=S*graph.zoom;
				if (Graph.selected != null)
					Graph.selected.y-=S*graph.zoom;
				break;
			case 'L':
				graph.panx-=S*graph.zoom;
				if (Graph.selected != null)
					Graph.selected.x+=S*graph.zoom;
				break;
			case '+':
				graph.zoom+=ZOOM_FACTOR;
				break;
			case '-':
				graph.zoom-=ZOOM_FACTOR;
				break;
			default:
				handled = false;
				break;
		}

		//expose(da, ev);
		da.queue_draw_area(0,0,5000,2000);

		return true;
	}

	public void do_popup_menu()
	{
		ImageMenuItem imi;
 		menu = new Menu();

		//imi = new ImageMenuItem.with_label("Focus");
		imi = new ImageMenuItem.from_stock("gtk-zoom-in", null);
		imi.activate += imi => {
	//		stdout.printf("go in!\n");
			Widget.focus_at_label(null, Graph.selected.get("label"));
			//MenuItem mi = menu.get_active();
			//load_graph_at(((Label)imi.child).get_text()); //"0x400");
			//stdout.printf(" cocococo "+ menu.);
		};
		menu.append(imi);

		imi = new ImageMenuItem.with_label("Breakpoint here");
		imi.activate += imi => {
	//		stdout.printf("add bp!\n");
			Widget.set_breakpoint(null, Graph.selected.get("label"));
		};
		menu.append(imi);

		imi = new ImageMenuItem.with_label("Remove breakpoint");
		imi.activate += imi => {
			Widget.unset_breakpoint(null, Graph.selected.get("label"));
		};
		menu.append(imi);

		// TODO: add continue until here
/*
		imi = new ImageMenuItem.with_label("Remove true branch");
		imi.activate += imi => {
///			stdout.printf("Focus!\n");
		};
		menu.append(imi);

		imi = new ImageMenuItem.with_label("Remove false branch");
		imi.activate += imi => {
	//		stdout.printf("Focus!\n");
		};
		menu.append(imi);
*/

		if (graph.selected != null) {
			menu.append(new SeparatorMenuItem());

			foreach(string str in graph.selected.calls) {
				imi = new ImageMenuItem.with_label(str);
				imi.activate += imi => {
					//stdout.printf("FUCKME: \n"+imi.submenu_placement());
					load_graph_at(((Label)imi.child).get_text()); //"0x400");
				};
				menu.append(imi);
			}
		}

		menu.show_all();
		//menu.popup(null, null, null, null, eb.button, 0);
		menu.popup(null, null, null, 0, 0);
	}

	private bool button_press (Gtk.DrawingArea da, Gdk.EventButton eb)
	{
		//EventButton eb = event.button;
		//EventMotion em = event.motion; 
		Node n = graph.click(eb.x-graph.panx, eb.y-graph.pany);

		graph.selected = n;
		if (n != null) {
			opanx = eb.x;
			opany = eb.y;
//			graph.draw(Gdk.cairo_create(da.window));
			da.queue_draw_area(0,0,5000,3000);
			
			if (eb.button == 3)
				do_popup_menu();
		}
		return true;
	}

	// drag nodes
	private double opanx = 0;
	private double opany = 0;

	private double offx = 0;
	private double offy = 0;

	private double abs(double x)
	{
		if (x>0)
			return x;
		return -x;
	}

	Node on = null;
	private bool button_release(Gtk.DrawingArea da, Gdk.EventButton em)
{
			on = null;
			opanx = opany = 0;
			return true;

}
	private bool motion (Gtk.DrawingArea da, Gdk.EventMotion em)
	{

		Node n = graph.selected; //graph.click(em.x-graph.panx, em.y-graph.pany);
		if (n != null) {
			/* drag node */
			/* TODO: properly handle the graph.zoom */
			if (n != on) {
				offx = (em.x - n.x);
				offy = (em.y - n.y);
				on = n;
			} 
				n.x = (em.x) - (offx);
				n.y = (em.y) - (offy);

				offx = (em.x - n.x);
				offy = (em.y - n.y);

				//offx += (offx/graph.zoom);
				//offy += (offy/graph.zoom);
//n.x += (offx-(offx*graph.zoom));
//n.y += (offy-(offy*graph.zoom));
//n.x*=graph.zoom;
//n.y*=graph.zoom;
			da.queue_draw_area(0,0,5000,3000);
			Graph.selected = n;
		} else {
			/* pan view */
			if ((opanx!=0) && (opany!=0)) {
				double x = em.x-opanx;
				double y = em.y-opany;
				graph.panx+=x*0.8;
				graph.pany+=y*0.8;
				//graph.draw(Gdk.cairo_create(da.window));
				da.queue_draw_area(0,0,5000,3000);
			}
			Graph.selected = null;
			opanx = em.x;
			opany = em.y;
		}
		return true;
	}

	private bool expose (Gtk.DrawingArea w, Gdk.EventExpose ev)
	{
		DrawingArea da = (DrawingArea)w;
		draw();
		return true;
	}

	public void draw()
	{
		Context ctx = Gdk.cairo_create(da.window);
		if (graph.zoom < 0.2)
			graph.zoom = 0.2;

		graph.draw(ctx);
	}

        [Import]
        [CCode (cname="core_load_graph_at_label")]
        public static void focus_at_label(void *obj, string addr);

        [Import]
        [CCode (cname="mygrava_bp_at")]
        public static void set_breakpoint(void *obj, string addr);

        [Import]
        [CCode (cname="mygrava_bp_rm_at")]
        public static void unset_breakpoint(void *obj, string addr);
}

		/* Usage example */
		/* nodes
		Node n = new Node();
		n.set("label", "0x8048490  _start:");
		n.set("color", "gray");
		n.set("body",
		"0x08048490  mov eax, 0x3842\n"+
		"0x08048495  xor ebx, ebx\n"+
		"0x08048497  jz 0x08048900");
		graph.add_node(n);

		Node n2 = new Node();
		n2.set("label", "0x08048900  _sub_start:");
		n2.set("color","gray");
		n2.set("body",
		"0x08048900  jmp 0x804890");
		graph.add_node(n2);

		Node n3 = new Node();
		n3.set("label", "0x0804849b  --");
		n3.set("color","gray");
		n3.set("body",
		"0x0804849b  rdtsc\n"+
		"0x0804849c  nop\n"+
		"            ...\n");
		graph.add_node(n3);

		// edges
		Edge e = new Edge().with(n, n2);
		e.set("color", "green");
		graph.add_edge(e);

		e = new Edge().with(n, n3);
		e.set("color", "red");
		graph.add_edge(e);

		e = new Edge().with(n2, n);
		e.set("color", "blue");
		graph.add_edge(e);
		 */
