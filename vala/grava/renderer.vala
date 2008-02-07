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

using Cairo;
using Gtk;
using GLib;

public class Grava.Renderer
{
	public static void draw_edge(weak Context ctx, weak Edge edge)
	{
		double dx = 0;
		double dy = 0;

		ctx.save();
		set_color(ctx, edge.data);

		ctx.set_line_width (3);
		if (edge.orig.y+edge.orig.h<(edge.dest.y)) {//-edge.dest.h)) {
			/* up to bottom */
			ctx.translate (edge.orig.x+(edge.orig.w/2),edge.orig.y+edge.orig.h);
			  dx = edge.dest.x-edge.orig.x-(edge.orig.w/2) + edge.dest.w/2; //-edge.orig.x;
			  dy = edge.dest.y-edge.orig.y - edge.orig.h;
			ctx.move_to(30,30);
			line(ctx, 0,0, dx, dy);
		} else {
			/* bottom to up */
			ctx.translate (edge.orig.x+(edge.orig.w/2),edge.orig.y+edge.orig.h);
			ctx.move_to(0, 0);
			  //dx = edge.dest.x-edge.orig.x;
			  dx = edge.dest.x-edge.orig.x-(edge.orig.w/2) + edge.dest.w/2; //-edge.orig.x;
			  dy = edge.dest.y-edge.orig.y- edge.orig.h; // or 80 or so depending if > or < ???
			double ox = dx;
			if (ox == 0){ ox = 150; }
			ctx.curve_to(0,100, ox, 120, dx, dy);
			ctx.stroke();
		}

		ctx.set_source_rgb (0.0, 0.0, 0.0);
		ctx.rectangle(dx, dy, 2, 2);
		ctx.stroke();
		ctx.restore();
		ctx.set_source_rgb (0.6, 0.6, 0.6);
	}

	public static void set_color(weak Context ctx, HashTable<string,string> ht)
	{
		weak string color = ht.lookup("color");
		set_color_str(ctx, color);
	}

	public static bool set_color_str(weak Context ctx, string color)
	{
		if (color != null) {
			if (color == "black")
				ctx.set_source_rgba (0.0, 0.0, 0.0, 0.7);
			else
			if (color == "white")
				ctx.set_source_rgba (1.0, 1.0, 1.0, 0.4);
			else
			if (color == "green")
				ctx.set_source_rgba (0.1, 0.7, 0.1, 0.8);
			else
			if (color == "red")
				ctx.set_source_rgba (1.0, 0.1, 0.1, 0.9);
			else
			if (color == "turqoise")
				ctx.set_source_rgba (0.6, 0.9, 1.0, 0.8);
			else
			if (color == "blue")
				ctx.set_source_rgba (0.2, 0.5, 0.9, 0.8);
			else
			if (color == "yellow")
				ctx.set_source_rgba (0.7, 0.7, 0.0, 0.8);
			else
				ctx.set_source_rgba (1.0, 1.0, 1.0, 0.4);
		} else
			ctx.set_source_rgba (1.0, 1.0, 1.0, 0.4);

		return (color!=null);
	}

	public static void draw_node(weak Context ctx, weak Node node)
	{
		ctx.save ();

		ctx.set_tolerance (0.1);
		ctx.set_line_join (LineJoin.ROUND);
		ctx.set_line_width (1);
		ctx.translate (node.x, node.y);

//		ctx.set_source_rgb (1, 1, 1);
		ctx.set_source_rgb (0.8, 0.8, 0.8);
/*#if 0
		if (node.calls.length() >0) 
			set_color(ctx, "red");
		else
		set_color(ctx, "blue");
#endif
*/
		set_color(ctx, node.data);
		set_color_str(ctx, node.data.lookup("bgcolor"));
		square (ctx, node.w, node.h);
		ctx.fill();

		/* title rectangle */
		if (node.data.lookup("color")!=null)
			set_color_str(ctx, node.data.lookup("color"));
		else
		if (node.calls.length() ==1) 
			ctx.set_source_rgba (0.2, 0.2, 0.4, 0.7);
		else
		if (node.calls.length() >0) 
			ctx.set_source_rgba (0.3, 0.3, 1, 0.7);
		else
			ctx.set_source_rgba (0.8,0.8,0.8, 0.7);
		square (ctx, node.w, 15);
		ctx.fill ();
		line(ctx, 0, 15, node.w, 0);

		ctx.select_font_face("Courier", 
			FontSlant.NORMAL,
			FontWeight.BOLD);
		ctx.set_font_size(10);
		
		/* set label */
		ctx.set_source_rgb (0.1, 0.1, 0.1);
		ctx.move_to(5,10);
		ctx.show_text(node.data.lookup("label"));

		/* set body */
		int y = 25;
		string body = node.data.lookup("body");
		if (body != null)
		foreach(string str in body.split("\n")) {
			ctx.move_to(5, y+=10);
			if((str.str("call ") != null)
			|| (str.str("bl ") != null)) {
				set_color_str(ctx, "blue");
			} else
			if (str.str("goto") != null) {
				set_color_str(ctx, "green");
			} else
			if (str.str(" j") != null) {
				set_color_str(ctx, "green");
			} else
			if (str.has_suffix(":")) {
				set_color_str(ctx, "red");
			} else
				set_color_str(ctx, "black");
			ctx.show_text(str);
		}
	
		//set_color(ctx, node.data);
		/* box square */
		if (Graph.selected == node) {
			ctx.set_source_rgba (1, 0.8, 0.0, 0.9);
			ctx.set_line_width (6);
		} else {
			ctx.set_source_rgba (0.2, 0.2, 0.2, 0.4);
			ctx.set_line_width (1);
		}
		square (ctx, node.w, node.h);
		ctx.stroke();

		ctx.restore();
	}

	public static void square (Context ctx, double w, double h) {
		ctx.move_to (0, 0);
		ctx.rel_line_to (w, 0);
		ctx.rel_line_to (0, h);
		ctx.rel_line_to (-w, 0);
		ctx.close_path ();
	}

	public static void line (Context ctx, double x, double y, double w, double h) {
		ctx.move_to (x,y);
		ctx.rel_line_to (w, h);
		ctx.stroke();
	}
}
