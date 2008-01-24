include ../../../../config.mk
CFLAGS+=-DDATADIR=\"${DATADIR}\"
AOBJ=aasm.o utils.o elf.o

.SUFFIXES: .c .o
.c.o:
	${E} 'Compiling $<'
	${Q}$(CC) -c $(CFLAGS) -o $@ $<

all: ${AOBJ}
	${E} 'Linking armasm'
	${Q}${CC} ${AOBJ} -o armasm

clean:
	${E} 'Cleaning src/arch/arm/aasm'
	${Q}rm -f ${AOBJ} armasm
