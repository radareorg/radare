include ../../config.mk

CFLAGS+=-DRADARE_CORE

all: rabin${BINSFX}

rabin${BINSFX}: rabin.o dietelf.o
	${CC} rabin.o dietelf.o ${DL_LIBS} -o rabin${BINSFX}

clean:
	-rm -f rabin.o rabin${BINSFX}
