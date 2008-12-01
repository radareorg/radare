all: baddr entry sections symbols imports

baddr:
	${CC} -g -I ../../include baddr.c ../*.o -o baddr
entry:
	${CC} -g -I ../../include entry.c ../*.o -o entry
sections:
	${CC} -g -I ../../include sections.c ../*.o -o sections
symbols:
	${CC} -g -I ../../include symbols.c ../*.o -o symbols
imports:
	${CC} -g -I ../../include imports.c ../*.o -o imports

clean:
	rm -f baddr entry sections symbols imports
