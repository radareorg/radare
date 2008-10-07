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

public class Grava.Edge : GLib.Object
{
	public HashTable<string,string> data;
	public Node orig;
	public Node dest;
	public bool visible;
	public bool jmpcnd; // verd == true , vermell == false

	construct {
		data = new HashTable<string,string>.full(str_hash, str_equal, g_free, Object.unref);
		orig = dest = null;
		visible = true;
	}

	public string get(string val)
	{
		return data.lookup(val);
	}

	public void set(string val, string key)
	{
		data.insert(val, key);
	}

	public Edge with(Node a, Node b)
	{
		orig = a;
		dest = b;
		return this;
	}

	/* workaround for netbsd's libm 
	private double exp2(double x)
	{
		return Math.exp(x * Math.log(2));
	}

	public double distance()
	{
		return Math.sqrt( this.exp2(orig.x-dest.x) + this.exp2(orig.y-dest.y));
	}
*/
	public double distance()
	{
		return Math.sqrt( Math.pow(orig.x-dest.x,2 ) + Math.pow(orig.y-dest.y, 2) );
		//return Math.sqrt( Math.exp2(orig.x-dest.x) + Math.exp2(orig.y-dest.y) );
	}
}
