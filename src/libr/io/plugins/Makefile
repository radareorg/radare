CFLAGS=-I../../include
all: ptrace.so
	@true

ptrace.so: ptrace.o
	${CC} ${CFLAGS} -fPIC -shared -o ptrace.so ptrace.c
	#strip -s ptrace.so

clean:
	-rm -f *.so *.o
