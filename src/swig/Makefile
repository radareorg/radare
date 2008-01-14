PY_LDFLAGS+=-lpython2.4
PY_CFLAGS=-I/usr/include/python2.4/
PL_CFLAGS+=`perl -MExtUtils::Embed -e ccopts`
# NOTE: umf..-lncurses is only needed on debian?
PL_LDFLAGS+=`perl -MExtUtils::Embed -e ldopts` -lncurses
obj=main.o radare.o radare_wrap.o

all: perl python
	./a.out < test.py

perl:
	-rm radare_wrap.c
	swig -perl5 radare.i
	gcc -o pl radare_wrap.c radare.c main-perl.c ${PL_CFLAGS} ${PL_LDFLAGS} -lperl
	-perl test.pl
	#gcc ${obj} ${PL_CFLAGS} ${PL_LDFLAGS}

python:
	-rm radare_wrap.c
	-rm -rf build
	python build.py build
	gcc -o py radare_wrap.c radare.c main.c ${PY_CFLAGS} ${PY_LDFLAGS}

radare_wrap.o:
	${CC} ${CFLAGS} -c radare_wrap.c

clean:
	rm -rf Radare.py radare.pyc build radare_wrap.c *.o
