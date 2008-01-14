include ../../config.mk
OBJ=rasm.o x86.o arm.o java.o ppc.o main.o

all: ${OBJ}
	gcc ${OBJ} -o rasm ../utils.o

clean:
	-rm -f ${OBJ} rasm
