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
 */

using GLib;
using Cairo;

public class Grava.Graph : GLib.Object
{
	public Layout layout;
	public SList<Node> nodes;
	public SList<Edge> edges;
	public static weak Node selected = null;
	public double zoom  = 1;
	public double panx  = 0;
	public double pany  = 0;
	public double angle = 0;

	/* constructor */
	construct {
		nodes  = new SList<Node>();
		edges  = new SList<Edge>();
		layout = new DefaultLayout();
	}

	public void reset() {
		nodes  = new SList<Node>();
		edges  = new SList<Edge>();
		layout = new DefaultLayout();
	}

	public void select_next()
	{
		foreach(weak Node node in nodes) {
			if (selected == null) {
				selected = node;
				break;
			}
			if (selected == node)
				selected = null;
		}
	}

	public static bool is_selected(weak Node n)
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
		n.fit();
		
		//stdout.printf ( "ADD NODE %d, addr: 0x%x\n", nodes.length() , n.baseaddr );

		ins = false;
		for ( count = 0 ; count <  len ; count ++ )  
		{
			p = nodes.nth_data ( count );
			//stdout.printf ("adding node base at %x , this is %x\n", p.baseaddr, n.baseaddr );

			/// el node que estic mirant ja te la base address mes gran que jo. Inserto a l'anterior.
			if ( p.baseaddr >= n.baseaddr )
			{
				ins=true;
				nodes.insert ( n , count   );
				break;
			}
		}
		if (  ins == false )
		{
			nodes.append(n);
		}
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

	public SList<Node> outer_nodes (weak Node n)
	{
		SList<Node> ret = new SList<Node>();
		foreach(weak Edge edge in edges) {
			if (edge.visible && (edge.orig == n))
				ret.append(edge.dest);
		}
		return ret;
	}

	public SList<Node> inner_nodes (weak Node n)
	{
		SList<Node> ret = new SList<Node>();
		foreach(weak Edge edge in edges ) {
			if (edge.visible && (edge.dest == n))
				ret.append(edge.orig);
		}
		return ret;
	}

	public weak Node click (double x, double y)
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

	public void draw(weak Context ctx)
	{
//		ctx.set_operator (Cairo.Operator.SOURCE);
	// XXX THIS FLICKERS! MUST USE DOUBLE BUFFER
		if (ctx == null)
			return;
		ctx.set_source_rgba(1, 1, 1, 1);
		ctx.translate( panx, pany);
		ctx.scale( zoom, zoom );
		ctx.rotate( angle );
	
		ctx.paint ();
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
