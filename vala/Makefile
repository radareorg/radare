include ../config.mk
include ../src/objects.mk

OBJ+=$(pref ../src/,${crOBJ})

# OBJ+=../libps2fd/*.o
# OBJ+=../libps2fd/arch/i386.o
# OBJ+=../libps2fd/arch/i386-bp.o
# OBJ+=../libps2fd/arch/i386-hack.o
#OBJ+= grava/*.o

include ../src/dbg/objects.mk
#include ../objects.mk
#FOO=$(subst dbg/,,${psOBJ})
OBJ+=$(pref ../src/dbg,${psOBJ})
#OBJ+=../src/dbg/arch/${CPU}-debug.o

OBJ+=${FOO}
OBJ+=../src/dbg/*.o
OBJ+=../src/dbg/arch/${CPU}.o
OBJ+=../src/dbg/arch/${CPU}-bp.o
OBJ+=../src/dbg/arch/${CPU}-hack.o
OBJ+=../src/dbg/unix/*.o

OBJ+=../src/arch/x86/udis86/*.o
OBJ+=../src/rabin/javasm.o
OBJ+=../src/plug/io/gdb.o
OBJ+=../src/radare.o
OBJ+=../src/utils.o
OBJ+=../src/inout.o
OBJ+=../src/project.o
OBJ+=../src/config.o
OBJ+=../src/flags.o
OBJ+=../src/cons.o
OBJ+=../src/print.o
OBJ+=../src/code.o
OBJ+=../src/socket.o
OBJ+=../src/dietline.o
OBJ+=../src/plugin.o
OBJ+=../src/plug/hack/*.o
OBJ+=../src/plug/io/*.o
#OBJ+=../rasc/sysproxy.o ../rasc/syscall-linux.o ../rasc/rpc.o

CFLAGS+=-DRADARE_CORE -I.
VALAFLAGS=--save-temps
VALAFLAGS=-g

%.o:%.c
	${CC} -c `pkg-config --cflags vte gtk+-2.0` $<

#shell.o

VOBJ=config.o core.o debug.o debugger.o info.o inout.o line.o list.o logs.o plugin.o project.o radget.o term.o visor.o default_layout.o edge.o graph.o layout.o node.o renderer.o shape.o widget.o
BIN=a.out

#${CC} -lewf -g ${OBJ} ${VOBJ} -o ${BIN} `pkg-config gtk+-2.0 vte --libs`
all: all_vala ${VOBJ}

all_vala:
	@do=0; \
	for o in ${VOBJ}; do \
		v=$$(echo grava/$$o|sed -e s,o$$,vala,) ; \
		c=$$(echo $$o|sed -e s,o$$,c,) ; \
		if [ $$v -nt $$c ]; then echo "updated $$v"; do=1 ; fi ; \
	done ; \
	if [ $${do} = 1 ]; then \
	echo "Generating C files..." ; \
	valac --help > /dev/null && valac ${VALAFLAGS} -C -c --pkg vte --pkg gtk+-2.0 grava/*.vala ; \
	fi

#${CC} -lewf -g ${OBJ} *.c `pkg-config --cflags --libs gtk+-2.0 vte` -lreadline
#${CC} -lewf -g ${OBJ} *.o `pkg-config --cflags --libs gtk+-2.0 vte` -lreadline

allavas: ../src/radare.o
	#${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala
	${CC} -lewf -g ${OBJ} *.o `pkg-config --cflags --libs gtk+-2.0` `pkg-config --cflags --libs vte` -lreadline
c:
	${VALAC} ${VALAFLAGS} -c --pkg vte --pkg gtk+-2.0 *.vala api/*.vala panel/*.vala

clean:
	rm -f *.o a.out
