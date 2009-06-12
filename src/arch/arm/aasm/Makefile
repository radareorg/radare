include ../../../../config.mk
CFLAGS+=-DSHAREDIR=\"${DATADIR}\"
AOBJ=aasm.o utils.o elf.o

ifeq ($(DARWIN),1)
CFLAGS+=-fnested-functions
endif

.SUFFIXES: .c .o
.c.o:
	-$(CC) -c $(CFLAGS) -o $@ $<

all: armasm

armasm: ${AOBJ} 

armasm:
	-${CC} ${AOBJ} -o armasm

clean:
	-rm -f ${AOBJ} armasm
