include ../../config.mk

OBJ=main.o ../rdb.o ../utils.o ../w32utils.o ../hasher/crc16.o rdbdiff.o bytediff.o
BIN=radiff${BINSFX}
CC?=gcc
OBJ_RDB=rdbdiff_main.o rdbdiff.o ../rdb.o ../utils.o ../w32utils.o ../hasher/crc16.o
CFLAGS+=-I..


all: _pre ${OBJ} _bdiff rdbdiff$(BINSFX)
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ} -o ${BIN}

_pre:
	-rm -f ../utils.o

_bdiff:
	-cd bdiff && ${MAKE} all

rdbdiff${BINSFX}: ${OBJ_RDB}
	${CC} ${LDFLAGS} -g ${OBJ_RDB} -o rdbdiff${BINSFX}

clean:
	cd bdiff && ${MAKE} clean
	-rm -f ${OBJ_RDB} ${OBJ} ${BIN} rdbdiff

.PHONY: bdiff all clean
