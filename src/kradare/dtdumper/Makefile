all: dtdumper
dtdumper: dtdumper.c
	gcc -Wall dtdumper.c -D_LARGEFILE64_SOURCE -o dtdumper
obsd: dtdumper.c
	gcc -Wall -D BSD_SOURCE dtdumper.c -o dtdumper
