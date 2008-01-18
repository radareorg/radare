include ../../../../config.mk
CFLAGS+=-DDATADIR=\"${DATADIR}\"
AOBJ=aasm.o utils.o elf.o

all: ${AOBJ}
	${CC} ${AOBJ} -o armasm

clean:
	rm ${AOBJ} armasm
