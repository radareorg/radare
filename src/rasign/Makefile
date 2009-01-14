CFLAGS+=-I../libr/include -g
OBJ=main.o sign.o
BIN=rasign

all: ${OBJ}
	${CC} ${OBJ} -o ${BIN}
	
clean:
	-rm -f ${OBJ} ${BIN}
