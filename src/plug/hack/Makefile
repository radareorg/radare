include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.5/ -I/usr/include/python2.5/
PY_LDFLAGS=-lpython2.5
LUA_CFLAGS=-I${PREFIX}/include/lua5.1/ -I/usr/include/lua5.1/ -DLIBDIR=\"${LIBDIR}\"
LUA_LDFLAGS=-llua
SO=${SHARED_EXT}
CFLAGS+=${LUA_CFLAGS} ${PY_CFLAGS}

CFLAGS+=-I../.. -g

all: hello.${SO} lua.${SO}  gtkhello.${SO}

#perl.${SO}

hello.${SO}:
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SO}

gtkhello.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-hello.c -I ../.. -fPIC -shared -g -o gtk-hello.${SO}

#perl.${SO}:
#	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SO}

lua.${SO}: lua.o
	-${CC} lua.o ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LDFLAGS} -o lua.so

clean:
	-rm -f *.${SHARED_EXT} *.o

