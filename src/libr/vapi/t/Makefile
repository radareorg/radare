all: hash sc socket asm search
	@true

sc:
	valac -C --vapidir=${PWD}/.. sc.vala --pkg r_syscall
	gcc sc.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_syscall -Wl,-R../../syscall -L../../syscall -o sc

search:
	valac -C --vapidir=${PWD}/.. search.vala --pkg r_search
	gcc search.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ \
		-lr_search -Wl,-R../../search -L../../search \
		-lr_util -Wl,-R../../util -L../../util \
		-g -o search
	
asm:
	valac -C --vapidir=${PWD}/.. asm.vala --pkg r_asm
	gcc asm.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_asm -Wl,-R../../asm -L../../asm -o asm

hash:
	valac -C --vapidir=${PWD}/.. hash.vala --pkg r_hash
	gcc hash.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_hash -Wl,-R../../hash -L../../hash -o hash

socket:
	valac -C --vapidir=${PWD}/.. socket.vala --pkg r_socket
	gcc socket.c `pkg-config gobject-2.0 --libs --cflags` -I../../include/ -lr_socket -Wl,-R../../socket -L../../socket -o socket

clean:
	-rm -f *.c hash sc *.o *.h
