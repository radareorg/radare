# core
#crOBJ=main.o
crOBJ=

OLLYOBJ=arch/x86/ollyasm/assembl.o
OLLYOBJ+=arch/x86/ollyasm/asmserv.o
OLLYOBJ+=arch/x86/ollyasm/disasm.o
crOBJ+=${OLLYOBJ}

crOBJ+=radare.o stripstr.o readline.o search.o undo.o io.o cons.o trace.o vm.o
crOBJ+=config.o flags.o utils.o environ.o visual.o print.o cmds.o binparse.o
crOBJ+=plugin.o socket.o analyze.o hist.o code.o rabin.o project.o hack.o bytepat.o
crOBJ+=rasm/rasm.o rasm/olly.o rasm/x86.o rasm/ppc.o rasm/arm.o rasm/java.o dietline.o

# plug/io

crOBJ+=plug/io/haret.o plug/io/winedbg.o plug/io/gxemul.o plug/io/remote.o
crOBJ+=plug/io/gdb.o plug/io/posix.o plug/io/gdbx.o plug/io/socket.o
crOBJ+=plug/io/shm.o plug/io/mmap.o plug/io/malloc.o
#plug/io/winegdb.o

# hasher

crOBJ+=hasher/entropy.o hasher/hash.o hasher/crc16.o aes-find.o

# arch

crOBJ+=arch/csr/dis.o
crOBJ+=arch/csr/code.o

crOBJ+=arch/arm/code.o
crOBJ+=arch/arm/disarm.o
crOBJ+=arch/arm/gnudisarm.o
crOBJ+=arch/arm/gnu/arm-dis.o

crOBJ+=arch/mips/vm.o
crOBJ+=arch/mips/code.o
crOBJ+=arch/mips/mips-dis.o
crOBJ+=arch/mips/mips-opc.o
crOBJ+=arch/mips/mips16-opc.o
crOBJ+=arch/mips/gnudismips.o

crOBJ+=arch/sparc/code.o
crOBJ+=arch/sparc/sparc-dis.o
crOBJ+=arch/sparc/sparc-opc.o
crOBJ+=arch/sparc/gnudisparc.o

crOBJ+=arch/ppc/code.o
crOBJ+=arch/ppc/ppc_disasm.o

crOBJ+=arch/x86/vm.o
crOBJ+=arch/x86/code.o
crOBJ+=arch/x86/dislen.o
crOBJ+=arch/x86/udis86/syn.o
crOBJ+=arch/x86/udis86/input.o
crOBJ+=arch/x86/udis86/opcmap.o
crOBJ+=arch/x86/udis86/udis86.o
crOBJ+=arch/x86/udis86/decode.o
crOBJ+=arch/x86/udis86/syn-att.o
crOBJ+=arch/x86/udis86/mnemonics.o
crOBJ+=arch/x86/udis86/syn-intel.o
crOBJ+=arch/x86/udis86/syn-pseudo.o

crOBJ+=arch/java/code.o
crOBJ+=rabin/javasm.o

crOBJ+=arch/m68k/code.o
crOBJ+=arch/m68k/m68k_disasm.o

crOBJ+=rdb.o radiff/rdbdiff.o

# plugins and so

crOBJ+=${RADARE_OBJ}
