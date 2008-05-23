include ../../config.mk

all: rabin${BINSFX}

rabin${BINSFX}: rabin.o
	${CC} rabin.o ${DL_LIBS} -o rabin${BINSFX}

clean:
	-rm -f rabin.o rabin${BINSFX}
