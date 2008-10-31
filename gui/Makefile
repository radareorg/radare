include ../config.mk

BIN=gradare${BINSFX}
OBJ=main.o topbar.o toolbar.o actions.o execute.o prefs.o menubar.o dialog.o
CFLAGS+=-DPREFIX=\"${PREFIX}\" -DGRSCDIR=\"${DATADIR}/radare/gradare\"
CFLAGS+=${GTK_FLAGS} ${VTE_FLAGS}
#`pkg-config glib-2.0 gtk+-2.0 vte --cflags`
LDFLAGS+=${GTK_LIBS} ${VTE_LIBS}
CFLAGS+=-D_MAEMO_=${MAEMO}
#`pkg-config glib-2.0 gtk+-2.0 vte --libs`
# MAEMO STUFF
ifeq ($(MAEMO),1)
CFLAGS+=`pkg-config --cflags hildon-1`
LDFLAGS+=`pkg-config --libs hildon-1`
endif


all: ${OBJ} ${BIN}
	${E} 'Linking ${BIN}'

${BIN}:
	${Q}${CC} ${CFLAGS} ${LDFLAGS} ${OBJ} -o ${BIN}

${OBJ}: %.o: %.c
	${E} 'Compiling $<'
	${Q}${CC} ${CFLAGS} -c -o $@ $<

install:
	${E} 'Installing gradare'
	${Q}${INSTALL_DIR} ${DESTDIR}/${PREFIX}/bin
	${Q}${INSTALL_PROGRAM} ${BIN} ${DESTDIR}/${PREFIX}/bin
	-${Q}rm -rf ${DESTDIR}/${DATADIR}/radare/gradare/*
	${Q}mkdir -p ${DESTDIR}/${DATADIR}/radare/gradare/
	${Q}cp -rf grsc/* ${DESTDIR}/${DATADIR}/radare/gradare/
	-${Q}chmod +x ${DESTDIR}/${DATADIR}/radare/gradare/Shell

clean:
	${E} 'Cleaning gui'
	-${Q}rm -f ${OBJ} ${BIN}
