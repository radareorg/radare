include ../config.mk
include ../src/objects.mk
OBJ+=$(pref ../src/,${crOBJ})

# OBJ+=../libps2fd/*.o
# OBJ+=../libps2fd/arch/i386.o
# OBJ+=../libps2fd/arch/i386-bp.o
# OBJ+=../libps2fd/arch/i386-hack.o
OBJ+= grava/*.o

include ../src/dbg/objects.mk
#include ../objects.mk
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
OBJ+=../src/plug/io/gdb.o
#OBJ+=../rasc/sysproxy.o ../rasc/syscall-linux.o ../rasc/rpc.o

CFLAGS+=-DRADARE_CORE
VALAFLAGS=--save-temps
VALAFLAGS=-g

all: ../src/radare.o
	${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala
	${CC} -lewf -g ${OBJ} *.o `pkg-config --cflags --libs gtk+-2.0 vte` -lreadline

c:
	${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala

clean:
	rm -f *.o a.out
