include ../config.mk

BIN=gradare${BINSFX}
OBJ=main.o topbar.o toolbar.o actions.o execute.o prefs.o menubar.o dialog.o
CFLAGS+=-DPREFIX=\"${PREFIX}\" -DGRSCDIR=\"${DATADIR}/radare/gradare\"
CFLAGS+=${GTK_FLAGS} ${VTE_FLAGS}
#`pkg-config glib-2.0 gtk+-2.0 vte --cflags`
LIBS+=${GTK_LIBS} ${VTE_LIBS}
CFLAGS+=-D_MAEMO_=${MAEMO}
#`pkg-config glib-2.0 gtk+-2.0 vte --libs`
# MAEMO STUFF
ifeq ($(MAEMO),1)
CFLAGS+=`pkg-config --cflags hildon-1`
LIBS+=`pkg-config --libs hildon-1`
endif


all: ${BIN}
	@echo 'Linking ${BIN}'

${BIN}: ${OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ} ${LIBS} -o ${BIN}

${OBJ}: %.o: %.c
	@echo 'Compiling $<'
	${CC} ${CFLAGS} -c -o $@ $<

install:
	@echo 'Installing gradare'
	${INSTALL_DIR} ${DESTDIR}/${PREFIX}/bin
	-${INSTALL_PROGRAM} ${BIN} ${DESTDIR}/${PREFIX}/bin
	-rm -rf ${DESTDIR}/${DATADIR}/radare/gradare/*
	mkdir -p ${DESTDIR}/${DATADIR}/radare/gradare/
	cp -rf grsc/* ${DESTDIR}/${DATADIR}/radare/gradare/
	-chmod +x ${DESTDIR}/${DATADIR}/radare/gradare/Shell

clean:
	@echo 'Cleaning gui'
	-rm -f ${OBJ} ${BIN}
