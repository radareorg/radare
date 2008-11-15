# Compiler
CC?=gcc
CFLAGS+=-I../include -fPIC
CC_LIB=${CC} -shared -o ${LIBSO}
CC_AR=ar -r ${LIBAR} ${OBJ}

# Output
EXT_AR=a
EXT_SO=so
LIBAR=${LIB}.${EXT_AR}
LIBSO=${LIB}.${EXT_SO}

# Rules
all: ${OBJ} ${LIBSO} ${LIBAR}
	@([ -e t/Makefile ] && cd t && ${MAKE} all)

${LIBSO}:
	${CC_LIB} ${LDFLAGS} ${OBJ}

${LIBAR}:
	${CC_AR} ${OBJ}

clean:
	-rm -f ${LIBSO} ${LIBAR} ${OBJ} ${BIN}
	@([ -e t/Makefile ] && cd t && ${MAKE} clean)
