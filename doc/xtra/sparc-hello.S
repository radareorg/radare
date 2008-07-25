.align 4
.global _start

_start:
	mov 1, %o0
	set string, %o1
	mov 0x11, %o2
	mov 4, %g1
	ta 0x10
	mov 0, %o0
	mov 1, %g1
	ta 0x10

string:
.asciz  "write() invoked!\n"
