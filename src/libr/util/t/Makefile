all: test rax2

test:
	${CC} -g -I ../../include test.c ../*.o -o test

rax2:
	${CC} -g -I ../../include rax2.c ../*.o -o rax2

clean:
	rm -f calc rax2
