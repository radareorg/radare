include ../config.mk

OBJ= ../src/radare.o ../src/stripstr.o ../src/readline.o ../src/tsearch.o ../src/undo.o ../src/dietline.o ../src/aes-find.o ../src/plugins/ewf.o ../src/hack.o ../src/rdb/rdb.o
OBJ+= ../src/config.o ../src/flags.o ../src/utils.o ../src/environ.o ../src/visual.o ../src/print.o ../src/cmds.o ../src/binparse.o ../src/arch/ppc/code.o
OBJ+= ../src/plugin.o ../src/plugins/posix.o ../src/plugins/remote.o ../src/socket.o ../src/plugins/haret.o ../src/analyze.o ../src/hist.o ../src/udis.o
OBJ+= ../src/arch/arm/disarm.o ../src/io.o ../src/plugins/debug.o ../src/cons.o ../src/project.o ../src/arch/java/code.o ../src/arch/arm/code.o 
OBJ+= ../src/hasher/hash.o ../src/hasher/entropy.o ../src/rasm/rasm.o ../src/grava.o ../src/plugins/winedbg.o ../src/plugins/gxemul.o ../src/arch/x86/dislen.o
OBJ+= ../src/rasm/x86.o ../src/rasm/arm.o ../src/rasm/ppc.o ../src/rdb/rdbdiff.o ../src/arch/ppc/ppc_disasm.o ../src/rasm/java.o ../src/arch/x86/code.o
OBJ+= ../src/hasher/crc16.o ../src/rabin.o ../src/arch/m68k/m68k_disasm.o
# OBJ+=../libps2fd/*.o
# OBJ+=../libps2fd/arch/i386.o
# OBJ+=../libps2fd/arch/i386-bp.o
# OBJ+=../libps2fd/arch/i386-hack.o
OBJ+= grava/*.o

include ../src/dbg/objects.mk
#FOO=$(subst dbg/,,${psOBJ})
OBJ+=$(pref ../src/dbg,${psOBJ})
OBJ+=../src/dbg/*.o
OBJ+=../src/dbg/arch/${CPU}.o
OBJ+=../src/dbg/arch/${CPU}-bp.o
OBJ+=../src/dbg/arch/${CPU}-hack.o
OBJ+=../src/dbg/unix/*.o
#OBJ+=../src/dbg/arch/${CPU}-debug.o

OBJ+=${FOO}

OBJ+=../src/arch/x86/udis86/*.o
OBJ+=../src/arch/java/javasm.o
OBJ+=../src/plugins/gdb.o
#OBJ+=../rasc/sysproxy.o ../rasc/syscall-linux.o ../rasc/rpc.o

VALAFLAGS=--save-temps
VALAFLAGS=-g

all:
	${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala
	${CC} -lewf -g ${OBJ} *.o `pkg-config --cflags --libs gtk+-2.0 vte` -lreadline

clean:
	rm -f *.o a.out
