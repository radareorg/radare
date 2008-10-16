include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.5/ -I/usr/include/python2.5/
PY_LDFLAGS=-lpython2.5
LUA_CFLAGS=-I${PREFIX}/include/lua5.1/ -I/usr/include/lua5.1/
LUA_LDFLAGS=
RUBY_CFLAGS=-I/usr/lib/ruby/1.8/i386-linux
RUBY_LDFLAGS=-lruby18
SO=${SHARED_EXT}
CFLAGS+=${LUA_CFLAGS} ${PY_CFLAGS}

CFLAGS+=-I../.. -g

all: hello.${SO} lua.${SO}  gtk-hello.${SO} scriptedit.${SO} python.${SO} ruby.${SO} gtk-prefs.${SO} gtk-topbar.${SO}
#perl.${SO}

hello.${SO}:
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SO}

gtk-hello.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-hello.c -I ../.. -fPIC -shared -g -o gtk-hello.${SO}

gtk-prefs.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-prefs.c -I ../.. -fPIC -shared -g -o gtk-prefs.${SO}

gtk-topbar.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-topbar.c -I ../.. -fPIC -shared -g -o gtk-topbar.${SO}

scriptedit.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` scriptedit.c -I ../.. -fPIC -shared -g -o scriptedit.${SO}

python.${SO}:
	-${CC} python.c ${SHARED_CFLAGS} ${LDFLAGS} ${CFLAGS} ${PY_CFLAGS} ${PY_LDFLAGS} -o python.${SO}

perl.${SO}:
	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SO}

ruby.${SO}:
	-${CC} ruby.c ${SHARED_CFLAGS} ${CFLAGS} ${RUBY_CFLAGS} ${RUBY_LDFLAGS} -o ruby.${SO}

# Try with -llua and -llua5.1 (stupid ubuntu)
lua.${SO}: lua.o
	-${CC} lua.o ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LDFLAGS} ${LUA_LIBS} -o lua.so

install:
	-[ -e lua.${SO} ] && ${INSTALL_LIB} lua.${SO} ${LIBDIR}/radare
	-[ -e hello.${SO} ] && ${INSTALL_LIB} hello.${SO} ${LIBDIR}/radare
	-[ -e gtk-hello.${SO} ] && ${INSTALL_LIB} gtk-hello.${SO} ${LIBDIR}/radare
	-[ -e scriptedit.${SO} ] && ${INSTALL_LIB} scriptedit.${SO} ${LIBDIR}/radare
	-[ -e perl.${SO} ] && ${INSTALL_LIB} perl.${SO} ${LIBDIR}/radare
	-[ -e ruby.${SO} ] && ${INSTALL_LIB} ruby.${SO} ${LIBDIR}/radare
	-[ -e python.${SO} ] && ${INSTALL_LIB} python.${SO} ${LIBDIR}/radare

clean:
	-rm -f *.${SO} *.o
