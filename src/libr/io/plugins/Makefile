CFLAGS=-I../../include -Wall

all: ptrace.so dbg.so
	@true

ptrace.so: ptrace.o
	${CC} ${CFLAGS} -fPIC -shared -o ptrace.so ptrace.c
	#strip -s ptrace.so

dbg.so: dbg.o
	${CC} ${CFLAGS} -fPIC -shared -o dbg.so dbg.c -lr_io -L../../io
	#strip -s ptrace.so

clean:
	-rm -f *.so *.o
