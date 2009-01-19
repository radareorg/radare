/* This vapi has been manually generated by me */
[CCode (cheader_filename="r_socket.h", cprefix="r_socket", lower_case_cprefix="r_socket_")]
namespace Radare.Socket {
	public int ready(int fd, int secs, int usecs);
	[NoArrayLength]
	public int read(int fd, out uint8[] buf, int len);
	[NoArrayLength]
	public int write(int fd, ref uint8[] buf, int len);
	public int connect(string host, int port);
	public int listen(int port);
	public int close(int fd);
	[NoArrayLength]
	public int fgets(int fd, out string buf, int len);
	public int printf(int fd, string str, ...);
	public int accept(int fd);
}
