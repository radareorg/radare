all: baddr entry info sections symbols imports scn_resize data_resize

baddr:
	${CC} -g -I ../../include baddr.c ../*.o -o baddr
entry:
	${CC} -g -I ../../include entry.c ../*.o -o entry
info:
	${CC} -g -I ../../include info.c ../*.o -o info
sections:
	${CC} -g -I ../../include sections.c ../*.o -o sections
symbols:
	${CC} -g -I ../../include symbols.c ../*.o -o symbols
imports:
	${CC} -g -I ../../include imports.c ../*.o -o imports
scn_resize:
	${CC} -g -I ../../include scn_resize.c ../*.o -o scn_resize
data_resize:
	${CC} -g -I ../../include data_resize.c ../*.o -o data_resize

clean:
	rm -f baddr entry info sections symbols imports scn_resize data_resize
