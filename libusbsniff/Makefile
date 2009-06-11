include ../config.mk
LIBS+=${DL_LIBS}

.SUFFIXES: .c .o
.c.o:
	@echo 'Compiling $<'
	$(CC) -c $(CFLAGS) ${SHARED_CFLAGS} -o $@ $<


all: hexdump.o libfdsniff.${SHARED_EXT} libusbsniff.${SHARED_EXT}

libfdsniff.${SHARED_EXT}: hexdump.o libfdsniff.o
	-${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libfdsniff.o ${LIBS} -o libfdsniff.${SHARED_EXT} 2>/dev/null

libusbsniff.${SHARED_EXT}: hexdump.o libusbsniff.o
##USBSNF##
	-${CC} ${CFLAGS} ${LDFLAGS} ${SHARED_CFLAGS} hexdump.o libusbsniff.o ${LIBS} -o libusbsniff.${SHARED_EXT} 2>/dev/null
##USBSNF##

clean:
	@echo 'Cleaning libusbsniff'
	-rm -f *.${SHARED_EXT} *.o a.out
