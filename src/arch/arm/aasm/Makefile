include ../../../../config.mk
CFLAGS+=-DSHAREDIR=\"${SHAREDIR}\"
AOBJ=aasm.o utils.o elf.o

#ifeq ($(DARWIN),1)
#CFLAGS+=-fnested-functions
#endif

.SUFFIXES: .c .o
.c.o:
	-${Q}$(CC) -c $(CFLAGS) -o $@ $<

all: ${AOBJ} armasm

armasm:
	-${Q}${CC} ${AOBJ} -o armasm

clean:
	-${Q}rm -f ${AOBJ} armasm
