all: hash sc socket
	@true

sc:
	valac -C --vapidir=${PWD}/.. sc.vala --pkg r_syscall
	gcc sc.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_syscall -Wl,-R../../syscall -L../../syscall -o sc
	

hash:
	valac -C --vapidir=${PWD}/.. hash.vala --pkg r_hash
	gcc hash.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_hash -Wl,-R../../hash -L../../hash -o hash

socket:
	valac -C --vapidir=${PWD}/.. socket.vala --pkg r_socket
	gcc socket.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_socket -Wl,-R../../socket -L../../socket -o socket

clean:
	-rm -f *.c hash sc *.o *.h
