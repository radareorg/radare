# Makefile - libudis86
#
#

top_srcdir      = ..
srcdir          = .
CC             ?= gcc
CFLAGS          = -g -O2 -Wall -O2
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
	$(CC) -c $(CFLAGS) -o $@ $<

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
	$(AR) -r libudis86.a $(OBJS)
mnemonics.c: mnemonics.dat
	$(PERL) mnemonics.pl < mnemonics.dat
mnemonics.h: mnemonics.dat
	$(PERL) mnemonics.pl < mnemonics.dat
input.c: input.h types.h
opcmap.c: opcmap.h input.h types.h
decode.c: opcmap.h input.h types.h extern.h
syn-intel.c: syn.h input.h types.h opcmap.h 
syn-att.c: syn.h input.h types.h opcmap.h 
syn.c: syn.h types.h opcmap.h 
udis86.c: input.h syn.h types.h extern.h

install: libudis86.a
	$(INSTALL_PROGRAM) $(srcdir)/libudis86.a $(INSTALLROOT)$(libdir)/libudis86.a
	$(srcdir)/../mkinstalldirs $(INSTALLROOT)$(includedir)/libudis86
	$(INSTALL_DATA) $(srcdir)/../udis86.h $(INSTALLROOT)$(includedir)/udis86.h
	$(INSTALL_DATA) $(srcdir)/types.h $(INSTALLROOT)$(includedir)/libudis86/types.h
	$(INSTALL_DATA) $(srcdir)/extern.h $(INSTALLROOT)$(includedir)/libudis86/extern.h
	$(INSTALL_DATA) $(srcdir)/mnemonics.h $(INSTALLROOT)$(includedir)/libudis86/mnemonics.h

uninstall:
	$(RM) -f $(INSTALLROOT)$(libdir)/libudis86.a
	$(RM) -frd $(INSTALLROOT)$(includedir)/libudis86
	$(RM) -f $(INSTALLROOT)$(includedir)/udis86.h
clean:
	$(MAKE) clean -f Makefile.standalone
	$(RM) -f core $(srcdir)/*.o $(srcdir)/*.a *~

distclean: clean
	$(RM) -f config.h Makefile *~ *.log
