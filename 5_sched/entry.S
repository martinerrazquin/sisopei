#define MAGIC 0x1BADB002
#define FLAGS 0
#define CRC ( -(MAGIC + FLAGS) )

.text
.globl _start

.align 4
constantes_multiboot:
	.long MAGIC
	.long FLAGS
	.long CRC

_start:
	movl $0, %ebp
	movl $(bootstacktop), %esp
	jmp main
l:
	hlt
	jmp l

.data
.globl bootstack
.globl bootstacktop

.align 4096
bootstack:
	.space 4096
bootstacktop:
