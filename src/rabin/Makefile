include ../../config.mk

all: rabin

rabin${BINSFX}: rabin.o
	${CC} rabin.o ${DL_LIBS} -o rabin${BINSFX}

clean:
	-rm -f rabin.o rabin${BINSFX}
