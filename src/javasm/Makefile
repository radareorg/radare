include ../../config.mk

OBJ=../rabin/javasm.o main.o
BIN=javasm${BINSFX}

all: ${BIN}

${BIN}: ${OBJ}
	${CC} ${LDFLAGS} ${OBJ} ${LIBS} -o ${BIN}

clean:
	-rm -f ${BIN} ${OBJ}
