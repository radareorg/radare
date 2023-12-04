include ../../../config.mk

# NOTE: umf..-lncurses is only needed on debian?
	#-${CC} perl.c ${CFLAGS}˘${HARED_CFLAGS}${PERL_CFLAGS} ${PERL_LDFLAGS} -o perl.${SHARED_EXT}
PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
PERL_LIBS=`perl -MExtUtils::Embed -e ldopts` -lncurses
PY_CFLAGS=-I${PREFIX}/include/python2.5/ -I/usr/include/python2.5/
PY_LIBS=-lpython2.5
PY26_CFLAGS=-I${PREFIX}/include/python2.6/ -I/usr/include/python2.6/
PY26_LIBS=-lpython2.6
PY27_CFLAGS=-I${PREFIX}/include/python2.7/ -I/usr/include/python2.7/
PY27_LIBS=-lpython2.7
LUA_CFLAGS=-I${PREFIX}/include/lua5.1/ -I/usr/include/lua5.1/
LUA_LIBS=
RUBY_CFLAGS=-I/usr/lib/ruby/1.8/i386-linux
RUBY_LIBS=-lruby18
SO=${SHARED_EXT}
CFLAGS+=${LUA_CFLAGS} ${PY_CFLAGS}

CFLAGS+=-I../.. -g

all: hello.${SO} lua.${SO} gtk-hello.${SO} scriptedit.${SO} python25.${SO} ruby.${SO} gtk-prefs.${SO} gtk-topbar.${SO} perl.${SO} python26.${SO} python27.${SO}

hello.${SO}:
	${CC} ${CFLAGS} ${SHARED_CFLAGS} hello.c -o hello.${SO}

ifeq ($(HAVE_GUI),0)
gtk-hello.${SO}:
gtk-actions.${SO}:
gtk-prefs.${SO}:
gtk-topbar.${SO}:
scriptedit.${SO}:
else
gtk-hello.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-hello.c -I ../.. -fPIC -shared -g -o gtk-hello.${SO}

gtk-actions.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-actions.c -I ../.. -fPIC -shared -g -o gtk-actions.${SO}

gtk-prefs.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-prefs.c -I ../.. -fPIC -shared -g -o gtk-prefs.${SO}

gtk-topbar.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` gtk-topbar.c -I ../.. -fPIC -shared -g -o gtk-topbar.${SO}

scriptedit.${SO}:
	-${CC} `pkg-config gtk+-2.0 --cflags --libs` scriptedit.c -I ../.. -fPIC -shared -g -o scriptedit.${SO}
endif

python25.${SO}:
ifeq ($(HAVE_LIB_PYTHON2_5),1)
	-${CC} python.c ${SHARED_CFLAGS} ${LDFLAGS} ${CFLAGS} ${PY_CFLAGS} ${PY_LIBS} -o python25.${SO}
endif

python26.${SO}:
ifeq ($(HAVE_LIB_PYTHON2_6),1)
	-${CC} python.c ${SHARED_CFLAGS} ${LDFLAGS} ${CFLAGS} ${PY26_CFLAGS} ${PY26_LIBS} -o python26.${SO}
endif

python27.${SO}:
ifeq ($(HAVE_LIB_PYTHON2_7),1)
	-${CC} python.c ${SHARED_CFLAGS} ${LDFLAGS} ${CFLAGS} ${PY26_CFLAGS} ${PY27_LIBS} -o python27.${SO}
endif

ifeq ($(HAVE_LANG_LUA),1)
perl.${SO}:
	-${CC} perl.c ${SHARED_CFLAGS} ${CFLAGS} ${PERL_CFLAGS} ${PERL_LIBS} -o perl.${SO}
perl.${SO}:
endif

ifeq ($(HAVE_LANG_RUBY),1)
ruby.${SO}:
	-ruby mkruby.rb
	#-${CC} ruby.c ${SHARED_CFLAGS} ${CFLAGS} ${RUBY_CFLAGS} ${RUBY_LDFLAGS} -o ruby.${SO} 

ruby.$(SO):
endif

# Try with -llua and -llua5.1 (stupid ubuntu)
lua.${SO}:
ifeq ($(HAVE_LANG_LUA),1)
#ifeq ($(HAVE_LIB_PYTHON2_5),1)
ifneq (,$(filter 1,$(HAVE_LIB_PYTHON2_5) $(HAVE_LIB_PYTHON2_6)))
ifneq ($(LUA_LIBS),)
	-${CC} lua.c ${SHARED_CFLAGS} ${CFLAGS} ${LUA_CFLAGS} ${LUA_LIBS} -o lua.so
endif
endif
endif
	@true

install:
	-[ -e lua.${SO} ] && ${INSTALL_LIB} lua.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e hello.${SO} ] && ${INSTALL_LIB} hello.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e gtk-hello.${SO} ] && ${INSTALL_LIB} gtk-hello.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e gtk-actions.${SO} ] && ${INSTALL_LIB} gtk-actions.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e gtk-prefs.${SO} ] && ${INSTALL_LIB} gtk-prefs.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e gtk-topbar.${SO} ] && ${INSTALL_LIB} gtk-topbar.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e scriptedit.${SO} ] && ${INSTALL_LIB} scriptedit.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e perl.${SO} ] && ${INSTALL_LIB} perl.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e ruby.${SO} ] && ${INSTALL_LIB} ruby.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e python25.${SO} ] && ${INSTALL_LIB} python25.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e python26.${SO} ] && ${INSTALL_LIB} python26.${SO} ${DESTDIR}/${LIBDIR}/radare
	-[ -e python27.${SO} ] && ${INSTALL_LIB} python27.${SO} ${DESTDIR}/${LIBDIR}/radare

clean:
	-rm -f *.${SO} *.o
