include ../../../config.mk

all: bdiff${BINSFX}

clean:
	-rm bdiff bdiff.o

ifeq ($(CXX),)
bdiff${BINSFX}:
	@echo "No g++ found by configure"
else
bdiff${BINSFX}: bdiff.o
	-g++ -O2 bdiff.o -o bdiff${BINSFX}
endif
