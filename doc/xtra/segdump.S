#define FROM 0 /* 0x8048000 */
#define LEN 0x940
#define TIMES 1
#define SEGMENT %gs /* %ds */

.global main

.data
buffer:
.fill LEN, TIMES, 0x00

.text
main:
	
	mov $TIMES, %ecx
  loopme:
	push %ecx

	sub $TIMES, %ecx
	movl $LEN, %eax
	imul %eax, %ecx
	movl %ecx, %esi /* esi = ($TIMES-%ecx)*$LEN */
	addl $FROM, %esi
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
