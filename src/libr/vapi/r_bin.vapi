/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

[CCode (cheader_filename="r_bin.h", cprefix="r_bin", lower_case_cprefix="r_bin_")]
namespace Radare {
	[Compact]
	[CCode (cname="r_bin_obj", free_function="r_bin_free", cprefix="r_bin_")]
	public class Bin {
		public const string file;
		public int fd;

		public Bin(string file, int rw);

		public int init(string file, int rw);
		public int close();
		public uint64 get_baddr();
		public Entry[] get_entry();
		public Section[] get_sections();
		public Symbol[] get_symbols();
		public Import[] get_imports();
		public Info[] get_info();
		public uint64 get_section_offset(string name);
		public uint64 get_section_rva(string name);
		public uint32 get_section_size(string name);
		public uint64 resize_section(string name, uint64 size);
	}
	
	public struct Entry {
		uint64 rva;
		uint64 offset;
	}

	public struct Section{
		string name;
		uint32 size;
		uint32 vsize;
		uint64 rva;
		uint64 offset;
		uint32 stringacteristics;
		int last;
	}

	public struct Symbol {
		string name;
		string forwarder;
		string bind;
		string type;
		uint64 rva;
		uint64 offset;
		uint32 size;
		uint32 ordinal;
		int last;
	}

	public struct Import {
		string name;
		string bind;
		string type;
		uint64 rva;
		uint64 offset;
		uint32 ordinal;
		uint32 hint;
		int last;
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
		uint32 dbg_info;
	}
}
