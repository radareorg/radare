/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

[CCode (cheader_filename="r_asm.h", cprefix="r_asm", lower_case_cprefix="r_asm_")]
namespace Radare {
	[Compact]
	[CCode (cname="struct r_asm_t", free_function="r_asm_free", cprefix="r_asm_")]
	public class Asm {
		public enum Arch {
			NULL  = 0,
		  	X86   = 1,
		  	ARM   = 2,
		  	PPC   = 3,
		  	M68K  = 4,
		  	JAVA  = 5,
		  	MIPS  = 6,
		  	SPARC = 7,
		  	CSR   = 8,
		  	MSIL  = 9,
		  	OBJD  = 10,
		  	BF    = 11
		}
		public enum Syn {
			NULL  = 0,
			INTEL = 1,
			ATT   = 2,
			OLLY  = 3
		}
		public enum Par {
			NULL    = 0,
			PSEUDO  = 1,
			REALLOC = 2
		}

		public uint32 arch;
		public uint32 bits;
		public uint32 big_endian;
		public uint32 syntax;
		public uint32 parser;
		public uint64 pc;
		public string buf_asm;
		public string buf_hex;
		public string buf_err;
		public void *aux;

		public Asm();

		public int init();
		public int set_arch(Arch arch);
		public int set_bits(int bits);
		public int set_syntax(Syn syntax);
		public int set_pc(uint64 addr);
		public int set_big_endian(bool big);
		public int set_parser(Par parser, parse_cb cb, void *aux);
		public uint32 disasm(uint8 *buf, int length);
		public uint32 asm(string buf);
		public uint32 parse();
	}

	public static delegate uint32 parse_cb(Asm a);
}
