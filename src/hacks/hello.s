/*
 *
 * Example radare hack plugin written in gas
 *
 */

.section .rodata
format_string .string	"Hello hack! %s\n"
string_name:  .string	"hello"
string_desc:  .string	"Hello hack example"

.text
.globl my_hack
.type	my_hack, @function
my_hack:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	subl	$8, %esp
	/* code */
	movl	radare_plugin+12, %eax /* plugin->config */
	pushl	76(%eax) /* config->file (+0x4c) */
	pushl	format_string
	call	printf
	/* code */
	addl	$16, %esp
	leave
	ret

.data
.align 4

/* PLUGIN_TYPE_HACK = 1 */
.globl radare_plugin_type
.type	radare_plugin_type, @object
.size	radare_plugin, 4
radare_plugin_type:
	.long	1

/* plugin_hack_t structure */
.globl radare_plugin
.type	radare_plugin, @object
.size	radare_plugin, 20
radare_plugin:
	.long	string_name
	.long	string_desc
	.long	my_hack
	.zero	4 ; config_t
	.zero   4 ; debug_t

