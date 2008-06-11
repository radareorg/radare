LIBS=-llua

all: posix.so
	@echo

posix.so: posix.o
	${CC} -fPIC ${LIBS} -shared -o posix.so

posix.o:
	${CC} -c posix.c
	
install:
	mkdir -p /usr/lib/lua/5.1
	cp posix.so /usr/lib/lua/5.1/

clean:
	-rm -f *.o *.so
