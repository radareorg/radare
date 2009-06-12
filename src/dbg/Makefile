include ../../config.mk
CFLAGS+=-I../arch/ -I ../include -I/usr/local/include
include objects.mk
FOO=$(subst dbg/,,${psOBJ})
OBJ+=${FOO}
LIB=ptrace.${SHARED_EXT}
#LIBPRE=libps2fd.so

#all: ptrace.o ${OBJ}
#	${CC} ${LDFLAGS} ptrace.o ${OBJ} ${LIBS} -shared -o ${LIB}

all: ${OBJ}
	@-echo [src/dbg] debugger layer build done

#${CC} ${LDFLAGS} ${OBJ} ${LIBS} ${SHARED_CFLAGS} -o ${LIB}

clean:
	@echo '[src/dbg] Cleaning ...'
	-rm -f ptrace.o ${OBJ} ${LIB} ${LIBPRE} demo dbg/unix/*.o dbg/win32/*.o
