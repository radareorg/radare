include ../../config.mk
CFLAGS+=-I ../arch/
include objects.mk
FOO=$(subst dbg/,,${psOBJ})
OBJ+=${FOO}
LIB=ptrace.so
#LIBPRE=libps2fd.so

#all: ptrace.o ${OBJ}
#	${CC} ${LDFLAGS} ptrace.o ${OBJ} -shared -o ${LIB}

all: ${OBJ}
	#${CC} ${LDFLAGS} ${OBJ} -shared -o ${LIB}

# why?
#  ..I should rewrite this stuff..
#
#libpreload: libps2fd.o ${OBJ}
#	${CC} ${LDFLAGS} @DL_LIBS@ libps2fd.o ${OBJ} -shared -o ${LIBPRE}

clean:
	${E} 'Cleaning src/dbg'
	-${Q}rm -f ptrace.o ${OBJ} ${LIB} ${LIBPRE} demo dbg/unix/*.o dbg/win32/*.o
