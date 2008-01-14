using Gtk;

public class Line : HBox
{
	Entry e;
	Label l;

	construct {
		e = new Entry();
		l = new Label("");
		pack_start(l, false, false, 5);
		add(e);
	}

	public Line name(string name, string val)
	{
		l.set_text(name);
		e.set_text(val);
		e.editable = false;
		return this;
	}

	public Line numeric(string name, int number)
	{
		l.set_text(name);
		e.editable = false;
		e.set_text("%d".printf(number));
		return this;
	}

/*
	public Line combo(string name, string[] items)
	{
		l.set_text(name);
		e.editable = false;
		e.set_text("%d".printf(number));
		return this;
	}
*/
}
