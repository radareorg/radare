include ../../config.mk

OBJ=../rabin/javasm.o main.o
BIN=javasm${BINSFX}

all: ${BIN}
	@echo done

${BIN}: ${OBJ}
	${CC} ${OBJ} ${LDFLAGS} -o ${BIN}
