include ../../../config.mk

all: bdiff${BINSFX}

clean:
	-rm bdiff bdiff.o

bdiff${BINSFX}: bdiff.o
	-g++ -O2 bdiff.o -o bdiff${BINSFX}
