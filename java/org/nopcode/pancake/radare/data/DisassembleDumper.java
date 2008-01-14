package org.nopcode.pancake.radare.data;

import java.io.*;
import java.util.*;
import org.nopcode.pancake.radare.asm.*;

public class DisassembleDumper extends DataDumper
{
  private static BufferedReader br = null;
  private static Process p = null;

  public static void println(byte[] bytes)
  {
	System.out.println(DisassembleDumper.toString(bytes));
  }

  public static Opcode[] getOpcodes(byte[] bytes)
  {
	List<String> ops = new ArrayList<String>();
	Opcode[] opz = null;
	
	try {
		String[] args = { "rsc", "dasm", HexDumper.toString(bytes) };
		executeProcess(args);
		String line = null;
		while((line = br.readLine()) != null) {
			ops.add(line);
		}
		Object[] lines = ops.toArray();
		opz = new Opcode[lines.length];
		for(int i = lines.length-1; i>=0; i--) {
			opz[i] = new Opcode((String)lines[i]);
		}
	//	p.exitValue();
	} catch(Exception e) {
		e.printStackTrace();
	}
	
	return opz;
  }

  private static void executeProcess(String[] args) throws RuntimeException, IOException
  {
	Runtime r = Runtime.getRuntime();
	p = r.exec(args);
	br = new BufferedReader(new InputStreamReader(p.getInputStream()));
  }

  public static String toString(byte[] bytes)
  {
	StringBuffer sb = new StringBuffer();

	try {
		String[] args = { "rsc", "dasm", HexDumper.toString(bytes) };
		executeProcess(args);
		String line = null;
		while((line = br.readLine()) != null) {
			sb = sb.append(line+"\n");
		}
	} catch(Exception e) {
		e.printStackTrace();
	}

	return sb.toString();
  }
}
