include ../../config.mk
DESTDIR?=/

FILES=add asm bdcolor bin2txt dasm elf-base-addr elf-dump-header dbg-report
FILES+=elf-entry-point elf-flag-header elf-quad-word-shell findrawsyms path
FILES+=gdl2gml make_symbolmap.sh objdiff objdump-output-beatifier.pl start2bin
FILES+=read rfile-foreach spcc syms-dump syms syms-dbg-flag maps adict monitor
FILES+=syms-flag syms-xrefs elf-trace write xtrace ffftoneg core gtkdialog
FILES+=bin2tab tab2asm tab2gml move bytediff net-tcp net-udp net-icmp
FILES+=strings-flag syms-pe-flag flag gokolu bin2xml search-asm strings-flag
FILES+=strings-pe-flag syscall-flag dwarf-lines dwarf-addr list syms-extract
FILES+=boomerang flag-sections elf-prg-maps flag-kallsyms spcc-fe asmcolor
FILES+=signsrch fds symdiff idc2rdb dwarf-addrs dwarf-at daemon daemon-cmd

BIN=rsc

all: main.o gtk-stuff
	${CC} -o ${BIN} main.o

ifeq ($(HAVE_GUI),0)
gtk-stuff:
	@echo No gtk is found..so no gtk rsc is built
else
FILES+= Display EntryDialog SeekTo SetRegister
gtk-stuff: 
	cd gtk && ${MAKE}
	cp -f gtk/Display gtk/EntryDialog gtk/SeekTo gtk/SetRegister pool
endif

clean:
	-rm -f main.o

list:
	@for a in pool/${FILES}; do echo $$a; done

miss:
	@perl miss.pl

install:
	${INSTALL_DIR} ${DESTDIR}/${LIBDIR}
	${INSTALL_DIR} ${DESTDIR}/${LIBDIR}/radare
	${INSTALL_DIR} ${DESTDIR}/${LIBDIR}/radare/bin
	rm -rf ${DESTDIR}/${LIBDIR}/radare/bin/*
	for A in pool/* ; do cp $$A ${DESTDIR}/${LIBDIR}/radare/bin ; done
	-chmod +x ${DESTDIR}/${LIBDIR}/radare/bin/*
	#for A in ${FILES} ; do cp pool/$$A ${DESTDIR}/${DATADIR}/radare/rsc ; done

files:
	@echo ${FILES}

.PHONY: files install miss list clean all gtk-stuff
