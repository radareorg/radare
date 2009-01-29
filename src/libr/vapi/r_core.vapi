/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

[CCode (cheader_filename="r_core.h", cprefix="r_core", lower_case_cprefix="r_core_")]
namespace Radare.Core {
	
	[CCode (cname="struct r_core_file_t", cprefix="r_core_")]
	public struct File {
		public string uri;
		public string filename;
		public uint64 seek;
		public uint64 size;
		public int rwx;
		public int fd;
	}

	[Compact]
	[CCode (cname="struct r_core_t", free_function="r_core_free", cprefix="r_core_")]
	public class State {
		/* lifecycle */
		public State();

		/* commands */
		public int prompt();
		public int cmd(string cmd, bool log);
		public int cmd0(string cmd);
		public int cmd_file(string file);
		public unowned string cmd_str(string cmd);

		/* io */
		public int read_at(uint64 addr, out uint8 *buf, int size);
		public int write_at(uint64 addr, uint8 *buf, int size);
		public int block_read(bool next);
		public int seek(uint64 addr);

		/* files */
		// XXX mode = Radare.Io.Mode
		public Core.File file_open(string file, int mode);
		public bool file_set(string file, Core.File file);
		public bool file_close(string file, Core.File file);
		public bool file_close_fd(string file, int fd);
	}
}
