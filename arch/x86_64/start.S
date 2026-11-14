#define ASM_FILE 1
#include "multiboot2.h"

.code32

.section .multiboot
.balign 8

multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long 0 # protected mode i386
    .long (multiboot_header_end - multiboot_header)
    .long -(MULTIBOOT2_HEADER_MAGIC + 0 + (multiboot_header_end - multiboot_header))

    .short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:

.text

.globl _start
_start:
    # Stack pointer
    movl $(stack + 8196), %esp

    # Reset EFLAGS
    pushl $0
    popf

    pushl %ebx # Push the boot *infomation
    pushl %eax # Push the magic

    .extern boot_main
    call boot_main # Call C main

    cli

loop:
    hlt
    jmp loop

.bss
.zero 8196
stack: