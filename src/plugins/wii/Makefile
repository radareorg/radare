OBJ=grecko.o usbdev.o

all: test ${OBJ} gecko.o
	${CC} ${OBJ} -o test.so -shared -lusb

test: ${OBJ} test.o
	${CC} ${OBJ} test.o -o test -lusb

clean:
	-rm -f ${OBJ} test test.so
