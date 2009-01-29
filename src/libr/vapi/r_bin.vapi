/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

[CCode (cheader_filename="r_bin.h", cprefix="r_bin", lower_case_cprefix="r_bin_")]
namespace Radare.Bin {
	
	[Compact]
	[CCode (cname="struct r_bin_t", free_function="r_bin_free", cprefix="r_bin_")]
	public class State {
		/* XXX: lifecycle is broken*/
		public State();
		public int init();
		public int open(string file, int rw);
		public int close();
		public Bin.Entry[] get_entry();
		public Bin.Section[] get_sections();
		public Bin.Symbol[] get_symbols();
		public Bin.Import[] get_imports();
		public Bin.Info[] get_info();
		public uint64 get_section_offset();
		public uint64 get_section_rva();
		public uint32 get_section_size();
		public uint64 resize_section(string name, uint64 size);
	}
	public struct Info {
		string type;
		string @class;
		string rclass;
		string arch;
		string machine;
		string os;
		string subsystem;
		int bigendian;
		u32 dbg_info;
	}
}
