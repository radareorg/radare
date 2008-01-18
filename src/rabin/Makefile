include ../../config.mk

all: rabin.o
	#gcc ia4d*
	${CC} rabin.o ${DL_LIBS} -o rabin${BINSFX}

clean:
	-rm -f rabin.o rabin${BINSFX}
