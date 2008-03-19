# core

OBJ=main.o radare.o stripstr.o readline.o search.o undo.o io.o cons.o trace.o
OBJ+=config.o flags.o utils.o environ.o visual.o print.o cmds.o binparse.o
OBJ+=plugin.o socket.o analyze.o hist.o udis.o rabin.o project.o hack.o
OBJ+=rasm/rasm.o rasm/x86.o rasm/ppc.o rasm/arm.o rasm/java.o dietline.o

# plug/io

OBJ+=plug/io/haret.o plug/io/winedbg.o plug/io/gxemul.o plug/io/remote.o
OBJ+=plug/io/gdb.o plug/io/posix.o plug/io/gdbx.o

# hasher

OBJ+=hasher/entropy.o hasher/hash.o hasher/crc16.o aes-find.o

# arch

OBJ+=arch/arm/code.o
OBJ+=arch/arm/gnudisarm.o
OBJ+=arch/arm/gnu/arm-dis.o
OBJ+=arch/arm/disarm.o

OBJ+=arch/mips/code.o
OBJ+=arch/mips/gnudismips.o
OBJ+=arch/mips/mips-dis.o
OBJ+=arch/mips/mips-opc.o
OBJ+=arch/mips/mips16-opc.o

OBJ+=arch/ppc/code.o
OBJ+=arch/ppc/ppc_disasm.o

OBJ+=arch/x86/code.o
OBJ+=arch/x86/dislen.o
OBJ2+=arch/x86/udis86/*.o

OBJ+=arch/java/code.o
OBJ+=arch/java/javasm.o

OBJ+=arch/m68k/m68k_disasm.o

OBJ+=rdb/rdb.o rdb/rdbdiff.o

OBJ+=${RADARE_OBJ}
