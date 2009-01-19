using Radare;

public class SocketExample
{
	public static void main(string[] args)
	{
		string str = new StringBuilder.sized(1024).str;
		int fd = Socket.connect("www.google.com", 80);

		Socket.printf(fd, "GET /\r\n\r\n");
		while(Socket.fgets(fd, out str, 1024) >=0 ) {
			stdout.printf(str);
		}
		Socket.close(fd);
	}
}
