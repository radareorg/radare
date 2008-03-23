include ../../config.mk

BIN=rdbdiff
OBJ=main.o rdb.o rdbdiff.o ../hasher/crc16.o  ../utils.o ../w32utils.o
#../utils.o
CC?=gcc
CFLAGS+=-DRDBDIFF

##UDIS86##
OBJ+=../arch/x86/udis86/*.o
##UDIS86##

all: pre ${BIN}${BINSFX}
	@echo ignored by conspiracy

pre:
	-rm -f ../utils.o

${BIN}${BINSFX}: ${OBJ}
	${CC} -g ${OBJ} -o ${BIN}${BINSFX}

clean:
	-rm -f *.o ${BIN}${BINSFX}
