OBJ=grecko.o usbdev.o

all: test ${OBJ} gecko.o
	${CC} ${OBJ} ${SHARED_CFLAGS} -o test.${SHARED_EXT} -lusb

test: ${OBJ} test.o
	${CC} ${OBJ} test.o -o test -lusb

clean:
	-rm -f ${OBJ} test test.${SHARED_EXT}
