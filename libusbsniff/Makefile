include ../config.mk
LDFLAGS+=${DL_LIBS}

.SUFFIXES: .c .o
.c.o:
	${E} 'Compiling $<'
	${Q}$(CC) -c $(CFLAGS) -o $@ $<


all: hexdump.o
	${E} 'Linking libfdsniff.so'
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} hexdump.o libfdsniff.c -shared -fPIC -o libfdsniff.so
##USBSNF##
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} hexdump.o libusbsniff.c -shared -fPIC -o libusbsniff.so
##USBSNF##

clean:
	${E} 'Cleaning libusbsniff'
	-${Q}rm -f *.so a.out

mrproper:
	${E} 'Distcleaning libusbsniff'
	-${Q}rm Makefile
