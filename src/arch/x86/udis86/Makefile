# Makefile - libudis86
#
#
include ../../../../config.mk

top_srcdir      = ..
srcdir          = .
CC             ?= gcc
CFLAGS          ?= -g -O2 -Wall -O2
LIBS            = 
INSTALL         = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA    = ${INSTALL} -m 644
prefix          = /usr/local
exec_prefix     = ${prefix}
bindir          = $(exec_prefix)/bin
libdir          = $(prefix)/lib
mandir          = ${prefix}/man
includedir      = ${prefix}/include
AR              = ar
RM              = rm
PERL            = perl

.SUFFIXES: .c .o
.c.o:
	${E} 'Compiling $<'
	${Q}$(CC) -c $(CFLAGS) -o $@ $<

OBJS =	mnemonics.o \
	opcmap.o \
	input.o \
	decode.o \
	syn.o \
	syn-intel.o \
	syn-pseudo.o \
	syn-att.o \
	udis86.o

all: libudis86.a

libudis86.a: $(OBJS)
	${E} 'Packaging libudis86.a'
	${Q}$(AR) -r libudis86.a $(OBJS)
mnemonics.c: mnemonics.dat
	${E} 'Generating mnemonics'
	${Q}$(PERL) mnemonics.pl < mnemonics.dat
mnemonics.h: mnemonics.dat
	${E} 'Generating mnemonics'
	${Q}$(PERL) mnemonics.pl < mnemonics.dat

input.c: input.h types.h
opcmap.c: opcmap.h input.h types.h
decode.c: opcmap.h input.h types.h extern.h
syn-intel.c: syn.h input.h types.h opcmap.h 
syn-att.c: syn.h input.h types.h opcmap.h 
syn.c: syn.h types.h opcmap.h 
udis86.c: input.h syn.h types.h extern.h

install: libudis86.a
	${E} 'Installing'
	${Q}$(INSTALL_PROGRAM) $(srcdir)/libudis86.a $(INSTALLROOT)$(libdir)/libudis86.a
	${Q}$(srcdir)/../mkinstalldirs $(INSTALLROOT)$(includedir)/libudis86
	${Q}$(INSTALL_DATA) $(srcdir)/../udis86.h $(INSTALLROOT)$(includedir)/udis86.h
	${Q}$(INSTALL_DATA) $(srcdir)/types.h $(INSTALLROOT)$(includedir)/libudis86/types.h
	${Q}$(INSTALL_DATA) $(srcdir)/extern.h $(INSTALLROOT)$(includedir)/libudis86/extern.h
	${Q}$(INSTALL_DATA) $(srcdir)/mnemonics.h $(INSTALLROOT)$(includedir)/libudis86/mnemonics.h

uninstall:
	${E} 'Uninstalling'
	${Q}$(RM) -f $(INSTALLROOT)$(libdir)/libudis86.a
	${Q}$(RM) -frd $(INSTALLROOT)$(includedir)/libudis86
	${Q}$(RM) -f $(INSTALLROOT)$(includedir)/udis86.h

clean:
	${E} 'Cleaning src/arch/x86/udis86'
	${Q}$(MAKE) clean -f Makefile.standalone
	${Q}$(RM) -f core $(srcdir)/*.o $(srcdir)/*.a *~

distclean: clean
	${E} 'Distcleaning src/arch/x86/udis86'
	${Q}$(RM) -f config.h Makefile *~ *.log
