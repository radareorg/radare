include ../config.mk

OBJ=main.o sidebar.o toolbar.o actions.o execute.o prefs.o menubar.o dialog.o
CFLAGS+=-DPREFIX=\"${PREFIX}\" -DGRSCDIR=\"${LIBEXECDIR}/gradare\"
CFLAGS+=${GTK_FLAGS} ${VTE_FLAGS}
#`pkg-config glib-2.0 gtk+-2.0 vte --cflags`
LDFLAGS+=${GTK_LIBS} ${VTE_LIBS}
#`pkg-config glib-2.0 gtk+-2.0 vte --libs`
BIN=gradare

all: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ} -o ${BIN}

install:
	${INSTALL_DIR} ${PREFIX}/bin
	${INSTALL_PROGRAM} ${BIN} ${PREFIX}/bin
	-rm -rf ${LIBEXECDIR}/gradare/*
	mkdir -p ${LIBEXECDIR}/gradare/
	cp -rf grsc/* ${LIBEXECDIR}/gradare/
	chmod +x ${LIBEXECDIR}/gradare/Shell

clean:
	-rm -f ${OBJ} ${BIN}
