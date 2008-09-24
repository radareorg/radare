#include ../../config.mk

CFLAGS+=-DRADARE_CORE

all: rabin${BINSFX}

rabin${BINSFX}: dietelf.o dietelf64.o rabin.o dietpe.o dietmach0.o dietmach0_utils.o
	${CC} dietelf.o dietelf64.o rabin.o dietpe.o dietmach0.o dietmach0_utils.o ${DL_LIBS} -o rabin${BINSFX}

clean:
	-rm -f dietelf.o dietelf64.o rabin.o dietpe.o dietmach0.o dietmach0_utils.o rabin${BINSFX}
