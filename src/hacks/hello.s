/*
*
* Example radare hack plugin written in gas
*
*/
.section	.rodata
.LC0:
	.string	"Hello hack! %s\n"
	.text
.LC1:
	.string	"hello"
.LC2:
	.string	"Hello hack example"
.LC3:
	.string	"i'm a plugin!\n"

.globl my_hack
	.type	my_hack, @function
my_hack:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	subl	$8, %esp
	movl	radare_plugin+12, %eax /*; plugin->config */
	pushl	76(%eax) /* ; config->file (+0x4c) */
	pushl	$.LC0
	call	printf
	addl	$16, %esp
	leave
	ret

	.size	my_hack, .-my_hack

.globl radare_plugin_type
	.data
	.align 4
	.type	radare_plugin_type, @object
	.size	radare_plugin_type, 4

radare_plugin_type:
	/* ; PLUGIN_TYPE_HACK = 1 */
	.long	1

.globl radare_plugin
	.data
	.align 4
	.type	radare_plugin, @object
	.size	radare_plugin, 20
radare_plugin:
	.long	.LC1
	.long	.LC2
	.long	my_hack
	.zero	8
	.section	.rodata
	.text
