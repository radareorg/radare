/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

using Radare;

public class AsmExample
{
	public static void main(string[] args)
	{
		Asm.State st = new Asm.State();
		st.set_arch( Asm.Arch.X86 );
		st.set_bits( 32 );
		st.set_bigendian( false );
		st.set_pc( 0x8048000 );

		uint8 *buf = "\xcd\x21";
		st.disasm_buf(buf, 2);
		stdout.printf("%s", st.buf_asm);
	}
}
