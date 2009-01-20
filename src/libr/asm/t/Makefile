all: test_x86 test_arm

test_x86:
	${CC} -g -I ../../include test_x86.c ../libr_asm.a -o test_x86

test_arm:
	${CC} -g -I ../../include test_arm.c ../libr_asm.a -o test_arm

clean:
	rm -f test_x86 test_arm
