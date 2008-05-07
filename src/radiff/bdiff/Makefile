include ../../../config.mk

all: bdiff${BINSFX}

clean:
	-rm bdiff bdiff.o

bdiff${BINSFX}: bdiff.o
	-g++ -O3 bdiff.o -o bdiff
