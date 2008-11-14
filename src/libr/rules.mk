# Compiler
CC?=gcc
CFLAGS+=-I../include
CC_LIB=${CC} -shared -o ${LIBSO}
CC_AR=ar -r ${LIBAR} ${OBJ}

# Output
EXT_AR=a
EXT_SO=so
LIBAR=${LIB}.${EXT_AR}
LIBSO=${LIB}.${EXT_SO}

# Rules
all: ${OBJ} ${LIBSO} ${LIBAR}

${LIBSO}:
	${CC_LIB} ${LDFLAGS} ${OBJ}

${LIBAR}:
	${CC_AR} ${OBJ}

clean:
	-rm -f ${LIBSO} ${LIBAR} ${OBJ} ${BIN}
