include ../../config.mk

OBJ=rasm.o x86.o arm.o java.o ppc.o main.o ../w32utils.o
OLLYOBJ=../arch/x86/ollyasm/assembl.o
OLLYOBJ+=../arch/x86/ollyasm/asmserv.o
OLLYOBJ+=../arch/x86/ollyasm/disasm.o
OBJ+=${OLLYOBJ}

all: rasm${BINSFX}

rasm${BINSFX}: ${OBJ}
	${CC} ${OBJ} -o rasm${BINSFX} ../utils.c

clean:
	-rm -f ${OBJ} rasm${BINSFX}
