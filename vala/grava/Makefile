include ../../config.mk

SOURCES=default_layout.vala edge.vala graph.vala layout.vala \
	node.vala renderer.vala widget.vala shape.vala
VALAFLAGS=--pkg gtk+-2.0 --pkg cairo -g

all:
	cd ../ && ${MAKE} all
	#valac cmain.c --save-temps *.vala --pkg gtk+-2.0 --pkg cairo -o a.out

radare: a.out

a.out: $(SOURCES:.vala=.o)
	#[ "${HAVE_VALAC}" = 1 ] && valac -c -g --save-temps *.vala --pkg gtk+-2.0 --pkg cairo -o a.out
	# only for releases (not development)
	${CC} `pkg-config gtk+-2.0 cairo --cflags --libs` cmain.c $(SOURCES:.vala=.o) -o a.out

%.o:%.vala
	valac --save-temps $(VALAFLAGS) -c $(SOURCES)

c:
	valac --save-temps *.c --pkg gtk+-2.0 --pkg cairo -o a.out

novala:
	${CC} `pkg-config gtk+-2.0 cairo --cflags --libs` -c *.c
	-rm -f cmain.o

clean:
	-mv cmain.c ___
	-rm -f *.c *.h *.o a.out
	-mv ___ cmain.c
