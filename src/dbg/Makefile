include ../../config.mk
CFLAGS+=-I ../arch/ -I ../include
include objects.mk
FOO=$(subst dbg/,,${psOBJ})
OBJ+=${FOO}
LIB=ptrace.${SHARED_EXT}
#LIBPRE=libps2fd.so

#all: ptrace.o ${OBJ}
#	${CC} ${LDFLAGS} ptrace.o ${OBJ} -shared -o ${LIB}

all: ${OBJ}
	@-${Q}echo [src/dbg] debugger layer build done

#${CC} ${LDFLAGS} ${OBJ} ${SHARED_CFLAGS} -o ${LIB}

# why?
#  ..I should rewrite this stuff..
#
#libpreload: libps2fd.o ${OBJ}
#	${CC} ${LDFLAGS} @DL_LIBS@ libps2fd.o ${OBJ} -shared -o ${LIBPRE}

clean:
	${E} '[src/dbg] Cleaning ...'
	-${Q}rm -f ptrace.o ${OBJ} ${LIB} ${LIBPRE} demo dbg/unix/*.o dbg/win32/*.o
