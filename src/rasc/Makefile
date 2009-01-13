include ../../config.mk

OBJ=shellcodes.o test.o rasc.o ${SYSPROXY_OBJ}
# XXX: only for sysproxy example. to be removed
CFLAGS+=-DMKPFX=\"../\"
OBJ2=test_sp.o rpc.o syscall-linux.o sysproxy.o

all: ${OBJ}
	${E} 'Linking rasc'
	${Q}${CC} -o rasc${BINSFX} ${OSOLIBS} ${LDFLAGS} ${OBJ}

${OBJ}: %.o: %.c
	${E} 'Compiling $<'
	${Q}${CC} ${CFLAGS} -c -o $@ $<

${OBJ2}: %.o: %.c
	$(E) 'Compiling $<'
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

_test_sp: ${OBJ2}
	${E} 'Linking test_sp'
	${Q}${CC} -o test_sp test_sp.o rpc.o syscall-linux.o

_rasc: ${OBJ}

test:
	${E} 'Linking test'
	${Q}${CC} -o srv test_srv1.c syscall.S -DLISTENER_TCP
	${Q}${CC} -o dump_sh dump_sh.c syscall.S -DLISTENER_TCP

clean:
	${E} 'Cleaning rasc'
	-${Q}rm -f *.o dump_sh srv test_sp rasc *.exe
