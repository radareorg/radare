include ../config.mk
LDFLAGS+=${DL_LIBS}

all: hexdump.o
	-${CC} ${CFLAGS} ${LDFLAGS} hexdump.o libfdsniff.c -shared -fPIC -o libfdsniff.so
##USBSNF##
	-${CC} ${CFLAGS} ${LDFLAGS} hexdump.o libusbsniff.c -shared -fPIC -o libusbsniff.so
##USBSNF##

clean:
	-rm -f *.so a.out

mrproper:
	-rm Makefile
