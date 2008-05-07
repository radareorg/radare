include ../../config.mk

OBJ=rdbdiff.o radiff.o ../utils.o ../rdb.o ../hasher/crc16.o
CFLAGS+=-I..

all: _cleanmeup ${OBJ} _bdiff
	${CC} ${LDFLAGS} ${OBJ} -o radiff

_bdiff:
	cd bdiff && ${MAKE} all

_cleanmeup:
	-rm -f ../utils.o

clean:
	cd bdiff && ${MAKE} clean
	-rm -f radiff ${OBJ} *.o
