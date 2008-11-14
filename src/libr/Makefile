# Libraries
LIBLIST=io hash line

# Under development
#LIBLIST+=print
#LIBLIST+=cons
#LIBLIST+=bin

# TODO : generate single library linking against the rest
#LIBSO=libr.so
#LIBAR=libr.a

all:
	for lib in ${LIBLIST}; do ( cd $${lib} && ${MAKE} all ); done

clean:
	for lib in ${LIBLIST}; do ( cd $${lib} && ${MAKE} clean ); done
