include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.4/ -I/usr/include/python2.4/
PY_LDFLAGS=-lpython2.4
LUA_CFLAGS=-I/usr/pkg/include -DLIBDIR=\"${LIBDIR}\"
LUA_LDFLAGS=-llua
CFLAGS+=${LUA_CFLAGS} ${PY_CFLAGS}

CFLAGS+=-I../.. -g

all: loa puthon
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SHARED_EXT}

purl:
	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.so

puthon: python.o
	-${CC} python.o ${SHARED_CFLAGS} ${CFLAGS} ${PY_CFLAGS} ${PY_LDFLAGS} -o python.so

loa: lua.o
	-${CC} lua.o ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LDFLAGS} -o lua.so

clean:
	-rm -f *.${SHARED_EXT} *.o
