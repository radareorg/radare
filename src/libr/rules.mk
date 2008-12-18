include ../config.mk

# Compiler
CC?=gcc
CFLAGS+=-I../include -fPIC
CC_LIB=${CC} -shared -o ${LIBSO}
CC_AR=ar -r ${LIBAR}
LINK?=

# Debug
CFLAGS+=-g -Wall

# Output
EXT_AR=a
EXT_SO=so
LIBAR=${LIB}.${EXT_AR}
LIBSO=${LIB}.${EXT_SO}

# Rules
all: ${OBJ} ${LIBSO} ${LIBAR}
	@if [ -e t/Makefile ]; then (cd t && ${MAKE} all) ; else true ; fi

${LIBSO}:
	${CC_LIB} ${LDFLAGS} ${LINK} ${OBJ}

${LIBAR}:
	${CC_AR} ${OBJ}

clean:
	-rm -f ${LIBSO} ${LIBAR} ${OBJ} ${BIN}
	@if [ -e t/Makefile ]; then (cd t && ${MAKE} clean) ; else true ; fi

.PHONY: all clean ${LIBSO} ${LIBAR}
