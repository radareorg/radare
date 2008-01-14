BIN=rdbdiff
OBJ=main.o rdb.o rdbdiff.o ../hasher/crc16.o  ../utils.o
#../utils.o
CC?=gcc
CFLAGS+=-DRDBDIFF -g

##UDIS86##
OBJ+=../arch/x86/udis86/*.o
##UDIS86##

all: pre all_orig
	@echo ignored by conspiracy

pre:
	-rm -f ../utils.o

all_orig: ${OBJ}
	${CC} -g ${OBJ} -o ${BIN}
