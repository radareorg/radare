# core
#crOBJ=main.o
crOBJ=

OLLYOBJ=arch/x86/ollyasm/assembl.o
OLLYOBJ+=arch/x86/ollyasm/asmserv.o
OLLYOBJ+=arch/x86/ollyasm/disasm.o
crOBJ+=${OLLYOBJ}

crOBJ+=radare.o stripstr.o readline.o search.o undo.o rio.o cons.o trace.o vm.o data.o
crOBJ+=config.o flags.o utils.o environ.o visual.o print.o cmds.o binparse.o section.o
crOBJ+=plugin.o socket.o analyze.o code.o rabin.o project.o hack.o bytepat.o macros.o
crOBJ+=rasm/rasm.o rasm/olly.o rasm/nasm.o rasm/x86.o rasm/ppc.o rasm/arm.o rasm/java.o dietline.o
crOBJ+=vars.o rdb.o radiff/rdbdiff.o rasm/rsc.o b64.o rtr.o btree.o

# plug/io

crOBJ+=plug/io/haret.o plug/io/winedbg.o plug/io/gxemul.o plug/io/remote.o plug/io/windbg.o
crOBJ+=plug/io/gdb.o plug/io/posix.o plug/io/gdbx.o plug/io/socket.o pas.o ranges.o
crOBJ+=plug/io/shm.o plug/io/mmap.o plug/io/malloc.o plug/io/bfdbg.o plug/io/serial.o
#plug/io/winegdb.o
#GDBWRAP
CFLAGS+=-Iplug/io/libgdbwrap/include
crOBJ+=plug/io/gdbwrap.o plug/io/libgdbwrap/interface.o plug/io/libgdbwrap/gdbwrapper.o


# rahash

crOBJ+=rahash/entropy.o rahash/hash.o rahash/crc16.o aes-find.o

# arch

crOBJ+=arch/csr/dis.o
crOBJ+=arch/csr/code.o

crOBJ+=arch/msil/demsil.o
crOBJ+=arch/msil/code.o

crOBJ+=arch/arm/code.o
crOBJ+=arch/arm/disarm.o
crOBJ+=arch/arm/gnudisarm.o
crOBJ+=arch/arm/gnu/arm-dis.o

crOBJ+=arch/bf/code.o

crOBJ+=arch/z80/code.o
crOBJ+=arch/z80/z80_disassembler.o

crOBJ+=arch/8051/code.o
crOBJ+=arch/8051/global.o
crOBJ+=arch/8051/pass1.o
crOBJ+=arch/8051/pass2.o

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

crOBJ+=arch/ppc/ppc-dis.o
crOBJ+=arch/ppc/ppc-opc.o
crOBJ+=arch/ppc/gnudisppc.o
crOBJ+=arch/ppc/code.o
ifeq (${NONFREE},1)
crOBJ+=arch/ppc/ppc_disasm.o
else
ifeq (${NONFREE},)
crOBJ+=arch/ppc/ppc_disasm.o
endif
endif

crOBJ+=arch/x86/vm.o
crOBJ+=arch/x86/code.o
crOBJ+=arch/x86/dislen.o
crOBJ+=arch/x86/udis86/syn.o
crOBJ+=arch/x86/udis86/input.o
#crOBJ+=arch/x86/udis86/opcmap.o
crOBJ+=arch/x86/udis86/udis86.o
crOBJ+=arch/x86/udis86/decode.o
crOBJ+=arch/x86/udis86/syn-att.o
crOBJ+=arch/x86/udis86/itab.o
#crOBJ+=arch/x86/udis86/mnemonics.o
crOBJ+=arch/x86/udis86/syn-intel.o
crOBJ+=arch/x86/udis86/syn-pseudo.o

crOBJ+=arch/java/code.o
crOBJ+=rabin/javasm.o

crOBJ+=arch/m68k/code.o
crOBJ+=arch/m68k/m68k_disasm.o

# plugins and so
CFLAGS+=-Wall

crOBJ+=${RADARE_OBJ}
