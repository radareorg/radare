# XXX
PREFIX=/tmp/libr-install

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

install:
	# libraries
	mkdir -p ${PREFIX}/lib
	for a in `find * | grep so$$` ; do cp $$a ${PREFIX}/lib ; done
	# includes
	mkdir -p ${PREFIX}/include
	cp -f include/* ${PREFIX}/include
	# programs
	mkdir -p ${PREFIX}/bin
	cp -f util/t/rax2 ${PREFIX}/bin
	# test programs
	mkdir -p ${PREFIX}/bin-test
	for a in `find */t -executable | perl -e 's/^[^t]*//g; print;'`; \
	do cp $$a ${PREFIX}/bin-test ; done
	@echo libr aka radare2 has been installed in PREFIX=${PREFIX}

clean:
	for lib in ${LIBLIST}; do ( cd $${lib} && ${MAKE} clean ); done
