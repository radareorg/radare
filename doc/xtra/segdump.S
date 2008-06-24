.global main

.data

buffer:
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

#define LEN 128
#define TIMES 200
#define SEGMENT %gs

.text

main:
	
	mov $TIMES, %ecx
  loopme:
	push %ecx

	sub $TIMES, %ecx
	movl $LEN, %eax
	imul %eax, %ecx
	movl %ecx, %esi /* esi = ($TIMES-%ecx)*$LEN */
	lea buffer, %edi
	movl $LEN, %ecx

	rep movsb SEGMENT:(%esi),%es:(%edi)
	/* movl %gs:(%esi), %edi */
	
	movl $4, %eax
	movl $1, %ebx
	lea buffer, %ecx
	movl $LEN, %edx
	int $0x80

	pop %ecx
	xor %eax, %eax
	cmp %eax, %ecx
	subl $1, %ecx
	jnz loopme
	/* loopz loopme */

	ret
