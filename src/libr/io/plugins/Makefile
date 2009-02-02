CFLAGS=-I../../include
all: ptrace.so
	@true

ptrace.so: ptrace.o
	${CC} -shared -o ptrace.so ptrace.o
	strip -s ptrace.so

clean:
	-rm -f *.so *.o
