include ../../config.mk
#all:
#	valac --save-temps *.vala --pkg gtk+-2.0 --pkg cairo -o a.out

all:
	valac -g cmain.c --save-temps *.vala --pkg gtk+-2.0 --pkg cairo -o a.out

radare:
	-[ "${HAVE_VALAC}" = 1 ] && valac -c -g --save-temps *.vala --pkg gtk+-2.0 --pkg cairo -o a.out
	# only for releases (not development)
	-gcc `pkg-config gtk+-2.0 cairo --cflags --libs` *.c -o a.out
	

c:
	valac --save-temps *.c --pkg gtk+-2.0 --pkg cairo -o a.out

novala:
	gcc `pkg-config gtk+-2.0 cairo --cflags --libs` -c *.c
	-rm -f cmain.o

clean:
	-mv cmain.c ___
	-rm *.c *.h *.o a.out
	-mv ___ cmain.c
