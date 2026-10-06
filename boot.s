/* Multiboot Header */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set VIDEOMODE, 1<<2            /* ask for video mode information */
.set FLAGS,    ALIGN | MEMINFO | VIDEOMODE
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0, 0, 0, 0, 0
.long 0 /* 0 = linear graphics mode */
.long 800 /* width */
.long 600 /* height */
.long 32  /* depth (32-bit color) */

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    /* Set up stack */
    movl $stack_top, %esp

    /* Push the pointer to the Multiboot information structure. */
    pushl %ebx

    /* Push the magic value. */
    pushl %eax

    /* Call the C main function. */
    .extern kernel_main
    call kernel_main

    /* Halt if kernel returns */
    cli
1:  hlt
    jmp 1b

.size _start, . - _start
