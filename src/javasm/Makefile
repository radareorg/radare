include ../../config.mk

OBJ=main.o
BIN=javasm${BINSFX}
CFLAGS+=-DHAVE_MAIN=1

all: ${BIN}

${BIN}: ${OBJ}
	${CC} ${LDFLAGS} ${OBJ} ${LIBS} -o ${BIN}

clean:
	-rm -f ${BIN} ${OBJ}
