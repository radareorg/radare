/*
 * Copyright (C) 2006
 *       pancake <pancake@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

package org.nopcode.pancake.radare.asm.cpu;

import org.nopcode.pancake.radare.asm.*;

public class x86 extends Core implements CPU
{
  int eax, ebx, ecx, edx;
  int esp, ebp, esi, edi;
  int eip;

  public x86()
  {
	super();
	this.setRegister("eip", RegisterType.COUNTER);
	this.setRegister("esp", RegisterType.STACK);
	this.setRegister("eax", RegisterType.INT32);
	this.setRegister("ebx", RegisterType.INT32);
	this.setRegister("ecx", RegisterType.INT32);
	this.setRegister("edx", RegisterType.INT32);
	this.setRegister("ebp", RegisterType.INT32);
	this.setRegister("esi", RegisterType.INT32);
	this.setRegister("edi", RegisterType.INT32);
  }

  public void reset()
  {
	eax = ebx = ecx = edx = 0;
	esp = ebp = esi = edi = 0;
  }

  public String xmlHeader()
  {
	return "<?xml version=\"1.0\" ?>\n<cpu architecture=\"x86\" />";
  }

  public String xmlFooter()
  {
	return "</cpu>";
  }

  public void stepi(Opcode o)
  {
  }

  public String toXMLString(Opcode o)
  {
	String[] arg;

	switch( this.getType(o) ) {
	case XOR:
		arg = o.getArgument().split(",");
		return  "<xor source=\""+Opcode.cleanup(arg[0])+
			"\" source=\""+Opcode.cleanup(arg[0])+
			"\" destination=\""+
			Opcode.cleanup(arg[1])+"\" />";
	case CALL:
		return "<call offset=\""+Opcode.cleanup(o.getArgument())+"\">";
	case RETURN:
		return "<return />";
	case NOP:
		return "<nop />";
	case PUSH:
		return "<push source=\""+Opcode.cleanup(o.getArgument())+"\" />";
	case POP:
		return "<pop destination=\""+Opcode.cleanup(o.getArgument())+"\" />";
	case AND:
		arg = o.getArgument().split(",");
		return  "<and source=\""+Opcode.cleanup(arg[0])+
			"\" source=\""+Opcode.cleanup(arg[1])+
			"\" destination=\""+Opcode.cleanup(arg[1])+"\">";
	case MOV:
		arg = o.getArgument().split(",");
		return  "<mov source=\""+Opcode.cleanup(arg[0])+
			"\" destination=\""+Opcode.cleanup(arg[1])+"\">";
	}

	return "<unknown value=\""+o.toString()+"\"/>";
  }

  public OpcodeType getType(Opcode o)
  {
	String words = o.getOpcode();

	if ("ret".equals(words))
		return OpcodeType.RETURN;

	if ("call".equals(words))
		return OpcodeType.CALL;

	if ("pop".equals(words))
		return OpcodeType.POP;

	if ("push".equals(words))
		return OpcodeType.PUSH;

	if ("xor".equals(words))
		return OpcodeType.XOR;

	if ("nop".equals(words))
		return OpcodeType.NOP;

	if ("mov".equals(words))
		return OpcodeType.MOV;

	if ("and".equals(words))
		return OpcodeType.AND;

	String[] ops = new String[]{"jmp","ja","jb","jz","jnz","je"};
	String op = o.toString();
	for(int i=0;i<ops.length;i++)
		if (ops[i].equals(op))
			return OpcodeType.BRANCH;

	return OpcodeType.UNKNOWN;
  }
}
