include ../config.mk
LIBS+=${DL_LIBS}

.SUFFIXES: .c .o
.c.o:
	${E} 'Compiling $<'
	${Q}$(CC) -c $(CFLAGS) -o $@ $<


all: hexdump.o libfdsniff.${SHARED_EXT} libusbsniff.${SHARED_EXT}

libfdsniff.${SHARED_EXT}: hexdump.o libfdsniff.o
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libfdsniff.o ${LIBS} -shared -fPIC -o libfdsniff.${SHARED_EXT} 2>/dev/null

libusbsniff.${SHARED_EXT}: hexdump.o libusbsniff.o
##USBSNF##
	-${Q}${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libusbsniff.o ${LIBS} -shared -fPIC -o libusbsniff.${SHARED_EXT} 2>/dev/null
##USBSNF##

clean:
	${E} 'Cleaning libusbsniff'
	-${Q}rm -f *.${SHARED_EXT} *.o a.out

mrproper:
	${E} 'Distcleaning libusbsniff'
	-${Q}rm Makefile
