using GLib;

public class Radare.IO
{
	public static int open(string file, int flags, int mode)
	{
		return io_open(file, flags, mode);
	}

	public static int close(int fd)
	{
		return io_close(fd);
	}

	public static int read(int fd, out string buf, long size)
	{
		return io_read(fd, out buf, size);
	}

	public static int write(int fd, string buf, long size)
	{
		return io_write(fd, buf, size);
	}

	public static int system(string command)
	{
		return io_system(command);
	}
}

[Import()]
public static int io_open(string file, int flags, int mode);

[Import()]
public static int io_close(int fd);

[Import()]
public static int io_read(int fd, out string buf, long size);

[Import()]
public static int io_write(int fd, out string buf, long size);

[Import()]
public static int io_system(string command);
