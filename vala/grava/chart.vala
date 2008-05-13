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

/*
Charting api for Vala
=====================
 - integrated with grava
 - support for colors
 - support for:
   - bars
   - 2D dot space
   - 2D lines
 Chart class
   setColor("#ff0000");
   setLimits(0,5000);
   setBar("code", 100);
   setColumn("food", 30);
   setPoint(x, y, "#ff0000")
*/

class Grava.Chart : GLib.Object
{
	//public SList<Bar> bars;

	/* constructor */
	construct {
//		bars = new SList<Bar>();
	}

	public void setColor(string color)
	{
	}

	public void setLimits(int from, int to)
	{
	}

	public void setBar(string name, int val)
	{
	}

	public void setColumn(string name, int val)
	{
	}

	public void setPoint(int x, int y, string color = color)
	{
	}

	public void draw()
	{
	}
}
