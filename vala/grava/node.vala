/*
 *  Grava - General purpose graphing library for Vala
 *  Copyright (C) 2007,2008  pancake <youterm.com>
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

public class Grava.Node : GLib.Object
{
	public HashTable<string,string> data;
	public uint baseaddr;
	public SList<string> calls;
	public SList<string> xrefs;
	public bool visible;
	public bool selected;
	public bool has_body;
	public int shape;
	public double x;
	public double y;
	public double w;
	public double h;

	construct {
		data     = new HashTable<string,string>.full(str_hash, str_equal, g_free, Object.unref);
		calls    = new SList<string>();
		xrefs    = new SList<string>();
		baseaddr = 0;
		x        = y = 0;
		w        = 150;
		h        = 200;
		shape    = Shape.RECTANGLE;
		visible  = true;
		has_body = true;
		selected = false;
	}

	public void set (string key, string val)
	{
		data.insert (key, val);
	}

	public void set_i (string key, uint64 val)
	{
		string str = "0x%llx".printf(val);
		data.insert (key, str);
	}

	public string get (string key)
	{
		return data.lookup(key);
	}

	public void add_call(uint64 addr)
	{
		string str = "0x%08llx".printf(addr);
		calls.append(str);
	}

	public void add_xref(uint64 addr)
	{
		string str = "0x%08llx".printf(addr);
		xrefs.append(str);
	}

	public bool overlaps(Node n)
	{
		return (n.x >= x && n.x <= x+w && n.y <= y && n.y <= y+h);
	}

	public void fit()
	{
		string label = data.lookup("label");
		string body  = data.lookup("body");
		double _y = 25;
		double _w = 0;

		if (label != null)
			_w = label.len()+2;

		if (has_body && body != null )
		foreach( string str in body.split("\n") ) {
			_y += 10;
			if ( str.len() > (long)_w )
				_w = (double)str.len();
		}

		w = (_w*7);
		h = (_y)+10;
	}
}
