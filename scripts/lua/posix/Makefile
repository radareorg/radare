include ../../../config.mk

LIBS=${LUA_LIBS}

all: posix.so
	@echo

posix.so:
	-${CC} ${LIBS} -fPIC posix.c -shared -o posix.so

#posix.so: posix.o
#	${CC} -fPIC ${LIBS} -shared -o posix.so

posix.o:
	${CC} -fPIC -shared -c posix.c
	
install:
	mkdir -p /usr/lib/lua/5.1
	cp posix.so /usr/lib/lua/5.1/

clean:
	-rm -f *.o *.so
