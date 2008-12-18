# Libraries
LIBLIST=io hash line lib syscall socket cons

# Under development
#LIBLIST+=print
#LIBLIST+=util
#LIBLIST+=search
LIBLIST+=bin

# TODO : generate single library linking against the rest
#LIBSO=libr.so
#LIBAR=libr.a

all:
	for lib in ${LIBLIST}; do ( cd $${lib} && ${MAKE} all ); done
	@echo Build done for: ${LIBLIST}

clean:
	for lib in ${LIBLIST}; do ( cd $${lib} && ${MAKE} clean ); done
