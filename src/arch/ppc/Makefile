CC	= gcc
COPTS	= -DBIGENDIAN -O2
CLIBS	=

vdappc: vdappc.c ppc_disasm.c ppc_disasm.h
	$(CC) $(COPTS) -o vdappc vdappc.c ppc_disasm.c $(CLIBS)
