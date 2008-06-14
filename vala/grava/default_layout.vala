/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007, 2008   pancake <youterm.com>
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

public class Grava.DefaultLayout : Grava.Layout
{
	public double y_offset = 100;
	public double x_offset = 50;
	private weak Graph graph;
	private static bool d = false; // debug output

	private void treenodes(Node n)
	{
		int ox = 0;
		SList<Node> nodes = graph.outer_nodes(n);
		foreach(weak Node node in nodes) {
			node.y = n.y+n.h+y_offset;
			node.x += n.w+ ox;
			ox+=50;
			treenodes(node);
		}
	}

	public void walkChild(Node node, int level)
	{
		if (level<1) return ;
		foreach(weak Edge edge in graph.edges) {
			if (edge.orig == node) {
				edge.dest.y = edge.orig.y + edge.orig.h + y_offset;
				walkChild(edge.dest, --level);
			}
		}
	}

	public weak Node? get_parent(Node node)
	{
		foreach(weak Edge edge in graph.edges) {
			if (edge.dest == node)
				return edge.orig;
		}
		return null;
	}

	public override void run(Graph graph)
	{
		this.graph = graph;
		double last_y;
		SList<Node> paint_nodes;
		int i, inorig, indst ,k;
		Node n,p, destn;
		Edge e;
		bool found;

		// reset all node positions

		last_y = 50;

		// Tots vertical, un sota l'altr ordenats per base addr
		foreach(weak Node node in graph.nodes) {
			if (!node.visible) continue;
			// reset node positions
			node.x = 50;
			node.fit();
			node.y = last_y ;
			last_y = node.y + node.h + 50 ;
			if (d) stdout.printf(" at %f %s %x\n", node.y, node.get ("label" ) , node.baseaddr );
		}
		
		// Per cada node. Segueixo la condiciÃ³ certa, tots els nodes que estiguin
		// entre el node que estic mirant i el de la condicio certa els desplaÃ§o a la dreta.
		// Tambe  apropo  el node desti a l'origen.
		//
		// Entre el node que miro i el desti vol dir que la x sigui la mateixa.

		for( i = 0 ; i < graph.nodes.length() ; i ++ ) {
			n = graph.nodes.nth_data ( i );

			/// busco l'edge verd d'aquest node
			found = false;
			foreach(weak Edge edge in graph.edges) {
				if (edge.orig == n ) {
					if (edge.jmpcnd == true )
					{
						if(d)stdout.printf ( "0x%x ----> 0x%x\n",edge.orig.baseaddr ,edge.dest.baseaddr );
						destn = edge.dest;
						found = true;
						break;
					}
				}
			}

			/// n es el node origen.
			/// destn es el node desti
			///
			last_y = n.y + n.h + 10 ;

			// Si la base del node origen es < que le desti .
			// sempre anem avall.
			//
			if ( (found == true ) && ( n.baseaddr < destn.baseaddr ) ) {
				/// Busco el node mes ample.
				///
				double maxw = 0;
				for( k = (i+1) ;  k < graph.nodes.length() ; k ++ ) {
					p = graph.nodes.nth_data ( k );
					if ( (p.x == n.x) && ( p.w > maxw ) )
						maxw = p.w;
				}
				/// DesplaÃ§o
				for( k = (i+1) ;  k < graph.nodes.length() ; k ++ ) {
					p = graph.nodes.nth_data ( k );
					
					if(d)stdout.printf ( "Displacing 0x%x\n", p.baseaddr );
				
					// El node estava entre el node origen i el desti
					if ( p.x == n.x )
						p.x += ( maxw + 10 );
				
					/// Es ja el node desti.
					if ( p == destn ) {
						if(d)stdout.printf ( "AT 0x%x\n", p.baseaddr );
						destn.x = n.x; 
						destn.y = n.y + n.h + 50;
						break;
					}
				}
			}
		}

		return;

/*
		walkChild(graph.selected, 5);
		//walkChild(graph.selected); //
		foreach(weak Node node in graph.nodes) {
			walkChild(node, 5);
		}

		foreach(weak Node node in graph.nodes) {
			if (!node.visible) continue;

			weak Node parent = get_parent(node);
			if (parent != null) {
				node.x = parent.x;
			}
		}

		bool overlaps = false;
		SList<Node> nodz;
		foreach(weak Node n in graph.nodes) {
			double totalx = 0;
			int total;
			nodz= new SList<Node>();
			nodz.append(n);
			foreach(weak Node n2 in graph.nodes) {
				if (n != n2 && n.overlaps(n2)) {
					totalx+=n2.w+x_offset;
					n.x += n.w + n2.w + x_offset;
					nodz.append(n2);
				}
			}
			totalx/=2;
			foreach(weak Node n2 in nodz) {
				if (n != n2 && n.overlaps(n2)) {
					totalx+=n2.w;
					n.x -= (total);
				}
			}
			nodz = null;
		}

		foreach(weak Node node in graph.nodes) {
			if (!node.visible) continue;

			if (graph.overlaps(node))
				node.x += node.w;
		}
		foreach(weak Node node in graph.nodes) {
			if (!node.visible) continue;
			foreach(weak Edge edge in graph.edges) {
				if (edge.orig == node ) {
					if (edge.dest.y < node.y)
						node.y = edge.dest.y-(node.h-y_offset)*3;
				}
			}
		}
primer incremento les x mentres vaig guardant el totalx
guardo en una llista enlla,cada tots els nodos q he mogut
els recorro again i els delpla,co a lesqerra lo que toquin
*/
	}
}

