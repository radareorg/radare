include ../../config.mk

CFLAGS+=-DRADARE_CORE

all: rabin${BINSFX} javasm${BINSFX}

rabin${BINSFX}: dietelf.o dietelf64.o rabin.o
	${CC} dietelf.o dietelf64.o rabin.o ${DL_LIBS} -o rabin${BINSFX}

javasm${BINSFX}: javasm.o
	-${Q}${CC} -DJAVA_ASM_MAIN javasm.c ${JAVASM_STUFF} -o javasm${BINSFX}

clean:
	-rm -f dietelf.o dietelf64.o rabin.o rabin${BINSFX}
