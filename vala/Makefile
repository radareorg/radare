include ../config.mk

OBJ= ../src/radare.o ../src/stripstr.o ../src/readline.o ../src/search.o ../src/tsearch.o ../src/undo.o
OBJ+= ../src/config.o ../src/flags.o ../src/utils.o ../src/environ.o ../src/visual.o ../src/print.o ../src/cmds.o ../src/binparse.o
OBJ+= ../src/plugin.o ../src/posix.o ../src/remote.o ../src/socket.o ../src/haret.o ../src/analyze.o ../src/history.o ../src/udis.o
OBJ+= ../src/disarm.o ../src/io.o
# OBJ+=../libps2fd/*.o
# OBJ+=../libps2fd/arch/i386.o
# OBJ+=../libps2fd/arch/i386-bp.o
# OBJ+=../libps2fd/arch/i386-hack.o

include ../src/dbg/objects.mk
OBJ+=$(pref ../src/,${psOBJ})
OBJ+=../src/dbg/*.o
OBJ+=../src/dbg/arch/${CPU}.o
OBJ+=../src/dbg/arch/${CPU}-bp.o
OBJ+=../src/dbg/arch/${CPU}-hack.o
OBJ+=../src/dbg/arch/${CPU}-debug.o

OBJ+=../src/arch/x86/udis86/*.o
OBJ+=../src/arch/java/javasm.o
OBJ+=../libgdb2fd/libgdb2fd.o
OBJ+=../rasc/sysproxy.o ../rasc/syscall-linux.o ../rasc/rpc.o

VALAFLAGS=--save-temps
VALAFLAGS=-g

all:
	${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala
	${CC} -g ${OBJ} *.o `pkg-config --cflags --libs gtk+-2.0 vte` -lreadline

clean:
	rm -f *.o a.out
