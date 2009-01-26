/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

[CCode (cheader_filename="r_asm.h", cprefix="r_asm", lower_case_cprefix="r_asm_")]
namespace Radare.Asm {
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
		BF    = 11,
	}
	public enum Syntax {
		SYN_NULL   = 0,
		SYN_INTEL  = 1,
		SYN_ATT    = 2,
		SYN_OLLY   = 3,
		SYN_PSEUDO = 4
	}
	[Compact]
	[CCode (cname="struct r_asm_t", free_function="r_asm_free", cprefix="r_asm_")]
	public class State {
		public unowned string buf_asm;
		public unowned string buf_hex;
		public State();
		public bool set_arch(Arch arch);
		public bool set_bits(int bits);
		public bool set_syntax(Syntax syntax);
		public bool set_pc(uint64 addr);
		public bool set_big_endian(bool big);
		public bool disasm(uint8 *buf, int length);
	}
}
