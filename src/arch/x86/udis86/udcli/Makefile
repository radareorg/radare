# Makefile
# udis86 - Cmd line interface.
#

top_srcdir	= ..
srcdir		= .
CC		?= gcc
CFLAGS		= -g -O2 -Wall -O2
LIBS		= 
INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644
prefix		= /usr/local
exec_prefix	= ${prefix}
bindir		= $(exec_prefix)/bin
libdir		= $(prefix)/lib
mandir		= ${prefix}/man
includedir	= ${prefix}/include
RM		= rm


.SUFFIXES: .c .o
.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS =	udcli.o ../libudis86.a

udcli: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o udcli
install: udcli
	$(INSTALL_PROGRAM) $(srcdir)/udcli $(INSTALLROOT)$(bindir)/udcli
uninstall:
	$(RM) -f $(INSTALLROOT)$(bindir)/udcli
clean:
	$(RM) -f core $(srcdir)/*.o $(srcdir)/udcli
distclean: clean
	$(RM) -f udcli config.h Makefile *~
