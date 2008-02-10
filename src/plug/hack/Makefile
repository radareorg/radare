include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.4/ -I/usr/include/python2.4/
PY_LDFLAGS=-lpython2.4

CFLAGS+=-I../.. -g

all: purl puthon
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SHARED_EXT}

purl:
	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.so
puthon:
	-${CC} python.c ${SHARED_CFLAGS} ${CFLAGS} ${PY_CFLAGS} ${PY_LDFLAGS} -o python.so

clean:
	-rm -f *.${SHARED_EXT} *.o
