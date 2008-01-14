include ../config.mk

OBJ=shellcodes.o test.o rasc.o ${SYSPROXY_OBJ}
# only for sysproxy example
OBJ2=test_sp.o rpc.o syscall-linux.o sysproxy.o

all: ${OBJ}
	${CC} -o rasc ${OBJ}

_test_sp: ${OBJ2}
	${CC} -o test_sp test_sp.o rpc.o syscall-linux.o

_rasc: ${OBJ}

test:
	${CC} -o srv test_srv1.c syscall.S -DLISTENER_TCP
	${CC} -o dump_sh dump_sh.c syscall.S -DLISTENER_TCP

clean:
	-rm -f *.o dump_sh srv test_sp rasc
