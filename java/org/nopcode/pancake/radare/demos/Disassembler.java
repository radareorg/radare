/**
 *
 * This example disassembles the entry point of an ELF file
 *
 */

package org.nopcode.pancake.radare.demos;

import org.nopcode.pancake.radare.Radare;
import org.nopcode.pancake.radare.data.*;
import org.nopcode.pancake.radare.asm.*;

public class Disassembler
{
  public static void main(String[] args)
  {
	Radare r = null;

	try {
		r = new Radare(args[0]);
	} catch(Exception e) {
		System.out.println("Usage: java Disassembler [file]");
		System.exit(1);
	}

	r.seekTo(0x100);
	r.setBlockSize(0x21);

/*
	HexDumper.println(r.getBytes());

	System.out.println("Disassemble:");
	DisassembleDumper.println(r.getBytes());

	System.out.println("Opcodes:");
*/
	Opcode[] ops = DisassembleDumper.getOpcodes(r.getBytes());
/*
	for(Opcode o : ops) {
		System.out.println(" "+o.getSize()+": "+o.getOffset()+" - "+o.toString());
	}
*/

	/* CPU Emulation */
	CPU cpu = new org.nopcode.pancake.radare.asm.cpu.x86();
	System.out.println(cpu.xmlHeader());
	for(Opcode o : ops) {
		//System.out.println(" <!-- "+o.toString()+" -->");
		System.out.println(" "+cpu.toXMLString(o));
		cpu.stepi(o);
	}
	System.out.println(cpu.xmlFooter());
  }
}
