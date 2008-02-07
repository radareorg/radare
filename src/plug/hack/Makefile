include ../../../config.mk

CFLAGS=-I../

all:
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.so

clean:
	-rm -f *.${SHARED_EXT} *.o
