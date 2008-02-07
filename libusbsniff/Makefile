include ../config.mk
LDFLAGS+=${DL_LIBS}

.SUFFIXES: .c .o
.c.o:
	${E} 'Compiling $<'
	${Q}$(CC) -c $(CFLAGS) -o $@ $<


all: hexdump.o
	${E} 'Linking libfdsniff.so'
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libfdsniff.c -shared -fPIC -o libfdsniff.${SHARED_EXT}
##USBSNF##
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libusbsniff.c -shared -fPIC -o libusbsniff.${SHARED_EXT}
##USBSNF##

clean:
	${E} 'Cleaning libusbsniff'
	-${Q}rm -f *.${SHARED_EXT} a.out

mrproper:
	${E} 'Distcleaning libusbsniff'
	-${Q}rm Makefile
