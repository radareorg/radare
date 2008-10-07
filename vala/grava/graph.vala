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

public class Grava.Graph : GLib.Object
{
	public Layout layout;
	public SList<Node> selhist;
	public SList<Node> nodes;
	public SList<Edge> edges;
	public HashTable<string,string> data;
	public static weak Node selected = null;
	public double zoom  = 1;
	public double panx  = 0;
	public double pany  = 0;
	public double angle = 0;
	//ImageSurface s;

	/* constructor */
	construct {
		nodes  = new SList<Node>();
		edges  = new SList<Edge>();
		selhist= new SList<int>();
		layout = new DefaultLayout();
		data   = new HashTable<string, string>.full(str_hash, str_equal, g_free, Object.unref);
	//	s = new ImageSurface.from_png("/tmp/file.png");
	}

	public void undo_select()
	{
		uint length = selhist.length();
		selected = selhist.nth_data(length-1);
		selhist.remove(selected);
	}

	/* TODO: Add boolean argument to reset layout too */
	public void reset() {
		layout.reset();
		nodes  = new SList<Node>();
		edges  = new SList<Edge>();
		selhist= new SList<int>();
		//layout = new DefaultLayout();
		/* add png here */
	}

	public void set(string key, string val)
	{
		data.insert(key, val);
	}

	public string get(string key)
	{
		return data.lookup(key);
	}

	public void select_next()
	{
/*
		foreach(weak Node node in nodes) {
			if (Graph.selected == null) {
				selected = node;
				Graph.selected = node;
				selhist.append(selected);
				break;
			}
			if (selected == node) {
				Graph.selected = node;
				selected = null;
			}
		}
*/
		//if (Graph.selected == null || selected == null)
			foreach(weak Node node in nodes) {
				Graph.selected = selected = node;
				break;
			}
	}

	public void select_true()
	{
		if (selected == null)
			return;

		foreach(weak Edge edge in edges) {
			if (selected == edge.orig) {
				if (edge.get("color") == "green") {
					selected = edge.dest;
					selhist.append(selected);
					break;
				}
			}
		}
	}

	public void select_false()
	{
		if (selected == null)
			return;

		foreach(weak Edge edge in edges) {
			if (selected == edge.orig) {
				if (edge.get("color") == "red") {
					selected = edge.dest;
					selhist.append(selected);
					break;
				}
			}
		}
	}

	public static bool is_selected(Node n)
	{
		return n == selected;
	}

	public void update()
	{
		layout.run(this);
	}

	// esteve modificat perque insereixi a la llista ordenat per baseaddr.
	// volia fer servir node.sort, pero no m'ha sortit...
	public void add_node (Node n)
	{
		int count;
		Node p;
		bool ins;
		uint len = nodes.length();

		layout.set_graph(this);
		n.fit();
		
		ins = false;
		for ( count = 0 ; count <  len ; count ++ ) {
			p = nodes.nth_data ( count );
			//stdout.printf ("adding node base at %x , this is %x\n", p.baseaddr, n.baseaddr );

			if ( p.baseaddr >= n.baseaddr ) {
				ins=true;
				nodes.insert(n, count);
				break;
			}
		}

		if (  ins == false )
			nodes.append(n);
	}

	public void add_edge (Edge e)
	{
		edges.append(e);
	}

	public SList<Node> unlinked_nodes ()
	{
		SList<Node> ret = new SList<Node>();
		foreach(weak Node node in nodes) {
			bool found = false;
			foreach(weak Edge edge in edges ) {
				if (node == edge.orig || node == edge.dest) {
					found = true;
					break;
				}
			}
			if (!found)
				ret.append(node);
		}
		return ret;
	}

	public SList<Node> outer_nodes (Node n)
	{
		SList<Node> ret = new SList<Node>();
		foreach(weak Edge edge in edges) {
			if (edge.visible && (edge.orig == n))
				ret.append(edge.dest);
		}
		return ret;
	}

	public SList<Node> inner_nodes (Node n)
	{
		SList<Node> ret = new SList<Node>();
		foreach(weak Edge edge in edges ) {
			if (edge.visible && (edge.dest == n))
				ret.append(edge.orig);
		}
		return ret;
	}

	public weak Node? click (double x, double y)
	{
		double z = zoom;
		foreach(weak Node node in nodes) {
			if (x>= node.x*z && x <= node.x*z+node.w*z
			&& y >= node.y*z && y <= node.y*z+node.h*z)
				return node;
		}
		return null;
	}

	public bool overlaps(Node n)
	{
		foreach(weak Node node in nodes) {
			if (node != n && node.overlaps(n))
				return true;
		}
		return false;
	}

	public void draw(Context ctx)
	{
//		ctx.set_operator (Cairo.Operator.SOURCE);
		// XXX THIS FLICKERS! MUST USE DOUBLE BUFFER
		if (ctx == null)
			return;

		ctx.set_source_rgba(1, 1, 1, 1);
		ctx.translate( panx, pany);
		ctx.scale( zoom, zoom );
		ctx.rotate( angle );
	
		/* blank screen */
		ctx.paint();

		/* draw bg picture
		ctx.save();
		ctx.set_source_surface(s, panx, pany);
		ctx.paint();
		ctx.restore ();
		*/

		foreach(weak Edge edge in edges ) {
			if (edge.visible)
				Renderer.draw_edge(ctx, edge);
		}
		foreach(weak Node node in nodes) {
			if (node.visible)
				Renderer.draw_node(ctx, node);
		}

		// TODO: double buffering
	}

	public void add(Node n)
	{
		nodes.append(n);
	}

	public void link(Node n, Node n2)
	{	
		edges.append( new Edge().with(n, n2) );
	}
}
