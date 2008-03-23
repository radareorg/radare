include ../../../../config.mk
CFLAGS+=-DDATADIR=\"${DATADIR}\"
AOBJ=aasm.o utils.o elf.o

.SUFFIXES: .c .o
.c.o:
	${Q}$(CC) -c $(CFLAGS) -o $@ $<

all: ${AOBJ} armasm

armasm:
	${Q}${CC} ${AOBJ} -o armasm

clean:
	${Q}rm -f ${AOBJ} armasm
