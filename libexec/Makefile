include ../config.mk

FILES=add asm bdcolor bin2txt dasm elf-base-addr elf-dump-header dbg-report
FILES+=elf-entry-point elf-flag-header elf-quad-word-shell findrawsyms path
FILES+=gdl2gml make_symbolmap.sh objdiff objdump-output-beatifier.pl start2bin
FILES+=read rfile-foreach spcc syms-dump syms syms-dbg-flag maps adict monitor
FILES+=syms-flag syms-xrefs elf-trace write xtrace ffftoneg core gtkdialog
FILES+=bin2tab tab2asm tab2gml move bytediff Display net-tcp net-udp net-icmp
FILES+=strings-flag syms-pe-flag flag gokolu bin2xml search-asm strings-flag
FILES+=strings-pe-flag syscall-flag dwarf-lines dwarf-addr
##DEBUGGER##
FILES+=radaredbg
##DEBUGGER##

all:
	@echo nothing here

list:
	@for a in ${FILES}; do echo $$a; done

miss:
	@perl miss.pl

install:
	${INSTALL_DIR} ${LIBEXECDIR}
	${INSTALL_DIR} ${LIBEXECDIR}/radare
	rm -rf ${LIBEXECDIR}/radare/*
	for A in ${FILES} ; do cp $$A ${LIBEXECDIR}/radare ; done

files:
	@echo ${FILES}
