include ../../config.mk

OBJ=shellcodes.o test.o rasc.o ${SYSPROXY_OBJ}
# XXX: only for sysproxy example. to be removed
CFLAGS+=-DMKPFX=\"../\"
OBJ2=test_sp.o rpc.o syscall-linux.o sysproxy.o
BIN=rasc${BINSFX}

all: ${BIN}

${BIN}: ${OBJ}
	@echo 'Linking rasc'
	${CC} ${LDFLAGS} -o ${BIN} ${OSOLIBS} ${LIBS} ${OBJ}

${OBJ}: %.o: %.c
	@echo 'Compiling $<'
	${CC} ${CFLAGS} -c -o $@ $<

${OBJ2}: %.o: %.c
	@echo 'Compiling $<'
	$(CC) $(CFLAGS) -c -o $@ $<

_test_sp: ${OBJ2}
	@echo 'Linking test_sp'
	${CC} -o test_sp test_sp.o rpc.o syscall-linux.o

_rasc: ${OBJ}

test:
	@echo 'Linking test'
	${CC} -o srv test_srv1.c syscall.S -DLISTENER_TCP
	${CC} -o dump_sh dump_sh.c syscall.S -DLISTENER_TCP

clean:
	@echo 'Cleaning rasc'
	-rm -f *.o dump_sh srv test_sp rasc *.exe
