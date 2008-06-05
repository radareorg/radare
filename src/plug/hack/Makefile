include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.5/ -I/usr/include/python2.5/
PY_LDFLAGS=-lpython2.5
LUA_CFLAGS=-I${PREFIX}/include/lua5.1/ -I/usr/include/lua5.1/ -DLIBDIR=\"${LIBDIR}\"
LUA_LDFLAGS=
SO=${SHARED_EXT}
CFLAGS+=${LUA_CFLAGS} ${PY_CFLAGS}

CFLAGS+=-I../.. -g

all: hello.${SO} lua.${SO}  gtkhello.${SO} luaedit.${SO}

#perl.${SO}

hello.${SO}:
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SO}

gtkhello.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-hello.c -I ../.. -fPIC -shared -g -o gtk-hello.${SO}

luaedit.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` luaedit.c -I ../.. -fPIC -shared -g -o luaedit.${SO}

#perl.${SO}:
#	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SO}

# Try with -llua and -llua5.1 (stupid ubuntu)
lua.${SO}: lua.o
	-${CC} lua.o ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LDFLAGS} -llua -o lua.so || \
	${CC} lua.o ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LDFLAGS} -llua5.1 -o lua.so

install:
	-[ -e lua.${SHARED_EXT} ] && ${INSTALL_LIB} lua.${SHARED_EXT} ${LIBDIR}/radare
	-[ -e hello.${SHARED_EXT} ] && ${INSTALL_LIB} hello.${SHARED_EXT} ${LIBDIR}/radare
	-[ -e gtk-hello.${SHARED_EXT} ] && ${INSTALL_LIB} gtk-hello.${SHARED_EXT} ${LIBDIR}/radare
	-[ -e luaedit.${SHARED_EXT} ] && ${INSTALL_LIB} luaedit.${SHARED_EXT} ${LIBDIR}/radare
	-[ -e perl.${SHARED_EXT} ] && ${INSTALL_LIB} perl.${SHARED_EXT} ${LIBDIR}/radare
	-[ -e python.${SHARED_EXT} ] && ${INSTALL_LIB} python .${SHARED_EXT} ${LIBDIR}/radare

clean:
	-rm -f *.${SHARED_EXT} *.o
