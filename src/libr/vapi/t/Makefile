all:
	valac -C --vapidir=${PWD}/.. hello.vala --pkg r_hash
	gcc hello.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_hash -Wl,-R../../hash -L../../hash -o hello

clean:
	-rm -f *.c hello *.o *.h
