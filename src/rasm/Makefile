include ../../config.mk
OBJ=rasm.o x86.o arm.o java.o ppc.o main.o ../w32utils.o

all: ${OBJ}
	${CC} ${OBJ} -o rasm${BINSFX} ../utils.o

clean:
	-rm -f ${OBJ} rasm${BINSFX}
