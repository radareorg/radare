# core
crOBJ=main.o

crOBJ=radare.o stripstr.o readline.o search.o undo.o io.o cons.o trace.o
crOBJ+=config.o flags.o utils.o environ.o visual.o print.o cmds.o binparse.o
crOBJ+=plugin.o socket.o analyze.o hist.o udis.o rabin.o project.o hack.o
crOBJ+=rasm/rasm.o rasm/x86.o rasm/ppc.o rasm/arm.o rasm/java.o dietline.o

# plug/io

crOBJ+=plug/io/haret.o plug/io/winedbg.o plug/io/gxemul.o plug/io/remote.o
crOBJ+=plug/io/gdb.o plug/io/posix.o plug/io/gdbx.o plug/io/socket.o

# hasher

crOBJ+=hasher/entropy.o hasher/hash.o hasher/crc16.o aes-find.o

# arch

crOBJ+=arch/arm/code.o
crOBJ+=arch/arm/gnudisarm.o
crOBJ+=arch/arm/gnu/arm-dis.o
crOBJ+=arch/arm/disarm.o

crOBJ+=arch/mips/code.o
crOBJ+=arch/mips/gnudismips.o
crOBJ+=arch/mips/mips-dis.o
crOBJ+=arch/mips/mips-opc.o
crOBJ+=arch/mips/mips16-opc.o

crOBJ+=arch/ppc/code.o
crOBJ+=arch/ppc/ppc_disasm.o

crOBJ+=arch/x86/code.o
crOBJ+=arch/x86/dislen.o
crOBJ+=arch/x86/udis86/decode.o
crOBJ+=arch/x86/udis86/syn-intel.o
crOBJ+=arch/x86/udis86/syn-att.o
crOBJ+=arch/x86/udis86/syn-pseudo.o
crOBJ+=arch/x86/udis86/syn.o
crOBJ+=arch/x86/udis86/udis86.o
crOBJ+=arch/x86/udis86/mnemonics.o
crOBJ+=arch/x86/udis86/input.o
crOBJ+=arch/x86/udis86/opcmap.o

crOBJ+=arch/java/code.o
crOBJ+=arch/java/javasm.o

crOBJ+=arch/m68k/m68k_disasm.o

crOBJ+=rdb/rdb.o rdb/rdbdiff.o

# plugins and so

crOBJ+=${RADARE_OBJ}
