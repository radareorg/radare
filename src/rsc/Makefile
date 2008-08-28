include ../../config.mk
DESTDIR?=/

FILES=add asm bdcolor bin2txt dasm elf-base-addr elf-dump-header dbg-report
FILES+=elf-entry-point elf-flag-header elf-quad-word-shell findrawsyms path
FILES+=gdl2gml make_symbolmap.sh objdiff objdump-output-beatifier.pl start2bin
FILES+=read rfile-foreach spcc syms-dump syms syms-dbg-flag maps adict monitor
FILES+=syms-flag syms-xrefs elf-trace write xtrace ffftoneg core gtkdialog
FILES+=bin2tab tab2asm tab2gml move bytediff Display net-tcp net-udp net-icmp
FILES+=strings-flag syms-pe-flag flag gokolu bin2xml search-asm strings-flag
FILES+=strings-pe-flag syscall-flag dwarf-lines dwarf-addr list syms-extract
FILES+=boomerang flag-sections elf-prg-maps flag-kallsyms spcc-fe asmcolor
FILES+=signsrch fds symdiff idc2rdb

BIN=rsc

all: main.o
	${CC} -o ${BIN} main.o

list:
	@for a in pool/${FILES}; do echo $$a; done

miss:
	@perl miss.pl

install:
	${INSTALL_DIR} ${DESTDIR}/${DATADIR}
	${INSTALL_DIR} ${DESTDIR}/${DATADIR}/radare
	${INSTALL_DIR} ${DESTDIR}/${DATADIR}/radare/rsc
	rm -rf ${DESTDIR}/${DATADIR}/radare/rsc/*
	for A in ${FILES} ; do cp pool/$$A ${DESTDIR}/${DATADIR}/radare/rsc ; done

files:
	@echo ${FILES}
